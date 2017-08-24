#ifdef MS_CLR

#include "rClr.h"


void init_global_msdotnet()
{
	pMetaHost = NULL;
	pRuntimeInfo = NULL;
	pClrRuntimeHost = NULL;
	spAssembly = NULL;
	spDefaultAppDomain = NULL;
	domainId = 0;
	callback = NULL;

#ifdef USE_COR_RUNTIME_HOST
	pCorRuntimeHost = NULL;
#else
	pRuntimeHost = nullptr;
#endif

	spAppDomainThunk = NULL;
	spTypeClrFacade = NULL;
}

void start_ms_clr() {
	init_global_msdotnet();
    HRESULT hr;
    hr = CLRCreateInstance( CLSID_CLRMetaHost, IID_PPV_ARGS( &pMetaHost ) );
    if (FAILED( hr ))
    {
        error( "CLRCreateInstance failed" );
    }
    hr = pMetaHost->GetRuntime( pszVersion, IID_PPV_ARGS( &pRuntimeInfo ) );
    if (FAILED( hr ))
    {
        error( "ICLRMetaHost::GetRuntime failed" );
    }
    BOOL fLoadable;
    hr = pRuntimeInfo->IsLoadable( &fLoadable );
    if (FAILED( hr ))
    {
        error( "ICLRRuntimeInfo::IsLoadable failed" );
    }

    if (!fLoadable)
    {
        error( ".NET runtime cannot be loaded" );
    }
    hr = pRuntimeInfo->GetInterface( CLSID_CLRRuntimeHost,
        IID_PPV_ARGS( &pClrRuntimeHost ) );
    if (FAILED( hr ))
    {
        error( "ICLRRuntimeInfo::GetInterface failed w/hr 0x%08lx\n", hr );
        goto Cleanup;
    }

    // Start the CLR.
    hr = pClrRuntimeHost->Start();
    if (FAILED( hr ))
    {
        error( "CLR failed to start w/hr 0x%08lx\n", hr );
        goto Cleanup;
    }
    return;
Cleanup:
    ms_rclr_cleanup();
}

void ms_rclr_cleanup()
{
    if (pMetaHost)
    {
        pMetaHost->Release();
        pMetaHost = NULL;
    }
    if (pRuntimeInfo)
    {
        pRuntimeInfo->Release();
        pRuntimeInfo = NULL;
    }
    if (pClrRuntimeHost)
    {
        // Please note that after a call to Stop, the CLR cannot be 
        // reinitialized into the same process. This step is usually not 
        // necessary. You can leave the .NET runtime loaded in your process.
        //error("Stop the .NET runtime\n");
        //pClrRuntimeHost->Stop();

        pClrRuntimeHost->Release();
        pClrRuntimeHost = NULL;
    }
	init_global_msdotnet();
}

void release_transient_objects()
{
    for (size_t i = 0; i < transientArgs.size(); i++)
    {
        VariantClear( transientArgs.at( i ) );
    }
    transientArgs.clear();
}


void free_variant_array( VARIANT ** a, int size ) {
    SAFEARRAY * a_ptr;
    for (int i = 0; i < size; i++)
    {
        // presume VT_UNKNOWN is a CLR Object that is referenced from 
        // R with an external pointer. Really bad idea to clear it.
        if ((a[i]->vt != VT_UNKNOWN) && (a[i]->vt != VT_DISPATCH)) // without check on DISPATCH one gets the bug https://r2clr.codeplex.com/workitem/71 
        {
            // A relevant read is at http://social.msdn.microsoft.com/Forums/vstudio/en-US/7313928a-e386-406a-b03b-499dd350009f/how-to-determine-if-variant-contains-an-array
            //if(a[i]->vt & VT_ARRAY) {
            //	a_ptr = a[i]->parray;
            //	//VariantClear(a[i]);
            //	SafeArrayDestroy(a_ptr);
            //}
            //else
            VariantClear( a[i] );
        }
    }
    delete[] a;
}

CLR_OBJ * rclr_ms_convert_element_rdotnet( SEXP el )
{
    // The idea here is that we create a SymbolicExpression in C#, that the C# code will intercept
    CLR_OBJ vtResult; // needs this: otherswise null pointer if using only objptr. 
    HRESULT hr = S_FALSE;
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( VT_VARIANT, 0, 1 );
    LONG index = 0;
    int size_of_voidptr = (sizeof( void* ));
    variant_t param( (LONGLONG)el );
    param.vt = VT_I8;
    hr = SafeArrayPutElement( psaStaticMethodArgs, &index, &param );
    bstr_t createSexpWrapper( "CreateSexpWrapperMs" );
    hr = spTypeClrFacade->InvokeMember_3( createSexpWrapper, static_cast<BindingFlags>(
        BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
        NULL, vtEmpty, psaStaticMethodArgs, &vtResult );
    if (FAILED( hr )) {
        error( "%s", "Failure in rclr_convert_element_rdotnet" );
    }
    SafeArrayDestroy( psaStaticMethodArgs );
    auto result = new CLR_OBJ( vtResult );
    transientArgs.push_back( result );
    return result;
}

HRESULT rclr_ms_create_clr_complex_direct( VARIANT * vtResult )
{
    return S_FALSE;
}

//void create_BSTR(VARIANT * v, char * cstr) {
//	VariantInit(v);
//	v->vt = VT_BSTR;
//	BSTR b = new _bstr_t(cstr);
//	v->bstrVal = SysAllocString(b);
//}
//
//void clear_BSTR(VARIANT * v) {
//	// SysFreeString(v->bstrVal); // I think if doubling up on VariantClear, can lead to heap corruption
//	VariantClear(v);
//}

HRESULT rclr_ms_call_static_method_facade( char * methodName, CLR_OBJ * objptr, VARIANT * result ) {
    //CLR_OBJ * rclr_ms_call_static_method_facade(char * methodName, CLR_OBJ * objptr) {
    bstr_t bstrMethodName( methodName );
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( VT_VARIANT, 0, 1 );
    LONG index = 0;
    variant_t param( objptr );
    HRESULT hr = SafeArrayPutElement( psaStaticMethodArgs, &index, &param );
    if (SUCCEEDED( hr ))
    {
        //CLR_OBJ tmpResult;
        hr = rclr_ms_call_static_method( spTypeClrFacade, &bstrMethodName, psaStaticMethodArgs, result );
    }
    SafeArrayDestroy( psaStaticMethodArgs );
    return hr;
}

SEXP rclr_ms_get_type_name( SEXP clrObj ) {
    CLR_OBJ * objptr = get_clr_object( clrObj );
    return make_char_single_sexp( get_type_full_name( objptr ) );
}

const char * get_type_full_name( CLR_OBJ * objptr ) {
    CLR_OBJ tmpResult;
    HRESULT hr = rclr_ms_call_static_method_facade( "GetObjectTypeName", objptr, &tmpResult );
    bstr_t blah( tmpResult.bstrVal );
    return bstr_to_c_string( &blah );

    /*
    _TypePtr pType;
    HRESULT hr = rclr_ms_gettype(objptr, &pType);
    if(FAILED(hr)) {
    error("Getting object type failed with message: %s\n", getComErrorMsg(hr));
    return "<unable to find type>";
    } else if(!pType) {
    return "<unable to find type>";
    }
    BSTR typeName;
    pType->get_FullName(&typeName);
    bstr_t bstrTypeName(typeName);
    return bstr_to_c_string(&bstrTypeName);
    */
}

void get_array_variant( CLR_OBJ * pobj, SAFEARRAY ** array, int * n_ptr, LONG * plUbound )
{
    *array = pobj->parray;
    SafeArrayGetUBound( *array, 1, plUbound );
    (*n_ptr) = ((int)(*plUbound)) + 1;
}

SEXP clr_obj_ms_convert_to_SEXP( CLR_OBJ &obj ) {
    CLR_OBJ * pobj = new CLR_OBJ( obj ); // the allocation of a pobj here will hold a reference (handle) to the managed object.
    SEXP result = NULL;
    int * iVals = NULL;
    double * rVals = NULL;
    bool * bVals = NULL;
    unsigned char * ucharVals = NULL;
    char ** strVals = NULL;
    int n = 1;
    SAFEARRAY * array = NULL;
    long uBound = 0;
    HRESULT hr;

    switch (pobj->vt) {
    case VT_UNKNOWN: // this is what we get for instance on a call to ModelRunner.get_Model. Maybe, when an interface type is returned rather than a class.
    case VT_DISPATCH:
        return clr_object_to_SEXP( pobj );
    case VT_ARRAY | VT_R8:
        get_array_variant( pobj, &array, &n, &uBound );
        rVals = (double*)malloc( sizeof( double )*n );
        for (long i = 0; i < n; i++) {
            SafeArrayGetElement( array, &i, &(rVals[i]) );
        }
        result = make_numeric_sexp( n, rVals );
        free( rVals );
        break;
    case VT_DATE | VT_ARRAY:
        //hr = rclr_ms_call_static_method_facade("DateTimeArrayToUtc", pobj, &date_time_utc);
        //get_array_variant(&date_time_utc, &array, &n, &uBound);
        get_array_variant( pobj, &array, &n, &uBound );
        rVals = (double*)malloc( sizeof( double )*n );
        for (long i = 0; i < n; i++) {
            SafeArrayGetElement( array, &i, &(rVals[i]) );
            rVals[i] = COMdatetimeToPosixCt( rVals[i] );
        }
        result = make_POSIXct_sexp( n, rVals );
        free( rVals );
        break;
    case VT_ARRAY | VT_I2:
    case VT_ARRAY | VT_I4:
        get_array_variant( pobj, &array, &n, &uBound );
        iVals = (int*)malloc( sizeof( int )*n );
        for (long i = 0; i < n; i++) {
            SafeArrayGetElement( array, &i, &(iVals[i]) );
        }
        result = make_int_sexp( n, iVals );
        free( iVals );
        break;
    case VT_ARRAY | VT_BOOL:
        get_array_variant( pobj, &array, &n, &uBound );
        bVals = (bool*)malloc( sizeof( bool )*n );
        for (long i = 0; i < n; i++) {
            SafeArrayGetElement( array, &i, &(bVals[i]) );
        }
        result = make_bool_sexp( n, bVals );
        free( bVals );
        break;
    case VT_ARRAY | VT_BSTR:
        array = pobj->parray;
        SafeArrayGetUBound( array, 1, &uBound );
        n = ((int)uBound) + 1;
        BSTR bstrVal;
        strVals = (char**)malloc( sizeof( char* )*n );
        for (long i = 0; i < n; i++) {
            SafeArrayGetElement( array, &i, &bstrVal );
            bstr_t bstrtVal( bstrVal );
            strVals[i] = bstr_to_c_string( &bstrtVal );
        }
        result = make_char_sexp( n, strVals );
        // TOCHECK: does R copy the char array, and do I need to free each (char*) too?
        free( strVals );
        break;
    case VT_ARRAY | VT_UI1:  // byte[]:  8209
        get_array_variant( pobj, &array, &n, &uBound );
        ucharVals = (unsigned char*)malloc( sizeof( unsigned char )*n );
        for (long i = 0; i < n; i++) {
            SafeArrayGetElement( array, &i, &(ucharVals[i]) );
        }
        result = make_uchar_sexp( n, ucharVals );
        free( ucharVals );
        break;
        // The following would be handling .NET characters; however these are unicode characters, not ANSI characters.
        //case VT_ARRAY | VT_UI2:  // char[]:  8210. 
    case VT_EMPTY:
    case VT_NULL:
    case VT_VOID:
        result = R_NilValue;
        break;
    case VT_I2:
        iVals = (int*)malloc( sizeof( int )*n );
        iVals[0] = pobj->iVal;
        result = make_int_sexp( 1, iVals );
        free( iVals );
        break;
    case VT_I4:
        iVals = (int*)malloc( sizeof( int )*n );
        iVals[0] = pobj->intVal;
        result = make_int_sexp( 1, iVals );
        free( iVals );
        break;
    case VT_R4:
        rVals = (double*)malloc( sizeof( double )*n );
        rVals[0] = pobj->fltVal;
        result = make_numeric_sexp( n, rVals );
        free( rVals );
        break;
    case VT_R8:
        rVals = (double*)malloc( sizeof( double )*n );
        rVals[0] = pobj->dblVal;
        result = make_numeric_sexp( n, rVals );
        free( rVals );
        break;
    case VT_DATE:
    {
        //bstr_t format("yyyy-MM-dd HH:mm:ss");
        //BSTR date;
        //VarFormat(pobj, format, 0, 0, VAR_CALENDAR_GREGORIAN, &date);
        //bstr_t bstrDate(date);
        //char * chDate = bstr_to_c_string(&bstrDate);
        //result = make_char_sexp(n, &chDate);
        //hr = rclr_ms_call_method_argless(pobj, "DateTimeArrayToUtc", &date_time_utc);
        //double rDateNumericValue = COMdatetimeToPosixCt(date_time_utc.dblVal);
        double rDateNumericValue = COMdatetimeToPosixCt( pobj->dblVal );
        result = make_POSIXct_sexp( n, &rDateNumericValue );
        break;
    }
    case VT_BSTR:
    {
        strVals = (char**)malloc( sizeof( char* )*n );
        bstr_t tmpBstr( pobj->bstrVal );
        strVals[0] = bstr_to_c_string( &tmpBstr );
        result = make_char_sexp( n, strVals );
        free( strVals );
        break;
    }
    case VT_BOOL:
        bVals = (bool*)malloc( sizeof( bool )*n );
        bVals[0] = (bool)pobj->boolVal;
        result = make_bool_sexp( n, bVals );
        free( bVals );
        break;
    case VT_INT:
        // This seems to be the case when an IntPtr is returned, native handle to an R.NET object.
        // However, this is not intVal that we need to retrieve. This is not well documented, but 
        // on 64 bits you would have nasty surprises. ullVal reports the same value as the IntPtr in the managed world
        return (SEXP)pobj->ullVal;
        //case VT_CY:
        //case VT_ERROR:
        //case VT_VARIANT:
        //case VT_DECIMAL:
        //case VT_I1:
        //case VT_UI1:
        //case VT_UI2:
        //case VT_UI4:
        //case VT_I8:
        //case VT_UI8:
        //case VT_INT_PTR:
        //case VT_UINT:
        //case VT_HRESULT:
        //case VT_PTR:
        //case VT_SAFEARRAY:
        //case VT_CARRAY:
        //case VT_USERDEFINED:
        //case VT_LPSTR:
        //case VT_LPWSTR:
        //case VT_RECORD:
        //case VT_UINT_PTR:
        //case VT_FILETIME:
        //case VT_BLOB:
        //case VT_STREAM:
        //case VT_STORAGE:
        //case VT_STREAMED_OBJECT:
        //case VT_STORED_OBJECT:
        //case VT_BLOB_OBJECT:
        //case VT_CF:
        //case VT_CLSID:
        //case VT_VERSIONED_STREAM:
        //case VT_BSTR_BLOB:
        //case VT_VECTOR:
        //case VT_BYREF:
        //case VT_RESERVED:
        //case VT_ILLEGAL:
    default:
        error( "clr_obj_ms_convert_to_SEXP: COM variant type code %d unsupported. Returning NULL", pobj->vt );
        result = R_NilValue;
    }
    delete(pobj); // Should this function be responsible for this delete?
    return result;
}


//case VT_EMPTY:
//case VT_NULL:
//case VT_UNKNOWN:
//case VT_VOID:
//case VT_I2:
//case VT_I4:
//case VT_R4:
//case VT_R8:
//case VT_CY:
//case VT_DATE:
//case VT_BSTR:
//case VT_DISPATCH:
//case VT_ERROR:
//case VT_BOOL:
//case VT_VARIANT:
//case VT_DECIMAL:
//case VT_I1:
//case VT_UI1:
//case VT_UI2:
//case VT_UI4:
//case VT_I8:
//case VT_UI8:
//case VT_INT:
//case VT_INT_PTR:
//case VT_UINT:
//case VT_HRESULT:
//case VT_PTR:
//case VT_SAFEARRAY:
//case VT_CARRAY:
//case VT_USERDEFINED:
//case VT_LPSTR:
//case VT_LPWSTR:
//case VT_RECORD:
//case VT_UINT_PTR:
//case VT_FILETIME:
//case VT_BLOB:
//case VT_STREAM:
//case VT_STORAGE:
//case VT_STREAMED_OBJECT:
//case VT_STORED_OBJECT:
//case VT_BLOB_OBJECT:
//case VT_CF:
//case VT_CLSID:
//case VT_VERSIONED_STREAM:
//case VT_BSTR_BLOB:
//case VT_VECTOR:
//case VT_ARRAY:
//case VT_BYREF:
//case VT_RESERVED:
//case VT_ILLEGAL:

// Create a safe array to contain the arguments of the method.
HRESULT rclr_ms_create_object( char * longtypename, VARIANT ** params, int argLength, VARIANT * vtResult ) {
    bstr_t bstrStaticMethodName( L"CreateInstance" );
    variant_t tname( longtypename );
    SAFEARRAY * psaCtorArgs = rclr_ms_create_constructor_parameters( &tname, params, argLength );
    HRESULT hr = rclr_ms_call_static_method( spTypeClrFacade, &bstrStaticMethodName, psaCtorArgs, vtResult );
    SafeArrayDestroy( psaCtorArgs );
    //clear_BSTR(&tname);
    return hr;
}

HRESULT rclr_ms_load_assembly( char ** filename, VARIANT * vtResult ) {
    bstr_t bstrStaticMethodName( L"LoadFrom" );
    return rclr_ms_call_static_method_stringarg( spTypeClrFacade, &bstrStaticMethodName, *filename, vtResult );
}

HRESULT rclr_ms_call_static_method_tname( char * ns_qualified_typename, char * mnam, VARIANT ** params, int argLength, VARIANT * result ) {
    HRESULT hr;
    SAFEARRAY * psaStaticMethodArgs = rclr_ms_create_static_method_call_parameters( ns_qualified_typename, mnam, params, argLength );
    bstr_t bstrCallStaticMethod( "CallStaticMethod" );
    // TODO / BUG: if there is only one array passed as an argument, then CallStaticMethod will "somehow" 
    // flatten to e.g. 4 arguments, instead of one objet argument that is an array/
    hr = rclr_ms_call_static_method( spTypeClrFacade, &bstrCallStaticMethod, psaStaticMethodArgs, result );
    SafeArrayDestroy( psaStaticMethodArgs );
    return hr;
}

HRESULT rclr_ms_call_method( CLR_OBJ * objptr, char * methodName, VARIANT ** params, int argLength, VARIANT * vtResult ) {
    SAFEARRAY * psaStaticMethodArgs = rclr_ms_create_method_call_parameters( objptr, methodName, params, argLength );
    bstr_t bstrCallInstanceMethod( "CallInstanceMethod" );
    HRESULT hr = rclr_ms_call_static_method( spTypeClrFacade, &bstrCallInstanceMethod, psaStaticMethodArgs, vtResult );
    SafeArrayDestroy( psaStaticMethodArgs );
    return hr;
}

HRESULT rclr_ms_call_static_method( _TypePtr spType, bstr_t * bstrStaticMethodNamePtr, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult ) {
    HRESULT hr = S_FALSE;
    bstr_t getLastException( "get_LastCallException" );
    //_TCHAR* blah;
    //try {
    hr = spType->InvokeMember_3( *bstrStaticMethodNamePtr, static_cast<BindingFlags>(
        BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
        NULL, vtEmpty, psaStaticMethodArgs, vtResult );
    //} 
    //catch(_com_error& e) {
    //	blah = TEXT("ERROR: %s\n"),(_TCHAR*)e.Description();
    //} 
    if (FAILED( hr )) {
        // Work around https://r2clr.codeplex.com/workitem/67
        hr = spType->InvokeMember_3( getLastException, static_cast<BindingFlags>(
            BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
            NULL, vtEmpty, NULL, vtResult );
        bstr_t tmpBstr( vtResult->bstrVal );
        if (std::string( (char*)vtResult->bstrVal ) == std::string( "" ))
            error( "%s", "Failure in rclr_ms_call_static_method, but could not retrieve an error message" );
        else
            // There seems to be a limit to the size printed by error, which is a problem for long stack traces
            // Nevertheless cannot use Rprintf here or this breaks the try/error patterns e.g. with the testthat package...
            // Rprintf("%s", bstr_to_c_string( &tmpBstr ));
            error( "%s", bstr_to_c_string( &tmpBstr ) );
    }
    return hr;
}

HRESULT rclr_ms_call_static_method_stringarg( _TypePtr spType, bstr_t * bstrStaticMethodNamePtr, char * strArg, VARIANT * vtResult ) {
    SAFEARRAY * psaStaticMethodArgs = create_array_one_string( strArg );
    return rclr_ms_call_static_method( spType, bstrStaticMethodNamePtr, psaStaticMethodArgs, vtResult );
}


HRESULT rclr_ms_get_current_object_direct( VARIANT * vtResult ) {
    HRESULT hr = S_FALSE;
    bstr_t getCurrentObject( "get_CurrentObject" );
    hr = spTypeClrFacade->InvokeMember_3( getCurrentObject, static_cast<BindingFlags>(
        BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
        NULL, vtEmpty, NULL, vtResult );
    if (FAILED( hr )) {
        error( "%s", "Failure in rclr_ms_get_current_object_direct" );
    }
    return hr;
}

// Obsolete??
HRESULT rclr_ms_call_method_interface( CLR_OBJ * obj, const char *interfaceName, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult ) {
    HRESULT hr;
    _ObjectPtr pObject;
    rclr_ms_get_raw_object( obj, &pObject );
    _TypePtr pType;
    hr = pObject->GetType( &pType );
    if (FAILED( hr )) {
        error( "Retrieving the CLR type failed with message: %s\n", getComErrorMsg( hr ) );
        return hr;
    }
    bstr_t ifName( interfaceName );
    //BSTR ifName(interfaceName);
    _TypePtr pIfType;
    hr = pType->GetInterface( ifName, VARIANT_BOOL( 0 ), &pIfType );
    if (FAILED( hr )) {
        error( "Retrieving the interface type for %s failed with message: %s\n", interfaceName, getComErrorMsg( hr ) );
        return hr;
    }
    return rclr_ms_call_method_type( obj, pIfType, mnam, psaStaticMethodArgs, vtResult );
}

HRESULT rclr_ms_call_method_type( CLR_OBJ * obj, _TypePtr pType, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult )
{
    bstr_t bstrMethodName( mnam );
    //SAFEARRAY * members = SafeArrayCreateVector(VT_VARIANT, 0, 1);
    HRESULT hr;
    //hr = pType->GetMember_3(bstrMethodName, &members);
    //_MethodBasePtr mPtr;
    //SafeArrayGetElement( members, 0, mPtr);
    //hr = mPtr->Invoke_3(*obj, psaStaticMethodArgs, vtResult);
    hr = pType->InvokeMember_3( bstrMethodName, static_cast<BindingFlags>(
        // The specification of binding flags as per the following line fails to find some interface methods (explicit interface implementations in C#, probably
        //        BindingFlags_InvokeMethod | BindingFlags_Instance | BindingFlags_Public), 
        BindingFlags_InvokeMethod),
        NULL, *obj, psaStaticMethodArgs, vtResult );
    if (FAILED( hr )) {
        error( "Instance method call failed with message: %s\n", getComErrorMsg( hr ) );
    }
    return hr;
}

// BUG: stubborn problems with this one. Give up... DEPRECATED
HRESULT rclr_ms_gettype( CLR_OBJ * obj, _Type ** pType ) {

    //bstr_t getTypeMname("GetObjectType");
    //variant_t ** args = new variant_t* [1]; args[0] = obj;
    //variant_t result;
    //HRESULT hr = rclr_ms_call_static_method(spTypeClrFacade, &getTypeMname, create_safe_array(args, 1), &result);
    //if(FAILED(hr))
    //	return hr;
    //else
    //{
    //	_ObjectPtr pObject; 
    //	hr = rclr_ms_get_raw_object(obj, &pObject);
    //	if(FAILED(hr))
    //		return hr;
    //	pType = (_Type**)(&pObject);
    //	hr = pObject->QueryInterface(__uuidof(_Type), (void**)pType);
    //	return hr;
    //}

    _ObjectPtr pObject;
    HRESULT hr = rclr_ms_get_raw_object( obj, &pObject );
    if (!SUCCEEDED( hr ))
        return hr;
    else
    {
        try
        {
            return pObject->GetType( pType ); // TODO: this fails with the MS CLR. Unclear why. Big blocker.
        }
        catch (...)
        {
            pType = NULL;
            return -1;
        }
    }
}

HRESULT rclr_ms_get_raw_object( CLR_OBJ * obj, _Object ** ppObject ) {
    HRESULT hr;
    switch (obj->vt)
    {
    case VT_DISPATCH:
        hr = obj->pdispVal->QueryInterface( __uuidof(_Object), (void**)ppObject );
        break;
    case VT_UNKNOWN:
        hr = obj->punkVal->QueryInterface( __uuidof(_Object), (void**)ppObject );
        break;
    default:
        hr = S_FALSE;
    }
    return hr;
}

// likely obsolete
HRESULT rclr_ms_call_method_sa( CLR_OBJ * obj, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult )
{
    _TypePtr pType;
    HRESULT hr = rclr_ms_gettype( obj, &pType );
    if (FAILED( hr ) || !pType) {
        return hr;
    }
    return rclr_ms_call_method_type( obj, pType, mnam, psaStaticMethodArgs, vtResult );
}

// likely obsolete
HRESULT rclr_ms_call_method_stringarg( CLR_OBJ * obj, const char *mnam, char * strArg, VARIANT * vtResult ) {
    SAFEARRAY * psaStaticMethodArgs = create_array_one_string( strArg );
    HRESULT hr = rclr_ms_call_method_sa( obj, mnam, psaStaticMethodArgs, vtResult );
    SafeArrayDestroy( psaStaticMethodArgs );
    return hr;
}

// likely obsolete
HRESULT rclr_ms_call_method_argless( CLR_OBJ * obj, const char *mnam, VARIANT * vtResult ) {
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( VT_VARIANT, 0, 0 );
    HRESULT hr = rclr_ms_call_method_sa( obj, mnam, psaStaticMethodArgs, vtResult );
    SafeArrayDestroy( psaStaticMethodArgs );
    return hr;
}

//struct managedplugin_t
//{
//	std::wstring                m_PluginName;
//	std::wstring                m_PluginPath;
//	CComPtr<IUnknown>           m_AppDomainSetupUnknown;
//	CComQIPtr<IAppDomainSetup>  m_AppDomainSetup;
//	CComPtr<IUnknown>           m_AppDomainUnknown;
//	CComPtr<_AppDomain>         m_AppDomain;
//
//	CComPtr<_Assembly>          m_AppAssembly;
//	CComVariant                 m_AppVariant;
//	managedplugin_t()
//	{
//		this->m_PluginName              = L"";
//		this->m_PluginPath              = L"";
//		this->m_AppDomainSetupUnknown   = NULL;
//		this->m_AppDomainUnknown        = NULL;
//		this->m_AppDomain               = NULL;
//		this->m_AppAssembly             = NULL;
//		this->m_AppVariant              = NULL;
//	}
//};

void rclr_ms_create_domain( char ** appBaseDir ) {
    HRESULT hr;
#ifdef USE_COR_RUNTIME_HOST
    hr = pRuntimeInfo->GetInterface( CLSID_CorRuntimeHost,
        IID_PPV_ARGS( &pCorRuntimeHost ) );
    if (FAILED( hr )) {
        error( "Getting the CorRuntimeHost failed with message: %s\n", getComErrorMsg( hr ) );
        return;
    }
    // Get a pointer to the default AppDomain in the CLR.
    //hr = pCorRuntimeHost->GetDefaultDomain(&spAppDomainThunk);
    //hr = spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&spDefaultAppDomain));

#else
    hr = pRuntimeInfo->GetInterface( CLSID_CLRRuntimeHost, IID_PPV_ARGS( &pRuntimeHost ) );
    ICLRControl* pCLRControl = nullptr;
    hr = pRuntimeHost->GetCLRControl( &pCLRControl );

    LPCWSTR appDomainManagerTypename = L"SampleAppDomainManager.CustomAppDomainManager";
    LPCWSTR assemblyName = L"SampleAppDomainManager";
    hr = pCLRControl->SetAppDomainManagerType( assemblyName, appDomainManagerTypename );
    pMetaHost->

        pClrRuntimeHost->GetCLRControl( &pClrControl );
    pClrControl->GetCLRManager(
#endif
        //****************************
        // ACKNOWLEDGEMENT:
        // the initialisation code is in part inspired by the work of 'atom0s' from the following page:
        // http://atom0s.wordpress.com/2012/11/30/native-clr-hosting-with-separate-appdomains-per-module/
        //****************************

        // Create a new AppDomain setup object for the new domain..
        //pCorRuntimeHost->CreateDomainSetup( &plugin->m_AppDomainSetupUnknown );
        IUnknown * appDomainSetupUnknown;
    hr = pCorRuntimeHost->CreateDomainSetup( &appDomainSetupUnknown );
    if (FAILED( hr )) {
        error( "CreateDomainSetup failed with message: %s\n", getComErrorMsg( hr ) );
        return;
    }

    // Fill some basic structure info about this domain..
    //plugin->m_AppDomainSetup = plugin->m_AppDomainSetupUnknown;
    IAppDomainSetup * appDomSetup = NULL;
    hr = appDomainSetupUnknown->QueryInterface( __uuidof(IAppDomainSetup), (void**)&appDomSetup );
    if (FAILED( hr )) {
        error( "Getting IAppDomainSetup failed with message: %s\n", getComErrorMsg( hr ) );
        return;
    }
    if (appDomSetup == NULL) {
        error( "Getting IAppDomainSetup returned NULL\n" );
        return;
    }

    //plugin->m_AppDomainSetup->put_ApplicationBase( CComBSTR( "F:\\bin\\R\\library\\rClr\\libs\\x64\\" ) );
    //plugin->m_AppDomainSetup->put_ShadowCopyFiles( CComBSTR( "false" ) );
    //plugin->m_AppDomainSetup->put_ApplicationName( CComBSTR( "rClr" ) );
    bstr_t appBaseDirPath( appBaseDir[0] );
    bstr_t shadowCopyFiles( "false" );
    bstr_t appName( "rClr" );

    appDomSetup->put_ApplicationBase( appBaseDirPath );
    appDomSetup->put_ShadowCopyFiles( shadowCopyFiles );
    appDomSetup->put_ApplicationName( appName );

    // Create the new AppDomain..
    // pCorRuntimeHost->CreateDomainEx( L"CLR Facade", plugin->m_AppDomainSetupUnknown, NULL, &plugin->m_AppDomainUnknown );
    IUnknown * appDomainUnknown;
    hr = pCorRuntimeHost->CreateDomainEx( L"CLR Facade", appDomainSetupUnknown, NULL, &appDomainUnknown );
    if (FAILED( hr )) {
        error( "CreateDomainEx failed with message: %s\n", getComErrorMsg( hr ) );
        return;
    }

    // Obtain the actual AppDomain object..
    //plugin->m_AppDomainUnknown->QueryInterface( __uuidof( mscorlib::_AppDomain ), (void**)&plugin->m_AppDomain );
    _AppDomain * appDomain;
    hr = appDomainUnknown->QueryInterface( __uuidof(mscorlib::_AppDomain), (void**)&appDomain );
    if (FAILED( hr )) {
        error( "Failed to retrieve _AppDomain: %s\n", getComErrorMsg( hr ) );
        return;
    }

    //plugin->m_AppDomain->Load_2( pwzDllName, &plugin->m_AppAssembly );
    //&m_AppDomain->Load_2( pwzDllName, &plugin->m_AppAssembly );
    //spAssembly = &plugin->m_AppAssembly;
    //// Load the .NET assembly.
    spDefaultAppDomain = appDomain;

    bstr_t bstrAssemblyName( pwzDllName );
    //bstr_t bstrAssemblyName(L"F:\\bin\\R\\library\\rClr\\libs\\x64\\ClrFacade.dll");
    //hr = spDefaultAppDomain->AppendPrivatePath(L"F:\\bin\\R\\library\\rClr\\libs\\x64");
    //BSTR baseDir;
    //hr = spDefaultAppDomain->get_BaseDirectory(&baseDir);
    // Load the plugin module into the AppDomain..
    hr = spDefaultAppDomain->Load_2( bstrAssemblyName, &spAssembly );
    if (FAILED( hr )) {
        error( "Failed to load the assembly '%s' from disk: %s. App base dir is %s\n", bstr_to_c_string( &bstrAssemblyName ), getComErrorMsg( hr ), appBaseDir[0] );
        return;
    }
    hr = spAssembly->GetType_2( bstrClassName, &spTypeClrFacade );
    if (FAILED( hr )) {
        error( "Getting the type of the 'CLR facade' failed with message: %s\n", getComErrorMsg( hr ) );
        return;
    }
}

HRESULT rclr_ms_get_facade_typeref( _Type ** spType ) {
    *spType = _TypePtr( spTypeClrFacade );
    return S_OK;
}

char * bstr_to_c_string( bstr_t * src ) {
#ifndef  UNICODE                     // r_winnt
    return (char*)(src);
#else
    // Convert the wchar_t string to a char* string.
    // see http://msdn.microsoft.com/en-us/library/ms235631.aspx
    size_t origsize = wcslen( *src ) + 1;
    size_t convertedChars = 0;
    const size_t newsize = origsize * 2;
    char *nstring = new char[newsize];
    wcstombs_s( &convertedChars, nstring, newsize, *src, _TRUNCATE );
    return nstring;
#endif

}

char * getComErrorMsg( HRESULT hr ) {
    bstr_t errMsg( _com_error( hr ).ErrorMessage() ); // FIXME: surely there is a way to retrieve the exception message, no?
    return bstr_to_c_string( &errMsg );
}

VARIANT * rclr_ms_create_safe_array( VARIANT ** params, int paramsArgLength ) {
    SAFEARRAY * psaMethodArgs = SafeArrayCreateVector( VT_VARIANT, 0, paramsArgLength );
    for (LONG i = 0; i < paramsArgLength; i++)
    {
        if (params[i] != NULL)
            SafeArrayPutElement( psaMethodArgs, &i, params[i] ); // TOCHECK: defaults in the array are VT_EMPTY?
    }
    return rclr_ms_create_vt_array( psaMethodArgs, VT_ARRAY | VT_VARIANT );
}

// This is a workaround for https://r2clr.codeplex.com/workitem/70. 
// SafeArrayPutElement on a VT_ARRAY | VT_R8 for instance will trigger a deep copy; 
// safe but onerous with CPU and memory.
HRESULT add_params_as_new_safearray( SAFEARRAY * methodCallArg, LONG* rgIndices, VARIANT ** params, int paramsArgLength ) {
    HRESULT hr;
    VARIANT * psafe_array;
    psafe_array = rclr_ms_create_safe_array( params, paramsArgLength );
    hr = SafeArrayPutElement( methodCallArg, rgIndices, psafe_array );
    VariantClear( psafe_array );
    return hr;
}

SAFEARRAY * rclr_ms_create_constructor_parameters( VARIANT * class_name, VARIANT ** params, int paramsArgLength )
{
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( VT_VARIANT, 0, 2 );
    LONG index = 0;
    HRESULT hr = SafeArrayPutElement( psaStaticMethodArgs, &index, class_name );
    index = 1;
    hr = add_params_as_new_safearray( psaStaticMethodArgs, &index, params, paramsArgLength );
    return psaStaticMethodArgs;
}

SAFEARRAY * rclr_ms_create_common_call_parameters( VARIANT * obj_or_class_name, char * mnam, VARIANT ** params, int paramsArgLength )
{
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( VT_VARIANT, 0, 3 );
    LONG index = 0;
    HRESULT hr = SafeArrayPutElement( psaStaticMethodArgs, &index, obj_or_class_name );
    index = 1;
    //create_BSTR(&vMname, mnam);
    variant_t vName( mnam );
    hr = SafeArrayPutElement( psaStaticMethodArgs, &index, &vName );
    index = 2;
    hr = add_params_as_new_safearray( psaStaticMethodArgs, &index, params, paramsArgLength );
    //clear_BSTR(&vMname);
    return psaStaticMethodArgs;
}

SAFEARRAY * rclr_ms_create_method_call_parameters( CLR_OBJ * objptr, char * mnam, VARIANT ** params, int paramsArgLength )
{
    return rclr_ms_create_common_call_parameters( objptr, mnam, params, paramsArgLength );
}

SAFEARRAY * rclr_ms_create_static_method_call_parameters( char * assemblyQualifiedTypeName, char * mnam, VARIANT ** params, int paramsArgLength )
{
    //VARIANT tname;
    SAFEARRAY * result;
    //create_BSTR(&tname, assemblyQualifiedTypeName);
    variant_t tname( assemblyQualifiedTypeName );
    result = rclr_ms_create_common_call_parameters( &tname, mnam, params, paramsArgLength );
    //clear_BSTR(&tname);
    return result;
}

VARIANT * rclr_ms_create_vt_array( SAFEARRAY * safeArray, VARTYPE vartype ) {
    VARIANT * pvt = new variant_t(); // TODO: I suspect this leads to a memory leak if it ends up in an array itself. https://r2clr.codeplex.com/workitem/70
    VariantInit( pvt );
    pvt->vt = vartype;
    pvt->parray = safeArray;
    //_variant_t variant;
    // variant.Attach(vt);
    return pvt;
}

SAFEARRAY * create_array_one_string( char * strArg ) {
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( VT_VARIANT, 0, 1 );
    LONG index = 0;
    bstr_t bstringArg( strArg );
    variant_t vtStringArg( bstringArg );
    HRESULT hr = SafeArrayPutElement( psaStaticMethodArgs, &index, &vtStringArg );
    return psaStaticMethodArgs;
}

SAFEARRAY * create_array_strings( char ** values, int length ) {
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( VT_BSTR, 0, length );
    for (LONG i = 0; i < length; i++)
    {
        // expletive aimed at COM [here]...
        //VARIANT aStr;
        //VariantInit(&aStr);
        //V_VT(&aStr) = VT_BSTR;
        ////V_BSTR(&aStr) = ::SysAllocString(bstr_t(values[i]));
        //V_BSTR(&aStr) = ::SysAllocString(L"a");
        //SafeArrayPutElement(psaStaticMethodArgs, &i, &aStr);
        //VariantClear(&aStr);

        // CURSE!
        // bstr_t bstringArg(values[(int)i]);
        // variant_t vtStringArg(bstringArg);
        // SafeArrayPutElement(psaStaticMethodArgs, &i, &vtStringArg);

        bstr_t * bstringArg = new bstr_t( (const char *)values[i] );
        SafeArrayPutElement( psaStaticMethodArgs, &i, bstringArg->GetBSTR() );
        delete(bstringArg);
    }
    return psaStaticMethodArgs;
}

// Trying a macro by inference from mono's mono_array_set; very shallow understanding of it...
#define create_safearray_oftype(vtype, values, length)	\
	do {	\
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(vtype, 0, length); \
	for (LONG i = 0; i < length; i++)\
{\
	SafeArrayPutElement(psaStaticMethodArgs, &i, &values[i]);\
}\
	return psaStaticMethodArgs;\
	} while (0)

SAFEARRAY * create_safe_array( VARIANT ** values, int length ) {
    create_safearray_oftype( VT_VARIANT, values, length );
    //SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, length); 
    //for (LONG i = 0; i < length; i++)
    //{
    //	SafeArrayPutElement(psaStaticMethodArgs, &i, values[i]);
    //}
    //return psaStaticMethodArgs;
}

SAFEARRAY * create_array_double( double * values, int length )
{
    create_safearray_oftype( VT_R8, values, length );
    //SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_R8, 0, length); 
    //for (LONG i = 0; i < length; i++)
    //{
    //	variant_t vtArg = new variant_t(values[i]);
    //	SafeArrayPutElement(psaStaticMethodArgs, &i, &values[i]);
    //}
    //return psaStaticMethodArgs;
}

SAFEARRAY * create_array_dates( double * values, int length )
{
    create_safearray_oftype( VT_DATE, values, length );
}

SAFEARRAY * create_array_bytes( unsigned char * values, int length )
{
    create_safearray_oftype( VT_UI1, values, length );
}

SAFEARRAY * create_array_int_or_bool( int * values, int length, VARTYPE vtype )
{
    SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector( vtype, 0, length );
    for (LONG i = 0; i < length; i++)
    {
        //variant_t vtArg = vtype == VT_BOOL ? new variant_t((bool)values[i]) : new variant_t(values[i]) ;
        SafeArrayPutElement( psaStaticMethodArgs, &i, &values[i] );
    }
    return psaStaticMethodArgs;
}

SAFEARRAY * create_array_int( int * values, int length )
{
    return create_array_int_or_bool( values, length, VT_I4 );
}

SAFEARRAY * create_array_bool( int * values, int length )
{
    return create_array_int_or_bool( values, length, VT_BOOL );
}


#endif
