#include "rClr.h"


int is_microsoft_clr() {
#ifdef MS_CLR 
	return 1;
#else
	return 0;
#endif
}

/////////////////////////////////////////
// Initialisation and disposal of the CLR
/////////////////////////////////////////



void rclr_create_domain(char ** appBaseDir, char ** filename, int * mono_debug) {

#ifdef MONO_CLR 
	rclr_mono_create_domain(filename[0], *mono_debug);
#elif MS_CLR
	start_ms_clr();
	rclr_ms_create_domain(appBaseDir);
#endif
}

void rclr_load_assembly(char ** filename) {
#ifdef MONO_CLR
	rclr_mono_load_assembly( filename );
#elif MS_CLR
	variant_t vtResult;
	rclr_ms_load_assembly( filename, &vtResult);
#endif
}

void rclr_shutdown_clr()
{
#ifdef MONO_CLR
	mono_jit_cleanup (domain);
#elif MS_CLR
	ms_rclr_cleanup();
#endif
}

static void clr_object_finalizer(SEXP clrSexp) {
	ClrObjectHandle * clroh_ptr;
	if (TYPEOF(clrSexp)==EXTPTRSXP) {
#ifdef MONO_CLR
		clroh_ptr = (ClrObjectHandle*)R_ExternalPtrAddr(clrSexp);
		mono_gchandle_free(clroh_ptr->handle);
		free(clroh_ptr);
#elif MS_CLR
		CLR_OBJ * objptr;
		clroh_ptr = static_cast<ClrObjectHandle*> (R_ExternalPtrAddr(clrSexp));
		objptr = clroh_ptr->objptr;
		if (objptr->vt == VT_EMPTY)
			warning("clr_object_finalizer called on a variant of type VT_EMPTY\n");
		else
			delete objptr;
#endif
	}
	// TODO else throw exception?
}


#ifdef MONO_CLR
MonoDomain * get_domain() { return domain; }
MonoAssembly * get_assembly() { return assembly; }
MonoImage * get_image() { return image; }


MonoMethod * rclr_mono_get_method (MonoClass * klass, char * mnam, int param_counts) {
	return mono_class_get_method_from_name(klass, mnam, param_counts);
}


CLR_OBJ * rclr_mono_create_object (MonoDomain *domain, MonoImage *image, char * ns_qualified_typename, SEXP ctorParams) {	
	MonoMethod * method = rclr_mono_get_method( spTypeClrFacade, "CreateInstance", 2);
	MonoArray* ctor_params = create_array_object(build_method_parameters(ctorParams), Rf_length(ctorParams));
	MonoObject * exception, *result;
	void** ci_p = (void**)malloc(2*sizeof(void*)); // The array of parameters for CreateInstance
	ci_p[0] = create_mono_string(ns_qualified_typename);
	ci_p[1] = ctor_params;
	result = mono_runtime_invoke (method, NULL, ci_p, &exception);
	print_if_exception(exception);
	free(ci_p);
	// TODO: how are transient Mono arrays and the like freed? should they be?
	// g_free(ctor_params);
	return result;
}

CLR_OBJ * rclr_mono_invoke_method_stringarg( MonoMethod * method, char * meth_arg) { 
	CLR_OBJ * result;
	MonoObject * exception = NULL;
	MonoString * monoStrArg = create_mono_string(meth_arg);
	result = mono_runtime_invoke (method, NULL, (void**)(&monoStrArg), &exception);
	print_if_exception(exception);
	return result;
}

CLR_OBJ * create_object (MonoDomain *domain, MonoImage *image, char * name_space, char * type_short_name)
{
	MonoClass *klass;
	CLR_OBJ * obj;

	klass = mono_class_from_name (image, name_space, type_short_name);
	if (!klass) {
		Rprintf("Can't find %s in assembly %s\n", type_short_name, mono_image_get_filename (image));
		return NULL;
	}

	obj = mono_object_new (domain, klass);
	/* mono_object_new () only allocates the storage: 
	* it doesn't run any constructor. Tell the runtime to run
	* the default argumentless constructor.
	*/
	mono_runtime_object_init (obj);

	//access_valuetype_field (obj);
	//access_reference_field (obj);

	//call_methods (obj);
	//more_methods (domain);
	return obj;
}

CLR_OBJ * get_property_value( CLR_OBJ * obj, const char * property_name ) {

	MonoClass *klass;
	MonoMethod *method = NULL;
	MonoProperty *prop;

	klass = mono_object_get_class (obj);
	prop = mono_class_get_property_from_name (klass, property_name);
	return mono_property_get_value (prop, obj, NULL, NULL);
}

CLR_OBJ * rclr_mono_get_current_object_direct() {
	MonoMethod * method = rclr_mono_get_method( spTypeClrFacade, "get_CurrentObject", 0);
	MonoObject * exception;
	CLR_OBJ * result;
	result = mono_runtime_invoke(method, NULL, NULL, &exception);
	print_if_exception(exception);
	return result;
}


void print_to_R(MonoString * monoString, void f(const char *, ...), const char * msg_container)
{
	char * msg;
	msg = mono_string_to_utf8(monoString);
	f(msg_container, msg);
	mono_free(msg);
}

void print_as_error(MonoString * monoString, const char * msg_container)
{
	print_to_R(monoString, error, msg_container);
}

void print_as_warning(MonoString * monoString, const char * msg_container)
{
	print_to_R(monoString, warning, msg_container);
}

void print_exception(CLR_OBJ * exception, char * property_name)
{
	print_to_R((MonoString*)get_property_value(exception, property_name), Rprintf, "%s\n");
}

MonoString * rclr_mono_get_last_clr_exception() {
	MonoMethod * method = rclr_mono_get_method(spTypeClrFacade, "get_LastCallException", 0);
	MonoObject * exception;
	CLR_OBJ * result;
	result = mono_runtime_invoke(method, NULL, NULL, &exception);
	return (MonoString*)result;
	//return result;
	//// Work around https://r2clr.codeplex.com/workitem/67
	//	hr = spType->InvokeMember_3(getLastException, static_cast<BindingFlags>(
	//		BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
	//		NULL, vtEmpty, NULL, vtResult);
	//	bstr_t tmpBstr(vtResult->bstrVal);
	//	if (std::string((char*)vtResult->bstrVal) == std::string(""))
	//		error("%s", "Failure in rclr_ms_call_static_method, but could not retrieve an error message");
	//	else
	//		// There seems to be a limit to the size printed by error, which is a problem for long stack traces
	//		// Nevertheless cannot use Rprintf here or this breaks the try/error patterns e.g. with the testthat package...
	//		// Rprintf("%s", bstr_to_c_string( &tmpBstr ));
	//		error("%s", bstr_to_c_string(&tmpBstr));
}

void print_if_exception( CLR_OBJ * exception )
{
	CLR_OBJ * inner_exception;
	if (exception) {
		//Rprintf("Exception thrown in the method invocation\n");
		//print_exception(exception, "Message");
		//print_exception(exception, "StackTrace");
		//inner_exception = get_property_value(exception, "InnerException");
		//if (inner_exception) {
		//	print_exception(inner_exception, "Message");
		//	print_exception(inner_exception, "StackTrace");
		//}
		print_as_error(rclr_mono_get_last_clr_exception(), "%s");
	}
}

const char * getKlassName(CLR_OBJ * obj)
{
	return mono_class_get_name(mono_object_get_class(obj));
}

SEXP rclr_mono_call_method_with_exception(const char * mnam, CLR_OBJ * obj, MonoClass * klass, SEXP methodParams, CLR_OBJ * exception) {
	MonoMethod * methodCallInstanceMethod = rclr_mono_get_method( spTypeClrFacade, "CallInstanceMethod", 3);
	CLR_OBJ * result;
	//char * debug_test;
	int paramCount = Rf_length(methodParams);
	void ** params = build_method_parameters(methodParams);
	void* static_mparams[3]; // = void*[3]; // TOCHECK
	MonoArray* methParams = create_array_object(params, paramCount);
	//debug_test = getKlassName(obj);
	static_mparams[0] = obj;
	static_mparams[1] = create_mono_string((char*)mnam);
	static_mparams[2] = methParams;
	result = mono_runtime_invoke (methodCallInstanceMethod, NULL, static_mparams, &exception);
	//debug_test = getKlassName(result);
	print_if_exception(exception);
	return clr_obj_mono_convert_to_SEXP(result);
}

SEXP rclr_mono_call_method_with_exception_directcall(const char * mnam, CLR_OBJ * obj, MonoClass * klass, SEXP methodParams, CLR_OBJ * exception) {
	CLR_OBJ * result;
	MonoMethod * method = mono_class_get_method_from_name(klass, mnam, Rf_length(methodParams));
	if (!method)
		error_return("rclr_mono_call_method_with_exception_directcall: cannot find valid method (name and/or parameters)");

	exception = NULL;

	//	p = mono_string_to_utf8 (str);
	//	printf ("Values of str/val from Values () are: %s/%d\n", p, val);
	//	/* we need to free the result from mono_string_to_utf8 () */
	//	mono_free (p);

	result = mono_runtime_invoke (method, obj, build_method_parameters(methodParams), &exception);
	print_if_exception(exception);

	if (!result) return R_NilValue;

	return clr_obj_mono_convert_to_SEXP(result);

}

SEXP rclr_mono_call_method(const char *mnam, CLR_OBJ * obj, SEXP methodParams )
{
	SEXP result;
	CLR_OBJ * exception = NULL;
	//MonoMethod *method = NULL;
	MonoClass *klass;
	klass = mono_object_get_class(obj);
	if (!klass)
		error_return("rclr_mono_call_method: cannot determine object class");

#ifdef DEBUG
	//Rprintf("Seeking method %s, with %i parameters, on class %s\n", mnam, Rf_length(p), mono_type_get_name(mono_class_get_byref_type(klass)));
#endif

	result = rclr_mono_call_method_with_exception( mnam, obj, klass, methodParams, exception );
	//print_if_exception(exception); done by rclr_mono_call_method. Should it be?
	return result;
}

// Trying a macro by inference from mono's mono_array_set; very shallow understanding of it...
#define create_array_oftype(type, mono_get_some_class, values, length)	\
	do {	\
	int j; \
	MonoArray* monoArray = mono_array_new(domain, mono_get_some_class(), length); \
	for (j = 0; j < length; j++) \
{ \
	mono_array_set(monoArray, type, j, values[j]); \
} \
	return monoArray; \
	} while (0)


#define create_array_from_monoarray(type, monoarray)	\
	do {	\
	int j; \
	type * values; \
	int length = mono_array_length(monoarray); \
	values = (type*)malloc(length*sizeof(type)); \
	for (j = 0; j < length; j++) \
{ \
	values[j] = mono_array_get(monoarray, type, j); \
} \
	return values; \
	} while (0)

double * create_array_double_from_monoarray( MonoArray* monoarray )
{
	create_array_from_monoarray(double, monoarray);
}

MonoArray * create_array_double( double * values, int length )
{
	create_array_oftype(double, mono_get_double_class, values, length);
}

MonoArray * create_array_int(int * values, int length)
{
	create_array_oftype(int, mono_get_int32_class, values, length);
}

MonoArray * create_array_bytes(unsigned char * values, int length)
{
	create_array_oftype(unsigned char, mono_get_byte_class, values, length);
}

MonoArray * create_array_bool( int * values, int length )
{
	create_array_oftype(int, mono_get_boolean_class, values, length);
}

MonoArray * create_array_object( void ** values, int length )
{
	create_array_oftype(void*, mono_get_object_class, values, length);
}

MonoString * create_mono_string(char * str) {
	const char * str2 = STR_DUP(str);
	return mono_string_new (domain, str2);
}

MonoObject * create_mono_double(double * val) {
	return mono_value_box(domain, mono_get_double_class(), val);
}

MonoObject * create_mono_int32(int * val) {
	return mono_value_box(domain, mono_get_int32_class(), val);
}

MonoObject * create_mono_int64(long * val) {
	return mono_value_box(domain, mono_get_int64_class(), val);
}

MonoObject * create_mono_bool(int * val) {
	return mono_value_box(domain, mono_get_boolean_class(), val);
}

MonoObject * create_mono_intptr(size_t * val) {
	return mono_value_box(domain, mono_get_intptr_class(), val);
}

#elif MS_CLR

void start_ms_clr() {
	HRESULT hr;
	hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&pMetaHost));
	if (FAILED(hr))
	{
		error("CLRCreateInstance failed");
	}
	hr = pMetaHost->GetRuntime(pszVersion, IID_PPV_ARGS(&pRuntimeInfo));
	if (FAILED(hr))
	{
		error("ICLRMetaHost::GetRuntime failed");
	}
	BOOL fLoadable;
	hr = pRuntimeInfo->IsLoadable(&fLoadable);
	if (FAILED(hr))
	{
		error("ICLRRuntimeInfo::IsLoadable failed");
	}

	if (!fLoadable)
	{
		error(".NET runtime cannot be loaded");
	}
	hr = pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, 
		IID_PPV_ARGS(&pClrRuntimeHost));
	if (FAILED(hr))
	{
		error("ICLRRuntimeInfo::GetInterface failed w/hr 0x%08lx\n", hr);
		goto Cleanup;
	}

	// Start the CLR.
	hr = pClrRuntimeHost->Start();
	if (FAILED(hr))
	{
		error("CLR failed to start w/hr 0x%08lx\n", hr);
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
}

#endif


/////////////////////////////////////////
// Functions with R specific constructs, namely SEXPs
/////////////////////////////////////////



SEXP make_int_sexp(int n, int * values) {
	SEXP result ;
	long i = 0;
	int * int_ptr;
	PROTECT(result = NEW_INTEGER(n));
	int_ptr = INTEGER_POINTER(result);
	for(i = 0; i < n ; i++ ) {
		int_ptr[i] = values[i];
	}
	UNPROTECT(1);
	return result;
}

SEXP make_numeric_sexp(int n, double * values) {
	SEXP result ;
	long i = 0;
	double * dbl_ptr;
	PROTECT(result = NEW_NUMERIC(n));
	dbl_ptr = NUMERIC_POINTER(result);
	for(i = 0; i < n ; i++ ) {
		dbl_ptr[i] = values[i];
	}
	UNPROTECT(1);
	return result;
}

SEXP make_char_sexp(int n, char ** values) {
	SEXP result ;
	long i = 0;
	PROTECT(result = NEW_CHARACTER(n));
	for(i = 0; i < n ; i++ ) {
		SET_STRING_ELT( result, i, mkChar( (const char*)values[i]));
	}
	UNPROTECT(1);
	return result;
}

SEXP make_char_sexp_one(char * values) {
	return make_char_sexp(1, &values);
}

SEXP make_class_from_numeric(int n, double * values, int numClasses, char ** classnames) {
	SEXP result;
	result = make_numeric_sexp(n, values);
	PROTECT(result);
	setAttrib(result, R_ClassSymbol, make_char_sexp(numClasses, classnames)); 
	UNPROTECT(1);
	return result;
}


// FIXME probably DEPRECATED
SEXP make_date_sexp(int n, double * values) { 
	char * classname = "Date";
	return make_class_from_numeric(n, values, 1, &classname);
}

SEXP make_POSIXct_sexp(int n, double * values) {
	SEXP result;
	char ** classname = (char**)malloc(sizeof(char*)*2);
	classname[0] = "POSIXct"; classname[1] = "POSIXt";
	result = make_class_from_numeric(n, values, 2, classname);
    if (NAMED(result) == 2)
	PROTECT(result = duplicate(result)); // TOCHECK: why??
    else
	PROTECT(result);
	// FOR INFO: following creates an access violation
	//setAttrib(result, mkChar("tzone"), mkChar("UTC")); 
	setAttrib(result, make_char_sexp_one("tzone"), make_char_sexp_one("UTC")); 
	UNPROTECT(1);
	free(classname);
	return result;
}



#ifdef MONO_CLR
SEXP make_bool_sexp(int n, MonoBoolean * values) {
#elif MS_CLR
SEXP make_bool_sexp(int n, bool * values) {
#endif
	SEXP result ;
	long i = 0;
	int * intPtr;
	PROTECT(result = NEW_LOGICAL(n));
	intPtr = LOGICAL_POINTER(result);
	for(i = 0; i < n ; i++ ) {
		intPtr[i] = 
			// found many tests in the mono codebase such as below. Trying by mimicry then.
			// 		if (*(MonoBoolean *) mono_object_unbox(res)) {
			values[i]; // See http://r2clr.codeplex.com/workitem/34;
	}
	UNPROTECT(1);
	return result;
}

SEXP make_uchar_sexp(int n, unsigned char * values) {
	SEXP result;
	long i = 0;
	Rbyte * bPtr;
	PROTECT(result = NEW_RAW(n));
	bPtr = RAW_POINTER(result);
	for (i = 0; i < n; i++) {
		bPtr[i] = values[i];
	}
	UNPROTECT(1);
	return result;
}

SEXP make_char_single_sexp(const char* str) { // FIXME: does that not duplicate mkString??
	return mkString(str);
	//SEXP ans;
	//PROTECT(ans = allocVector(STRSXP, 1));
	//SET_STRING_ELT(ans, 0, str == NULL ? NA_STRING : mkChar(str));
	//UNPROTECT(1);
	//return ans;
}

// Try to work around issue rclr:33
SEXP r_get_object_direct() {
	SEXP result = NULL;
#ifdef MS_CLR
	CLR_OBJ obj; // needs this: otherswise null pointer if using only objptr. 
	rclr_ms_get_current_object_direct(&obj);
	result = clr_obj_ms_convert_to_SEXP(obj);
#elif MONO_CLR
	//error("%s", "r_get_object_direct not yet implemented for Mono");
	CLR_OBJ * objptr = NULL;
	objptr = rclr_mono_get_current_object_direct();
	result = clr_obj_mono_convert_to_SEXP(objptr);
#endif
	return result;
}

SEXP r_create_clr_object( SEXP p ) {
	SEXP methodParams, result;
	CLR_OBJ * objptr = NULL;
	char* ns_qualified_typename = NULL;
	R_len_t argLength;
	p=CDR(p); /* skip the first parameter: function name*/
	get_FullTypeName(p, &ns_qualified_typename); p=CDR(p);
	methodParams = p;
	argLength = Rf_length(methodParams);
#ifdef MS_CLR
	HRESULT hr;
	CLR_OBJ obj; // needs this: otherswise null pointer if using only objptr. 
	VARIANT ** params = build_method_parameters(methodParams);
	hr = rclr_ms_create_object(ns_qualified_typename, params, argLength, &obj);
	free_variant_array(params, argLength);
	if (FAILED(hr))
	{
		error("Failed to create a new object of class %s", ns_qualified_typename);
		result = R_NilValue;
	}
	else 
	{
		result = clr_obj_ms_convert_to_SEXP(obj);
	}
#elif MONO_CLR
	objptr = rclr_mono_create_object (domain, get_image(), ns_qualified_typename, methodParams);
	result = clr_object_to_SEXP(objptr);
#endif
	free(ns_qualified_typename);
	return result;
}

SEXP r_get_null_reference() {
	return R_MakeExternalPtr(0, R_NilValue, R_NilValue);
}

SEXP r_get_type_name(SEXP clrObj) {
#ifdef MONO_CLR
	return rclr_mono_get_type_name(clrObj);
#elif MS_CLR
	return rclr_ms_get_type_name(clrObj);
#endif
}

SEXP r_show_args(SEXP args)
{
	const char *name;
	int i, j, nclass;
	SEXP el, names, klass;
	int nargs, nnames;
	args = CDR(args); /* skip 'name' */

	nargs = Rf_length(args);
	for(i = 0; i < nargs; i++, args = CDR(args)) {
		name = 
			isNull(TAG(args)) ? "<unnamed>" : CHAR(PRINTNAME(TAG(args)));
		el = CAR(args);
		Rprintf("[%d] '%s' R type %s, SEXPTYPE=%d\n", i+1, name, type2char(TYPEOF(el)), TYPEOF(el));
		Rprintf("[%d] '%s' length %d\n", i+1, name, LENGTH(el));
		names = getAttrib(el, R_NamesSymbol);
		nnames = Rf_length(names);
		Rprintf("[%d] names of length %d\n", i+1, nnames);		
		//for(j = 0; j < nnames; j++, names = CDR(names))
		//{
		//	name=CAR(names);
		//Rprintf("[%d] %s\n", i+1, CHAR(STRING_ELT(el, 0)), );		
		//}
		klass = getAttrib(el, R_ClassSymbol);
		nclass = length(klass);
		for (j = 0; j < nclass; j++) {
			Rprintf("[%d] class '%s'\n", i+1, CHAR(STRING_ELT(klass, j)) );
		}
	}
	return(R_NilValue);
}

SEXP r_get_sexp_type(SEXP par) {
	SEXP p = par, e;
	int typecode;
	p=CDR(p); e=CAR(p);
	typecode = TYPEOF(e);
	return make_int_sexp(1, &typecode);
}

SEXP r_diagnose_parameters(SEXP p) {
	SEXP e, methodParams;
	p = CDR(p);; /* skip the first parameter: function name*/
	methodParams = p;

#ifdef MONO_CLR
	return rclr_mono_diagnose_method_parameters(methodParams);
#elif MS_CLR
	error_return("r_diagnose_parameters: not implemented for MS CLR");
	return R_NilValue;
#endif
}

SEXP r_call_static_method(SEXP p) {
	SEXP e, methodParams;
	const char *mnam;
	char* ns_qualified_typename = NULL; // My.Namespace.MyClass,MyAssemblyName

	p = CDR(p);; /* skip the first parameter: function name*/
	get_FullTypeName(p, &ns_qualified_typename); p = CDR(p);
	e = CAR(p); p = CDR(p); // get the method name.
	if (TYPEOF(e) != STRSXP || LENGTH(e) != 1)
	{
		free(ns_qualified_typename);
		error_return("r_call_static_method: invalid method name");
	}
	mnam = CHAR(STRING_ELT(e, 0));
	methodParams = p;

#ifdef MONO_CLR
	return rclr_mono_call_static_method(ns_qualified_typename, mnam, methodParams);
#elif MS_CLR
	HRESULT hr;
	R_len_t argLength = Rf_length(methodParams);
	VARIANT ** params = build_method_parameters(methodParams);
	CLR_OBJ result;
	hr = rclr_ms_call_static_method_tname(ns_qualified_typename, (char *)mnam, params, argLength, &result);
	free_variant_array(params, argLength);
	free(ns_qualified_typename);
	if (FAILED(hr))
	{
		return R_NilValue;
	}
	return clr_obj_ms_convert_to_SEXP(result);
#endif
}

SEXP r_call_method(SEXP par) {
	SEXP p = par, e, methodParams;
	const char *mnam = 0;
	CLR_OBJ * objptr=NULL;

	// retrieve the class name
	p=CDR(p); e=CAR(p); p=CDR(p);
	if (e==R_NilValue) 
		error_return("r_call_method: call on a NULL object");
	if (TYPEOF(e)==EXTPTRSXP) {
		objptr = GET_CLR_OBJ_FROM_EXTPTR(e);
	} else 
		error_return("r_call_method: invalid object parameter");

	if (!objptr)
		error_return("r_call_method: attempt to call a method of a NULL object.");

	e=CAR(p); p=CDR(p);
	if (TYPEOF(e)!=STRSXP || LENGTH(e)!=1)
		error_return("r_call_method: invalid method name");
	mnam = CHAR(STRING_ELT(e,0));
	methodParams = p;

#ifdef MONO_CLR
	return rclr_mono_call_method(mnam, objptr, methodParams );
#elif MS_CLR
	R_len_t argLength = Rf_length(methodParams);
	VARIANT ** params = build_method_parameters(methodParams);
	CLR_OBJ result;
	rclr_ms_call_method(objptr, (char *)mnam, params, argLength, &result);
	free_variant_array(params, argLength);
	return clr_obj_ms_convert_to_SEXP(result);
#endif

}

#ifdef MS_CLR
SEXP rclr_ms_get_type_name(SEXP clrObj) {
	CLR_OBJ * objptr = get_clr_object(clrObj);
	return make_char_single_sexp(get_type_full_name(objptr));
}

void get_array_variant(CLR_OBJ * pobj, SAFEARRAY ** array, int * n_ptr, LONG * plUbound)
{
	*array = pobj->parray;
	SafeArrayGetUBound(*array, 1, plUbound);
	(*n_ptr) = ((int)(*plUbound))+1;
}

SEXP clr_obj_ms_convert_to_SEXP(CLR_OBJ &obj) {
	CLR_OBJ * pobj = new CLR_OBJ(obj); // the allocation of a pobj here will hold a reference (handle) to the managed object.
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

	switch(pobj->vt) {
	case VT_UNKNOWN: // this is what we get for instance on a call to ModelRunner.get_Model. Maybe, when an interface type is returned rather than a class.
	case VT_DISPATCH:
		return clr_object_to_SEXP(pobj);
	case VT_ARRAY | VT_R8 :
		get_array_variant(pobj, &array, &n, &uBound);
		rVals = (double*)malloc(sizeof(double)*n);
		for(long i = 0; i < n ; i++ ) {
			SafeArrayGetElement(array, &i, &(rVals[i])); 
		}
		result = make_numeric_sexp(n, rVals);
		free(rVals);
		break;
	case VT_DATE | VT_ARRAY:
		//hr = rclr_ms_call_static_method_facade("DateTimeArrayToUtc", pobj, &date_time_utc);
		//get_array_variant(&date_time_utc, &array, &n, &uBound);
		get_array_variant(pobj, &array, &n, &uBound);
		rVals = (double*)malloc(sizeof(double)*n);
		for(long i = 0; i < n ; i++ ) {
			SafeArrayGetElement(array, &i, &(rVals[i])); 
			rVals[i] = COMdatetimeToPosixCt(rVals[i]);
		}
		result = make_POSIXct_sexp(n, rVals);
		free(rVals);
		break;
	case VT_ARRAY | VT_I2:
	case VT_ARRAY | VT_I4:
		get_array_variant(pobj, &array, &n, &uBound);
		iVals = (int*)malloc(sizeof(int)*n);
		for (long i = 0; i < n; i++) {
			SafeArrayGetElement(array, &i, &(iVals[i]));
		}
		result = make_int_sexp(n, iVals);
		free(iVals);
		break;
	case VT_ARRAY | VT_BOOL:
		get_array_variant(pobj, &array, &n, &uBound);
		bVals = (bool*)malloc(sizeof(bool)*n);
		for (long i = 0; i < n; i++) {
			SafeArrayGetElement(array, &i, &(bVals[i]));
		}
		result = make_bool_sexp(n, bVals);
		free(bVals);
		break;
	case VT_ARRAY | VT_BSTR:
		array = pobj->parray;
		SafeArrayGetUBound(array, 1, &uBound);	
		n = ((int)uBound)+1;
		BSTR bstrVal;
		strVals = (char**)malloc(sizeof(char*)*n);
		for(long i = 0; i < n ; i++ ) {
			SafeArrayGetElement(array, &i, &bstrVal); 
			bstr_t bstrtVal(bstrVal);
			strVals[i] = bstr_to_c_string( &bstrtVal );
		}
		result = make_char_sexp(n, strVals); 
		// TOCHECK: does R copy the char array, and do I need to free each (char*) too?
		free(strVals);
		break;
	case VT_ARRAY | VT_UI1:  // byte[]:  8209
		get_array_variant(pobj, &array, &n, &uBound);
		ucharVals = (unsigned char*)malloc(sizeof(unsigned char)*n);
		for (long i = 0; i < n; i++) {
			SafeArrayGetElement(array, &i, &(ucharVals[i]));
		}
		result = make_uchar_sexp(n, ucharVals);
		free(ucharVals);
		break;
		// The following would be handling .NET characters; however these are unicode characters, not ANSI characters.
		//case VT_ARRAY | VT_UI2:  // char[]:  8210. 
	case VT_EMPTY:
	case VT_NULL:
	case VT_VOID:
		result = R_NilValue;
		break;
	case VT_I2:
		iVals = (int*)malloc(sizeof(int)*n);
		iVals[0] = pobj->iVal;
		result = make_int_sexp(1, iVals);
		free(iVals);
		break;
	case VT_I4:
		iVals = (int*)malloc(sizeof(int)*n);
		iVals[0] = pobj->intVal;
		result = make_int_sexp(1, iVals);
		free(iVals);
		break;
	case VT_R4:
		rVals = (double*)malloc(sizeof(double)*n);
		rVals[0] = pobj->fltVal;
		result = make_numeric_sexp(n, rVals);
		free(rVals);
		break;
	case VT_R8:
		rVals = (double*)malloc(sizeof(double)*n);
		rVals[0] = pobj->dblVal;
		result = make_numeric_sexp(n, rVals);
		free(rVals);
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
			double rDateNumericValue = COMdatetimeToPosixCt(pobj->dblVal);
			result = make_POSIXct_sexp(n, &rDateNumericValue); 
			break;
		}
	case VT_BSTR:
		{
			strVals = (char**)malloc(sizeof(char*)*n);
			bstr_t tmpBstr(pobj->bstrVal);
			strVals[0] = bstr_to_c_string( &tmpBstr );
			result = make_char_sexp(n, strVals);
			free(strVals);
			break;
		}
	case VT_BOOL:
		bVals = (bool*)malloc(sizeof(bool)*n);
		bVals[0] = (bool)pobj->boolVal;
		result = make_bool_sexp(n, bVals);
		free(bVals);
		break;
	case VT_INT: 
		// This seems to be the case when an IntPtr is returned, native handle to an R.NET object.
		// However, this is not intVal that we need to retrieve. This is not well documented, but 
		// on 64 bits you would have nasty surprises. ullVal reports the same value as the IntPtr in the managed world
		return (SEXP) pobj->ullVal;
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
		error("clr_obj_ms_convert_to_SEXP: COM variant type code %d unsupported. Returning NULL", pobj->vt);
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

#endif

void get_FullTypeName( SEXP p, char ** tname) {
	SEXP e;
	e=CAR(p); /* second is the namespace */
	if (TYPEOF(e)!=STRSXP || LENGTH(e)!=1)
		error("get_FullTypeName: cannot parse type name: need a STRSXP of length 1");
	(*tname) = STR_DUP(CHAR(STRING_ELT(e,0))); // is all this really necessary? I recall trouble if using less, but this still looks over the top.
}

// called by clrTypeNameExtPtr
SEXP r_get_typename_externalptr(SEXP p) {
	SEXP e;
	CLR_OBJ * objptr = NULL;
	p=CDR(p); /* skip first parameter which is the function name */
	e=CAR(p); /* second is expected to be the external ptr in the R sense */
	if (TYPEOF(e)!=EXTPTRSXP)
		error("get_typename_externalptr: cannot get type name: need a EXTPTRSXP, but got a type %d", TYPEOF(e));
	objptr = GET_CLR_OBJ_FROM_EXTPTR(e);
#ifdef MS_CLR
	if ((objptr->vt != VT_DISPATCH) && (objptr->vt != VT_UNKNOWN))
	{
		error("Variant type of the CLR object is not a VT_DISPATCH or VT_UNKNOWN but: %d", objptr->vt);
		return mkChar("");
	}
#elif MONO_CLR
	// TODO?
#endif
	return make_char_single_sexp(get_type_full_name(objptr));
}


RCLR_BOOL check_POSIXct_has_simple_timezone(SEXP p, const char * tzchar) {
	SEXP t; const char * c;
	SEXP tzone = getAttrib(p, mkChar("tzone"));
	if (length(tzone) > 0) { 
		t = tzone;
		if(length(tzone) > 1) {
			// I actually did see a POSIXt with 3 elements c("","EST","EST") after class conversions. Not sure whether makes sense...
			error("Time zone on date-time object has more than one elements!");
			return FALSE_BOOL;
		}
		else {
			c = CHAR(STRING_ELT(t,0));
			//if ( (strcmp(c, "") == 0) || (strcmp(c, "GMT") == 0) || ( strcmp(c, "UTC") == 0) ) { // TOCHECK: is it case sensitive in R??
			if ( (strcmp(c, "") == 0) || (strcmp(c, "GMT") == 0) || ( strcmp(c, "UTC") == 0) ) { 
				tzchar = c; 
			}
			else {
				tzchar = "";
				error("Sorry, only UTC and GMT POSIXt is supported, not '%s'", c);
				return FALSE_BOOL;
			}
		}
	}
	return TRUE_BOOL;
}

RCLR_BOOL r_has_class(SEXP s, const char * classname) {
	SEXP klasses;
	R_xlen_t j;
	int n_classes;
	klasses = getAttrib(s, R_ClassSymbol);
	n_classes = length(klasses);
	if (n_classes > 0) {
		for(j = 0 ; j < n_classes ; j++)
			if( strcmp(CHAR(STRING_ELT(klasses,j)), classname) == 0)
				return TRUE_BOOL;
	}
	return FALSE_BOOL;
}

RCLR_BOOL r_is_date(SEXP s) {
	return r_has_class(s, "Date");
}

RCLR_BOOL r_is_POSIXlt(SEXP s) {
	return r_has_class(s, "POSIXlt");
}

RCLR_BOOL r_is_POSIXct(SEXP s) {
	return r_has_class(s, "POSIXct");
}


#ifdef MONO_CLR
void ** build_method_parameters(SEXP largs) {
#elif MS_CLR
VARIANT ** build_method_parameters(SEXP largs) {
#endif
	int i;
	int nargs = Rf_length(largs);
	int lengthArg = 0;
	SEXP args = largs;
	SEXP el;
	const char *name;

#ifdef MONO_CLR
	void ** mparams;
#endif
	if (nargs==0) {
		return NULL;
	}

#ifdef MONO_CLR
	mparams = (void**)malloc(nargs*sizeof(void*)); // TOCHECK
#elif MS_CLR
	VARIANT ** mparams = new VARIANT*[nargs];
#endif

	for(i = 0; args != R_NilValue; i++, args = CDR(args)) {
		name = isNull(TAG(args)) ? "" : CHAR(PRINTNAME(TAG(args)));
		el = CAR(args);
		mparams[i] = rclr_convert_element(el);
	}
	return mparams;
}

#if MS_CLR
void free_variant_array(VARIANT ** a, int size) {
	SAFEARRAY * a_ptr;
	for (int i = 0; i < size; i++)
	{
		// presume VT_UNKNOWN is a CLR Object that is referenced from 
		// R with an external pointer. Really bad idea to clear it.
		if((a[i]->vt != VT_UNKNOWN) && (a[i]->vt != VT_DISPATCH)) // without check on DISPATCH one gets the bug https://r2clr.codeplex.com/workitem/71 
		{
			// A relevant read is at http://social.msdn.microsoft.com/Forums/vstudio/en-US/7313928a-e386-406a-b03b-499dd350009f/how-to-determine-if-variant-contains-an-array
			//if(a[i]->vt & VT_ARRAY) {
			//	a_ptr = a[i]->parray;
			//	//VariantClear(a[i]);
			//	SafeArrayDestroy(a_ptr);
			//}
			//else
				VariantClear(a[i]);
		}
	}
	delete[] a;
}

CLR_OBJ * rclr_ms_convert_element_rdotnet(SEXP el)
{
	// The idea here is that we create a SymbolicExpression in C#, that the C# code will intercept
	CLR_OBJ vtResult; // needs this: otherswise null pointer if using only objptr. 
	HRESULT hr = S_FALSE;
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	LONG index = 0;
	int size_of_voidptr = (sizeof(void*));
	variant_t param((LONGLONG)el);
	param.vt = VT_I8;
	hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &param);
	bstr_t createSexpWrapper("CreateSexpWrapperMs");
	hr = spTypeClrFacade->InvokeMember_3(createSexpWrapper, static_cast<BindingFlags>(
		BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
		NULL, vtEmpty, psaStaticMethodArgs, &vtResult);
	if (FAILED(hr))	{
		error("%s", "Failure in rclr_convert_element_rdotnet");
	}
	SafeArrayDestroy(psaStaticMethodArgs);
	return new CLR_OBJ(vtResult);
}

HRESULT rclr_ms_create_clr_complex_direct(VARIANT * vtResult)
{
	return S_FALSE;
}

#endif

CLR_OBJ * rclr_convert_element_rdotnet(SEXP el)
{
	// The idea here is that we create a SymbolicExpression in C#, that the C# code will intercept
#if MS_CLR
	return rclr_ms_convert_element_rdotnet(el);
#else
    return rclr_mono_convert_element_rdotnet(el);
    // error("%s", "Failure in rclr_convert_element_rdotnet: nor implemented yet on Mono");
#endif
}

CLR_OBJ * rclr_convert_element( SEXP el ) 
{
	int j;
	int lengthArg = 0;
	const char *val;
	double * values;
	int * intVals;
	Rbyte * rawVals;
	Rcomplex * cpl;
	int element_type;
	const char * tzone;
	RCLR_BOOL is_date;
	RCLR_BOOL is_POSIXct;
#if MS_CLR
	double dval;
	VARIANT * result;
	VARTYPE vtype = VT_I4;
#elif MONO_CLR
	CLR_OBJ * parameter;
	void * result;
#endif

	element_type = TYPEOF(el);
	switch (element_type) {
	case S4SXP:
		result = get_clr_object(el);
		return (CLR_OBJ *)result;
	case VECSXP:
		// If this is a list of S4 objects CLR objects, then either we create a safe array of variants, 
		// or we call a C# function that creates an array of objects.
		//SAFEARRAY * safeArray = create_array_double(stringArray, LENGTH(el));
		//result = rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_BSTR);
		//result = build_method_parameters(el);
		//// TODO if this is a data frame
		// result = rclr_wrap_data_frame(el);
		if (r_is_POSIXlt(el))
			// TODO: do_asPOSIXct in R core datetime.c would be handy. Cannot access - seems hidden.
			error("POSIXlt objects not yet supported - only UTC/GMT POSIXct can be reliably converted");
		else
			return rclr_create_array_objects(el);
		break;
	}
	if (use_rdotnet)
	{
		return rclr_convert_element_rdotnet(el);
	}

	// Fallback code, to be superseded by C# code using R.NET, gradually.
	// 
	switch (element_type) {
	case REALSXP:
		lengthArg = LENGTH(el);
		values = REAL(el);
		is_date = r_is_date(el);
		is_POSIXct = r_is_POSIXct(el);
		tzone = NULL;
		if(is_POSIXct) {
			if(!check_POSIXct_has_simple_timezone(el, tzone)) {
				return NULL;
			}
		}
#if MS_CLR
		vtype = (is_date || is_POSIXct) ?  VT_DATE : VT_R8;
#endif
		if( lengthArg == 1)
		{
#ifdef MONO_CLR
			if (is_date) 
			{
				// NOTE : if using rclr_mono_call_static_method_tname_directcall we do not want to pass the argument as a mono boxed double
				// with create_mono_double(values); 
				// this does NOT work. It took me to step back from "intuition"
				// to notice that be it a DateTime or a Double, what you want to pass as parameters (void ** params)
				// is the unboxed object.
				parameter = rclr_mono_call_static_method_tname_directcall("Rclr.ClrFacade", "CreateDateFromREpoch", (void**)(&values), 1);

				// however if using rclr_mono_call_static_method_tname now using the ClrFacade, we have to pass the mono_double
				// parameter = rclr_mono_call_static_method_tname("Rclr.ClrFacade", "CreateDateFromREpoch", (void**)(&create_mono_double(values)), 1);

				result = parameter;
			}
			else if (is_POSIXct) 
			{
				parameter = rclr_mono_call_static_method_tname_directcall("Rclr.ClrFacade", "CreateDateFromRPOSIXct", (void**)(&values), 1);
				result = parameter;
			}
			else
			{
				result = create_mono_double(values);
			}
#elif MS_CLR
			dval = is_date ? DateRToCOMdatetime(values[0]) : (is_POSIXct ? PosixCtToCOMdatetime(values[0]) : values[0] );
			result = new variant_t(dval, vtype);
#endif
		}
		else if (lengthArg > 1 || lengthArg == 0)
		{
			int nVals = lengthArg;
#ifdef MONO_CLR
			MonoArray* monoArray = NULL;
			monoArray = create_array_double( values, lengthArg);
			if (is_date) {
				monoArray = (MonoArray*)rclr_mono_call_static_method_tname_directcall("Rclr.ClrFacade", "CreateDateArrayFromREpoch", (void**)(&monoArray), 1);
			}
			else if (is_POSIXct) 
			{
				monoArray = (MonoArray*)rclr_mono_call_static_method_tname_directcall("Rclr.ClrFacade", "CreateDateArrayFromRPOSIXct", (void**)(&monoArray), 1);
			}
			result = monoArray;
#elif MS_CLR
			double * dateval_offset;
			if (vtype == VT_DATE )
			{
				dateval_offset = (double*)malloc(sizeof(double)*nVals);
				for (j = 0; j < nVals; j++)
				{
					dateval_offset[j] = is_date ? DateRToCOMdatetime(values[j]) : (is_POSIXct ? PosixCtToCOMdatetime(values[j]) : values[j] );
				}
			}
			SAFEARRAY * safeArray = vtype == VT_DATE ? create_array_dates(dateval_offset, nVals) : create_array_double(values, nVals);
			result = rclr_ms_create_vt_array(safeArray, VT_ARRAY | vtype);
			if (vtype == VT_DATE)
				free(dateval_offset);
#endif
		}
		break;
	case LGLSXP:
		lengthArg = LENGTH(el);
		intVals = LOGICAL(el);
		if( lengthArg == 1)
		{
#ifdef MONO_CLR
			result = create_mono_bool(intVals);
#elif MS_CLR
			result = new variant_t((bool)LOGICAL(el)[0]);
#endif
		}
		else if (lengthArg > 1 || lengthArg == 0)
		{
			int nVals = lengthArg;
#ifdef MONO_CLR
			MonoArray* monoArray = create_array_bool(intVals, nVals);
			result = monoArray;
#elif MS_CLR
			SAFEARRAY * safeArray = create_array_bool(intVals, nVals);
			result = rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_BOOL);
#endif
		}
		break;
	case INTSXP:
		lengthArg = LENGTH(el);
		intVals = INTEGER(el);
		if( lengthArg == 1)
		{
#ifdef MONO_CLR
			result = create_mono_int32(intVals);
#elif MS_CLR
			result = new variant_t(intVals[0]);
#endif
		}
		else if (lengthArg > 1 || lengthArg == 0)
		{
			int nVals = lengthArg;
#ifdef MONO_CLR
			MonoArray* monoArray = create_array_int( intVals, lengthArg);
			result = monoArray;
#elif MS_CLR
			SAFEARRAY * safeArray = create_array_int(intVals, nVals);
			result = rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_I4);
#endif
		}
		// free(values); // TODO: do I need to do this? what is R doing with this?
		break;
	case CPLXSXP:
		lengthArg = LENGTH(el);
		cpl = COMPLEX(el);
		result = create_clr_complex_direct(cpl, lengthArg);
		break;
	case RAWSXP:
		lengthArg = LENGTH(el);
		rawVals = RAW(el);
		if (lengthArg == 1)
		{
#ifdef MONO_CLR
			result = create_mono_int32(intVals);
#elif MS_CLR
			result = new variant_t(rawVals[0]);
#endif
		}
		else if (lengthArg > 1 || lengthArg == 0)
		{
			int nVals = lengthArg;
#ifdef MONO_CLR
			MonoArray* monoArray = create_array_bytes(rawVals, lengthArg);
			result = monoArray;
#elif MS_CLR
			SAFEARRAY * safeArray = create_array_bytes(rawVals, nVals);
			result = rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_UI1);
#endif
		}
		break;
	case STRSXP:
		lengthArg = LENGTH(el);
		if( lengthArg == 1)
		{
			val = CHAR(STRING_ELT(el, 0));
			result = 
#ifdef MONO_CLR
				mono_string_new(domain, val);
#elif MS_CLR
				new variant_t(val); // WARNING: how is this memory reclaimed??
#endif
		}
		else if (lengthArg > 1 || lengthArg == 0)
		{
			char ** stringArray = (char**)malloc(sizeof(char*)*lengthArg);
#ifdef MONO_CLR
			MonoArray* monoArray = mono_array_new(domain, mono_get_string_class(), lengthArg);
			result = monoArray;
#endif
			for (j = 0; j < lengthArg; j++)
			{
				stringArray[j] = (char*)Rf_translateCharUTF8(STRING_ELT(el, j));
#ifdef MONO_CLR
				mono_array_set(monoArray, MonoString*, j, mono_string_new( domain, stringArray[j]));
#endif
			}
#ifdef MS_CLR
			SAFEARRAY * safeArray = create_array_strings(stringArray, LENGTH(el));
			result = rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_BSTR);
#endif
			free(stringArray);
		}
		break;
	case EXTPTRSXP:
		result = GET_CLR_OBJ_FROM_EXTPTR(el);
		break;
	case VECSXP:
		// If this is a list of S4 objects CLR objects, then either we create a safe array of variants, 
		// or we call a C# function that creates an array of objects.
		//SAFEARRAY * safeArray = create_array_double(stringArray, LENGTH(el));
		//result = rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_BSTR);
		//result = build_method_parameters(el);
		//// TODO if this is a data frame
		// result = rclr_wrap_data_frame(el);
		if(r_is_POSIXlt(el))
			// TODO: do_asPOSIXct in R core datetime.c would be handy. Cannot access - seems hidden.
			error("POSIXlt objects not yet supported - only UTC/GMT POSIXct can be reliably converted");
		else
			result = rclr_create_array_objects(el);
		break;
	default:
		result = NULL;
		warning("argument of R type %s (%d) converted to NULL\n", type2char(TYPEOF(el)), TYPEOF(el));
		break;
	}
	return (CLR_OBJ *) result;
}

CLR_OBJ * rclr_create_array_objects( SEXP s ) {
	int nElements = Rf_length(s), i;
#ifdef MS_CLR
	VARIANT ** result = new VARIANT*[nElements];
#elif MONO_CLR
	MonoArray * result_arr;
	void ** result = (void**)malloc(nElements*sizeof(void*)); // TOCHECK
#endif
	for (i = 0; i < nElements; i++) {
		result[i] = rclr_convert_element(VECTOR_ELT(s, i));
	}
#ifdef MS_CLR
	SAFEARRAY * safeArray = create_safe_array(result, nElements);
	return new CLR_OBJ(rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_VARIANT));
#elif MONO_CLR
	result_arr = create_array_object(result, nElements);
	free(result);
	return (CLR_OBJ*)result_arr;
#endif
}

CLR_OBJ * create_clr_complex_direct(Rcomplex * complex, int length)
{
	return NULL;
}

CLR_OBJ * get_clr_object( SEXP clrObj ) {
	SEXP a, clrobjSlotName;
	SEXP s4classname = getAttrib(clrObj, R_ClassSymbol);
	if (strcmp(CHAR(STRING_ELT(s4classname, 0)),"cobjRef" ) == 0)
	{
		PROTECT(clrobjSlotName = NEW_CHARACTER(1));
		SET_STRING_ELT( clrobjSlotName, 0,  mkChar( "clrobj" ));
		a = getAttrib(clrObj, clrobjSlotName);
		UNPROTECT(1);
		if (a != NULL && a != R_NilValue && TYPEOF(a)==EXTPTRSXP) {
			return GET_CLR_OBJ_FROM_EXTPTR(a);
		}
		else
			return NULL;
	}
	else
	{
		error("Incorrect type of S4 Object: Not of type 'cobjRef'");
		return NULL;
	}
}

SEXP clr_object_to_SEXP(CLR_OBJ * objptr) {
	SEXP result;
	//ClrObjectHandle clroh;
	ClrObjectHandle * clroh_ptr;
	if(objptr == NULL)
		return R_NilValue;
	clroh_ptr=(ClrObjectHandle *)malloc(sizeof(ClrObjectHandle));
	clroh_ptr->objptr = objptr;
	clroh_ptr->handle = 0;
#ifdef MONO_CLR
	clroh_ptr->handle = mono_gchandle_new(objptr, (mono_bool)1);
#elif MS_CLR
	// TODO
#endif
	result = R_MakeExternalPtr(clroh_ptr, R_NilValue, R_NilValue);
	R_RegisterCFinalizerEx(result, clr_object_finalizer, (Rboolean) 1/*TRUE*/);
	return result;
}

// Obsolete?
// const long long nineteenHundsTicks = 599266080000000000L;
// const long long ticksPerDay = 864000000000L;

#ifdef MONO_CLR

double * clr_datetimearray_obj_to_numeric(CLR_OBJ * datetimearray_ptr, MonoMethod * method) {
	MonoObject * exception;
	void ** params;
	params = (void**) &(datetimearray_ptr);
	return create_array_double_from_monoarray(mono_runtime_invoke(method, NULL, params, &exception));
}

double * clr_datetimearray_obj_to_r_date_numeric(CLR_OBJ * datetimearray_ptr) {
	return clr_datetimearray_obj_to_numeric( datetimearray_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDateDoubleRepresentations", 1));
}

double * clr_datetimearray_obj_to_r_posixct_numeric(CLR_OBJ * datetimearray_ptr) {
	return clr_datetimearray_obj_to_numeric( datetimearray_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDatePosixtcNumericValues", 1));
}

double * clr_datetime_obj_to_double(CLR_OBJ * datetime_ptr, MonoMethod * method) {
	MonoObject * exception;
	void * unboxed_obj;
	void ** params;
	unboxed_obj = mono_object_unbox(datetime_ptr);
	params = (void**) &(unboxed_obj);
	return (double*)mono_object_unbox(mono_runtime_invoke(method, NULL, params, &exception));
}

double * clr_datetime_obj_to_r_date_numeric(CLR_OBJ * datetime_ptr) {
	return clr_datetime_obj_to_double( datetime_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDateDoubleRepresentation", 1));
	//char * mnam = "get_Ticks";
	//CLR_OBJ * nTicksObj = rclr_mono_call_inst_method(mnam, pobj, NULL, 0);
	//long long * nTicks = (long long*)mono_object_unbox(nTicksObj);
	//long long offset = (*nTicks)-nineteenHundsTicks;
	//double result = (double)offset / ticksPerDay;
	//return &result;
}

double * clr_datetime_obj_to_r_posixtc_numeric(CLR_OBJ * datetime_ptr) {
	return clr_datetime_obj_to_double( datetime_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDatePosixtcNumericValue", 1));
	//char * mnam = "get_Ticks";
	//CLR_OBJ * nTicksObj = rclr_mono_call_inst_method(mnam, pobj, NULL, 0);
	//long long * nTicks = (long long*)mono_object_unbox(nTicksObj);
	//long long offset = (*nTicks)-nineteenHundsTicks;
	//double result = (double)offset / ticksPerDay;
	//return &result;
}

SEXP clr_obj_mono_convert_to_SEXP( CLR_OBJ * pobj) {
	SEXP result = NULL;
	char * klassName;
	/*MonoTypeEnum*/ int type_il;
	int n, j;
	char * str;
	double * values;
	MonoArray * monoarray;
	if( !pobj )
		return R_NilValue;
	else
	{
		klassName =  STR_DUP(mono_class_get_name(mono_object_get_class(pobj)));
		type_il = mono_type_get_type(mono_class_get_type(mono_object_get_class(pobj)));
		switch(type_il)  /*MonoTypeEnum*/
		{
		case MONO_TYPE_I          	: // IntPtr, that we assume to be a SEXP as coming from R.NET. This is a big assumption.
			// HACK? Assume this is an IntPtr, native handle to an R.NET object.
			result = (SEXP)(*((gpointer*)mono_object_unbox(pobj)));
			break;
		case MONO_TYPE_I4         	:
			result = make_int_sexp(1, (int*)mono_object_unbox (pobj));
			break;
		case MONO_TYPE_BOOLEAN    	:
			result = make_bool_sexp(1, (MonoBoolean*)mono_object_unbox (pobj));
			break;
		case MONO_TYPE_R8         	:
			result = make_numeric_sexp(1, (double*)mono_object_unbox(pobj));
			break;
		case MONO_TYPE_STRING     	:
			str = mono_string_to_utf8((MonoString*)pobj);
			result = make_char_sexp(1, &str);
			mono_free(str); // TOCHECK (or does the result still uses that memory??)
			break;
		case MONO_TYPE_CHAR       	:
			result = make_char_single_sexp((char*)mono_object_unbox (pobj));
			break;
		case MONO_TYPE_VALUETYPE    :
			{
				if(strcmp( klassName, "DateTime") == 0)
				{
					double * valueInREpoch = clr_datetime_obj_to_r_posixtc_numeric(pobj);
					result = make_POSIXct_sexp(1, valueInREpoch);
					break;
				}
			}
		case MONO_TYPE_SZARRAY      :  /* 0-based one-dim-array */
			monoarray = (MonoArray*) pobj;
			n = mono_array_length(monoarray);
			// FIXME: ask on mono-devel list where there are cleaner ways to check.
			if(mono_object_get_class(pobj) == mono_array_class_get(mono_get_double_class(), 1))
			{
				values = create_array_double_from_monoarray(monoarray);
				result = make_numeric_sexp(n, values);
				free(values);
				break;
			}
			else if(mono_object_get_class(pobj) == mono_array_class_get(mono_get_string_class(), 1))
			{
				char ** stringArray = (char**)malloc(sizeof(char*)*n);
				for( j = 0 ; j < n; j++)
					stringArray[j] = mono_string_to_utf8( mono_array_get(monoarray, MonoString*, j));
				result = make_char_sexp(n, stringArray);
				// TODO??? is make_char_sexp creating new strings??
				//for( j = 0 ; j < n; j++)
				//	mono_free(stringArray[j]); 
				free(stringArray);
				break;
			}
			else if(strcmp(klassName, "DateTime[]") == 0)
			{
				values = clr_datetimearray_obj_to_r_posixct_numeric(monoarray);
				result = make_POSIXct_sexp(n, values);
				free(values);
				break;
			}
		case MONO_TYPE_CLASS        :   /* arg: <type> token */
			result = clr_object_to_SEXP(pobj);
			break;
		case MONO_TYPE_END          :   /* End of List */
		case MONO_TYPE_VOID       	:
		case MONO_TYPE_I1         	:
		case MONO_TYPE_U1         	:
		case MONO_TYPE_I2         	:
		case MONO_TYPE_U2         	:
		case MONO_TYPE_U4         	:
		case MONO_TYPE_I8         	:
		case MONO_TYPE_U8         	:
		case MONO_TYPE_R4         	:
		case MONO_TYPE_PTR          :   /* arg: <type> token */
		case MONO_TYPE_BYREF        :   /* arg: <type> token */
		case MONO_TYPE_VAR	        :   /* number */
		case MONO_TYPE_ARRAY        :   /* type, rank, boundsCount, bound1, loCount, lo1 */
		case MONO_TYPE_GENERICINST  :   /* <type> <type-arg-count> <type-1> \x{2026} <type-n> */
		case MONO_TYPE_TYPEDBYREF 	:
		case MONO_TYPE_U            :
			//warning("As yet, unhandled type of underlying MonoTypeEnum hex code %x", type_il);
		default:
			result = clr_object_to_SEXP(pobj);
			break;
		}
	}
	return result;
}

SEXP rclr_mono_reflect_object(CLR_OBJ * obj) {
	SEXP result;
	int nmeth = 0, nfields = 0;
	int i;
	int counter = 0;

	void * iter;
	char ** mnames = NULL; //= getNames(clrobj);
	MonoClass *klass;
	MonoClassField * field;
	MonoMethod *method = NULL, *m = NULL;
	klass = mono_object_get_class (obj);

	iter = NULL;
	while ((m = mono_class_get_methods (klass, &iter))) {
		nmeth ++;
	}
	iter = NULL;
	while ((field = mono_class_get_fields (klass, &iter))) {
		nfields ++;
	}

	mnames = (char**)malloc((nmeth+nfields)*sizeof(char*));

	iter = NULL; counter = 0;
	while ((m = mono_class_get_methods (klass, &iter))) {
		mnames[counter] = STR_DUP(mono_method_get_name(m));
		counter ++;
	}

	iter = NULL;
	while ((field = mono_class_get_fields (klass, &iter))) {
		mnames[counter] = STR_DUP(mono_field_get_name(field));
		counter ++;
	}

	PROTECT(result = NEW_CHARACTER(nmeth + nfields));
	for(i = 0; i < nmeth+nfields ; i++ ) {
		SET_STRING_ELT( result, i, mkChar( mnames[i] ));
	}
	UNPROTECT(1);
	free(mnames);
	return result;
}

SEXP rclr_mono_get_type_name(SEXP clrObj) {
	CLR_OBJ * pbjptr = get_clr_object(clrObj);
	return make_char_single_sexp( get_type_full_name(pbjptr) );
}


/* This seems buggy.
void get_ns_and_type_from_fqtn(char * ns_qualified_typename, char ** name_space, char ** type_short_name)
{
char * tmp = STR_DUP(ns_qualified_typename);
char * long_type_name = strtok( tmp, ","); // My.Namespace.ShortTypeName,MyAssembly ==> My.Namespace.ShortTypeName
// MyNamespace.ShortTypeName
char * cand_short_type_name = (strrchr( long_type_name, (int)('.')) + 1); // get a pointer to 'ShortTypeName'
size_t count = (cand_short_type_name-1) - long_type_name; // 'a.b.c.d' : 6 - 1 - 0 = 5 characters, i.e. 'a.b.c'
char * n_s = (char*) malloc((count+1)*sizeof(char));
strncpy(n_s, long_type_name, count);
n_s[count] = '\0';
*name_space = n_s;
*type_short_name = cand_short_type_name;
free(tmp);
}
*/

SEXP rclr_mono_call_static_method(char * ns_qualified_typename, const char *mnam, SEXP methodParams) {
	CLR_OBJ * obj = NULL;
	CLR_OBJ * result = NULL;
	// char * name_space;
	//	char * type_short_name;
	//get_ns_and_type_from_fqtn(ns_qualified_typename, &name_space, &type_short_name);
	result = rclr_mono_call_static_method_tname(ns_qualified_typename, (char*)mnam, build_method_parameters(methodParams), Rf_length(methodParams));
	return clr_obj_mono_convert_to_SEXP(result);
}

SEXP rclr_mono_diagnose_method_parameters(SEXP methodParams) {
	CLR_OBJ * obj = NULL;
	MonoMethod * methodCallStaticMethod = rclr_mono_get_method(spTypeClrFacade, "DiagnoseMethodCall", 1);
	MonoObject * exception, *result;
	void** static_mparams = (void**)malloc(1 * sizeof(void*));
	MonoArray* methParams = create_array_object(build_method_parameters(methodParams), Rf_length(methodParams));
	static_mparams[0] = methParams;
	result = mono_runtime_invoke(methodCallStaticMethod, NULL, static_mparams, &exception);
	print_if_exception(exception);
	free(static_mparams);
	return clr_obj_mono_convert_to_SEXP(result);
}

CLR_OBJ * rclr_mono_call_static_method_tname(char * ns_qualified_typename, char * mnam, void ** params, int paramCount) {
	MonoMethod * methodCallStaticMethod = rclr_mono_get_method(spTypeClrFacade, "CallStaticMethodMono", 3);
	MonoObject * exception, *result;
	void** static_mparams = (void**)malloc(3 * sizeof(void*)); // TOCHECK
	MonoArray* methParams = create_array_object(params, paramCount);
	static_mparams[0] = create_mono_string(ns_qualified_typename);
	static_mparams[1] = create_mono_string(mnam);
	static_mparams[2] = methParams;
	result = mono_runtime_invoke(methodCallStaticMethod, NULL, static_mparams, &exception);
	print_if_exception(exception);
	free(static_mparams);
	return result;
}

CLR_OBJ * rclr_mono_convert_element_rdotnet(SEXP el)
{
	// The idea here is that we create a SymbolicExpression in C#, that the C# code will intercept
	CLR_OBJ * obj = NULL;
	MonoMethod * method = 
		rclr_mono_get_method(spTypeClrFacade, "CreateSexpWrapper", 1);
		//rclr_mono_get_method(spTypeClrFacade, "CreateSexpWrapperMs", 1);
	MonoObject * exception, *result;
	void** static_mparams = (void**)malloc(1 * sizeof(void*));
	// Note to self: it took me some trial and error to figure out what to pass in order to have things working. Not sure why &el directly, but this works...
	// MonoArray* methParams = create_array_object(el, 1);
	// static_mparams[0] = create_mono_intptr(el);
	// static_mparams[0] = create_mono_intptr(&el);
	// static_mparams[0] = create_mono_int64(&el);
	// static_mparams[0] = create_mono_int64(el);
	static_mparams[0] = &el;
	result = mono_runtime_invoke(method, NULL, static_mparams, &exception);
	print_if_exception(exception);
	free(static_mparams);
	return result;
}

// This calls the mono method directly on the static class, rather than going through the ClrFacade.
// To keep consistency with the MS.NET and allow for the interception of the result for custom data marshalling, usage is not recommended.
CLR_OBJ * rclr_mono_call_static_method_tname_directcall(char * ns_qualified_typename, char * mnam, void ** params, int paramCount) {
	MonoMethod * methodGetType = rclr_mono_get_method( spTypeClrFacade, "GetType", 1);
	CLR_OBJ * type = rclr_mono_invoke_method_stringarg(methodGetType, ns_qualified_typename);
	MonoReflectionType* theType;
	MonoClass * klass;
	MonoMethod * method;
	MonoObject * exception, *result;
	if(!type)
	{
		error("type not found: %s", ns_qualified_typename);
		return NULL;
	}
	theType = (MonoReflectionType*) type;
	klass = mono_class_from_mono_type( theType->type );
	if(!klass)
	{
		return NULL;
	}
	method = rclr_mono_get_method(klass, mnam, paramCount);
	if(!method)
	{
		return NULL;
	}
	// TODO: not sure how to check that the method is static or not
	//if(mono_signature_is_instance(mono_method_desc_from_method(method)))
	//{
	//	Rf_error("Trying to call an instance method as a static method");
	//	return NULL;
	//}
	result = mono_runtime_invoke (method, NULL, params, &exception);
	print_if_exception(exception);
	return result;
}

CLR_OBJ * rclr_mono_call_inst_method(const char *mnam, CLR_OBJ * obj, void ** params, int param_count ) {
	MonoClass *klass;
	MonoMethod * method;
	CLR_OBJ * result = NULL;
	CLR_OBJ * exception = NULL;
	klass = mono_object_get_class(obj);
	method = mono_class_get_method_from_name(klass, mnam, param_count);
	result = mono_runtime_invoke (method, obj, params, &exception);
	print_if_exception(exception);
	return result;
}


#endif // MONO_CLR



/////////////////////////////////////////
// Functions without R specific constructs
/////////////////////////////////////////

#ifdef MONO_CLR

void rclr_mono_create_domain( char* filename, int mono_debug) {
	if(domain != NULL)
	{
		warning("rclr_mono_create_domain called with argument %s, but the domain is already set. Ignoring.", filename);
		return;
	}
	if(mono_debug) {
		mono_jit_parse_options(sizeof(options)/sizeof(char*), (char**)options);
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);
	}
	if(INIT_CLR_FROM_FILE)
		domain = mono_jit_init(filename);
	else {
		domain = mono_jit_init_version("rClr_domain", RCLR_DEFAULT_RUNTIME_VERSION);
		if (!domain) {
			error("Failed to create rClr MonoDomain. Requested runtime was %s", RCLR_DEFAULT_RUNTIME_VERSION);
			return;
		}
		else {
			//TODO: have a message that prints the version of the runtime. One seems not to always get the requested one with Mono, somehow. 
			// warning()
			//mono_jit_info_get_code_start
			//mono_jit_info_get_code_start
		}
	}

	assembly = mono_domain_assembly_open (domain, filename);
	if (!assembly) {
		error("Mono assembly seems to be NULL??");
		return;
	}
	image = mono_assembly_get_image (assembly);
	if (!image) {
		error("Mono image seems to be NULL??");
		return;
	}
	/*
	* mono_jit_exec() will run the Main() method in the assembly.
	* The return value needs to be looked up from
	* System.Environment.ExitCode.
	*/
	mono_jit_exec (domain, assembly, 1, &filename);

	spTypeClrFacade = mono_class_from_name( image, "Rclr", "ClrFacade");
}

void rclr_mono_load_assembly( char ** filename )
{
	MonoMethod * method = rclr_mono_get_method( spTypeClrFacade, "LoadFrom", 1);
	MonoString * fname = create_mono_string(*filename);
	CLR_OBJ * result;
	MonoObject * exception = NULL;
	result = mono_runtime_invoke (method, NULL, (void**)(&fname), &exception);
	print_if_exception(exception);
}


const char * get_type_full_name(CLR_OBJ * objptr) {
	MonoClass * klass;
	char result[1024]; // Well, I know...
	klass = mono_object_get_class(objptr);
	if ( !klass ) return "";

	result[0] = 0;
	strcat(result,mono_class_get_namespace(klass));
	strcat(result,".");
	strcat(result,mono_class_get_name(klass));
	return STR_DUP(result);
}

#elif MS_CLR

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

HRESULT rclr_ms_call_static_method_facade(char * methodName, CLR_OBJ * objptr, VARIANT * result) {
	//CLR_OBJ * rclr_ms_call_static_method_facade(char * methodName, CLR_OBJ * objptr) {
	bstr_t bstrMethodName(methodName);
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	LONG index = 0;
	variant_t param(objptr);
	HRESULT hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &param);
	if(SUCCEEDED(hr))
	{
		//CLR_OBJ tmpResult;
		hr = rclr_ms_call_static_method(spTypeClrFacade, &bstrMethodName, psaStaticMethodArgs, result);
	}
	SafeArrayDestroy(psaStaticMethodArgs);
	return hr;
}

const char * get_type_full_name(CLR_OBJ * objptr) {
	CLR_OBJ tmpResult; 
	HRESULT hr = rclr_ms_call_static_method_facade("GetObjectTypeName", objptr, &tmpResult);
	bstr_t blah(tmpResult.bstrVal);
	return bstr_to_c_string(&blah);

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

// Create a safe array to contain the arguments of the method.
HRESULT rclr_ms_create_object(char * longtypename, VARIANT ** params, int argLength, VARIANT * vtResult) {
	bstr_t bstrStaticMethodName(L"CreateInstance");
	variant_t tname(longtypename);
	SAFEARRAY * psaCtorArgs = rclr_ms_create_constructor_parameters(&tname, params, argLength );
	HRESULT hr = rclr_ms_call_static_method(spTypeClrFacade, &bstrStaticMethodName, psaCtorArgs, vtResult);
	SafeArrayDestroy(psaCtorArgs);
	//clear_BSTR(&tname);
	return hr;
}

HRESULT rclr_ms_load_assembly(char ** filename, VARIANT * vtResult) {
	bstr_t bstrStaticMethodName(L"LoadFrom");
	return rclr_ms_call_static_method_stringarg(spTypeClrFacade, &bstrStaticMethodName, *filename, vtResult);
}

HRESULT rclr_ms_call_static_method_tname(char * ns_qualified_typename, char * mnam, VARIANT ** params, int argLength, VARIANT * result) {
	HRESULT hr;
	SAFEARRAY * psaStaticMethodArgs = rclr_ms_create_static_method_call_parameters(ns_qualified_typename, mnam, params, argLength );
	bstr_t bstrCallStaticMethod("CallStaticMethod");
	// TODO / BUG: if there is only one array passed as an argument, then CallStaticMethod will "somehow" 
	// flatten to e.g. 4 arguments, instead of one objet argument that is an array/
	hr = rclr_ms_call_static_method(spTypeClrFacade, &bstrCallStaticMethod, psaStaticMethodArgs, result);
	SafeArrayDestroy(psaStaticMethodArgs);
	return hr;
}

HRESULT rclr_ms_call_method(CLR_OBJ * objptr, char * methodName, VARIANT ** params, int argLength, VARIANT * vtResult) {
	SAFEARRAY * psaStaticMethodArgs = rclr_ms_create_method_call_parameters(objptr, methodName, params, argLength );
	bstr_t bstrCallInstanceMethod("CallInstanceMethod");
	HRESULT hr = rclr_ms_call_static_method(spTypeClrFacade, &bstrCallInstanceMethod, psaStaticMethodArgs, vtResult);
	SafeArrayDestroy(psaStaticMethodArgs);
	return hr;
}

HRESULT rclr_ms_call_static_method(_TypePtr spType, bstr_t * bstrStaticMethodNamePtr, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult) {
	HRESULT hr = S_FALSE;
	bstr_t getLastException("get_LastCallException");
	//_TCHAR* blah;
	//try {
	hr = spType->InvokeMember_3(*bstrStaticMethodNamePtr, static_cast<BindingFlags>(
		BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public), 
		NULL, vtEmpty, psaStaticMethodArgs, vtResult);
	//} 
	//catch(_com_error& e) {
	//	blah = TEXT("ERROR: %s\n"),(_TCHAR*)e.Description();
	//} 
	if (FAILED(hr))	{
        // Work around https://r2clr.codeplex.com/workitem/67
		hr = spType->InvokeMember_3(getLastException, static_cast<BindingFlags>(
			BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public), 
			NULL, vtEmpty, NULL, vtResult);
		bstr_t tmpBstr(vtResult->bstrVal);
		if (std::string((char*)vtResult->bstrVal) == std::string(""))
			error("%s", "Failure in rclr_ms_call_static_method, but could not retrieve an error message");
		else
		// There seems to be a limit to the size printed by error, which is a problem for long stack traces
		// Nevertheless cannot use Rprintf here or this breaks the try/error patterns e.g. with the testthat package...
		// Rprintf("%s", bstr_to_c_string( &tmpBstr ));
			error("%s", bstr_to_c_string( &tmpBstr )); 
	}
	return hr;
}

HRESULT rclr_ms_call_static_method_stringarg(_TypePtr spType, bstr_t * bstrStaticMethodNamePtr, char * strArg, VARIANT * vtResult) {
	SAFEARRAY * psaStaticMethodArgs = create_array_one_string(strArg);
	return rclr_ms_call_static_method(spType, bstrStaticMethodNamePtr, psaStaticMethodArgs, vtResult);
}


HRESULT rclr_ms_get_current_object_direct(VARIANT * vtResult) {
	HRESULT hr = S_FALSE;
	bstr_t getCurrentObject("get_CurrentObject");
	hr = spTypeClrFacade->InvokeMember_3(getCurrentObject, static_cast<BindingFlags>(
		BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
		NULL, vtEmpty, NULL, vtResult);
	if (FAILED(hr))	{
		error("%s", "Failure in rclr_ms_get_current_object_direct");
	}
	return hr;
}

// Obsolete??
HRESULT rclr_ms_call_method_interface(CLR_OBJ * obj, const char *interfaceName, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult) {
	HRESULT hr;
	_ObjectPtr pObject; 
	rclr_ms_get_raw_object(obj, &pObject);
	_TypePtr pType;
	hr = pObject->GetType(&pType);
	if (FAILED(hr))	{
		error("Retrieving the CLR type failed with message: %s\n", getComErrorMsg(hr));
		return hr;
	}
	bstr_t ifName(interfaceName);
	//BSTR ifName(interfaceName);
	_TypePtr pIfType;
	hr = pType->GetInterface(ifName, VARIANT_BOOL(0), &pIfType);
	if (FAILED(hr))	{
		error("Retrieving the interface type for %s failed with message: %s\n", interfaceName, getComErrorMsg(hr));
		return hr;
	}
	return rclr_ms_call_method_type(obj, pIfType, mnam, psaStaticMethodArgs, vtResult);
}

HRESULT rclr_ms_call_method_type(CLR_OBJ * obj, _TypePtr pType, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult)
{
	bstr_t bstrMethodName(mnam);
	//SAFEARRAY * members = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	HRESULT hr;
	//hr = pType->GetMember_3(bstrMethodName, &members);
	//_MethodBasePtr mPtr;
	//SafeArrayGetElement( members, 0, mPtr);
	//hr = mPtr->Invoke_3(*obj, psaStaticMethodArgs, vtResult);
	hr = pType->InvokeMember_3(bstrMethodName, static_cast<BindingFlags>(
		// The specification of binding flags as per the following line fails to find some interface methods (explicit interface implementations in C#, probably
		//        BindingFlags_InvokeMethod | BindingFlags_Instance | BindingFlags_Public), 
		BindingFlags_InvokeMethod), 
		NULL, *obj, psaStaticMethodArgs, vtResult);
	if (FAILED(hr))	{
		error("Instance method call failed with message: %s\n", getComErrorMsg(hr));
	}
	return hr;
}

// BUG: stubborn problems with this one. Give up... DEPRECATED
HRESULT rclr_ms_gettype(CLR_OBJ * obj, _Type ** pType) {

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
	HRESULT hr = rclr_ms_get_raw_object(obj, &pObject);
	if(!SUCCEEDED(hr))
		return hr;
	else
	{
		try
		{
			return pObject->GetType(pType); // TODO: this fails with the MS CLR. Unclear why. Big blocker.
		}
		catch(...)
		{
			pType = NULL;
			return -1;
		}
	}
}

HRESULT rclr_ms_get_raw_object(CLR_OBJ * obj, _Object ** ppObject) {
	HRESULT hr;
	switch(obj->vt)
	{
	case VT_DISPATCH:
		hr = obj->pdispVal->QueryInterface(__uuidof(_Object), (void**)ppObject);
		break;
	case VT_UNKNOWN:
		hr = obj->punkVal->QueryInterface(__uuidof(_Object), (void**)ppObject);
		break;
	default:
		hr = S_FALSE;
	}
	return hr;
}

// likely obsolete
HRESULT rclr_ms_call_method_sa(CLR_OBJ * obj, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult)
{
	_TypePtr pType;
	HRESULT hr = rclr_ms_gettype(obj, &pType);
	if(FAILED(hr) || !pType) {
		return hr;
	}
	return rclr_ms_call_method_type(obj, pType, mnam, psaStaticMethodArgs, vtResult);
}

// likely obsolete
HRESULT rclr_ms_call_method_stringarg(CLR_OBJ * obj, const char *mnam, char * strArg, VARIANT * vtResult) {
	SAFEARRAY * psaStaticMethodArgs = create_array_one_string(strArg);	
	HRESULT hr = rclr_ms_call_method_sa(obj, mnam, psaStaticMethodArgs, vtResult);
	SafeArrayDestroy(psaStaticMethodArgs);
	return hr;
}

// likely obsolete
HRESULT rclr_ms_call_method_argless(CLR_OBJ * obj, const char *mnam, VARIANT * vtResult) {
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 0);
	HRESULT hr = rclr_ms_call_method_sa(obj, mnam, psaStaticMethodArgs, vtResult);
	SafeArrayDestroy(psaStaticMethodArgs);
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

void rclr_ms_create_domain(char ** appBaseDir) {
	HRESULT hr;
#ifdef USE_COR_RUNTIME_HOST
	hr = pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, 
		IID_PPV_ARGS(&pCorRuntimeHost));
	if (FAILED(hr))	{
		error("Getting the CorRuntimeHost failed with message: %s\n", getComErrorMsg(hr));
		return;
	}
	// Get a pointer to the default AppDomain in the CLR.
	//hr = pCorRuntimeHost->GetDefaultDomain(&spAppDomainThunk);
	//hr = spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&spDefaultAppDomain));

#else
	hr = pRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost,IID_PPV_ARGS(&pRuntimeHost));
	ICLRControl* pCLRControl = nullptr;
	hr = pRuntimeHost->GetCLRControl(&pCLRControl);

	LPCWSTR appDomainManagerTypename = L"SampleAppDomainManager.CustomAppDomainManager";
	LPCWSTR assemblyName = L"SampleAppDomainManager";
	hr = pCLRControl->SetAppDomainManagerType(assemblyName, appDomainManagerTypename);
	pMetaHost->

		pClrRuntimeHost->GetCLRControl(&pClrControl);
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
	hr = pCorRuntimeHost->CreateDomainSetup( &appDomainSetupUnknown);
	if (FAILED(hr))	{
		error("CreateDomainSetup failed with message: %s\n", getComErrorMsg(hr));
		return;
	}

	// Fill some basic structure info about this domain..
	//plugin->m_AppDomainSetup = plugin->m_AppDomainSetupUnknown;
	IAppDomainSetup * appDomSetup = NULL;
	hr = appDomainSetupUnknown->QueryInterface(__uuidof(IAppDomainSetup), (void**)&appDomSetup);
	if (FAILED(hr))	{
		error("Getting IAppDomainSetup failed with message: %s\n", getComErrorMsg(hr));
		return;
	}
	if (appDomSetup == NULL){
		error("Getting IAppDomainSetup returned NULL\n");
		return;
	}

	//plugin->m_AppDomainSetup->put_ApplicationBase( CComBSTR( "F:\\bin\\R\\library\\rClr\\libs\\x64\\" ) );
	//plugin->m_AppDomainSetup->put_ShadowCopyFiles( CComBSTR( "false" ) );
	//plugin->m_AppDomainSetup->put_ApplicationName( CComBSTR( "rClr" ) );
	bstr_t appBaseDirPath(appBaseDir[0]);
	bstr_t shadowCopyFiles("false");
	bstr_t appName("rClr");

	appDomSetup->put_ApplicationBase( appBaseDirPath );
	appDomSetup->put_ShadowCopyFiles( shadowCopyFiles );
	appDomSetup->put_ApplicationName( appName );

	// Create the new AppDomain..
	// pCorRuntimeHost->CreateDomainEx( L"CLR Facade", plugin->m_AppDomainSetupUnknown, NULL, &plugin->m_AppDomainUnknown );
	IUnknown * appDomainUnknown;
	hr = pCorRuntimeHost->CreateDomainEx( L"CLR Facade", appDomainSetupUnknown, NULL, &appDomainUnknown);
	if (FAILED(hr))	{
		error("CreateDomainEx failed with message: %s\n", getComErrorMsg(hr));
		return;
	}

	// Obtain the actual AppDomain object..
	//plugin->m_AppDomainUnknown->QueryInterface( __uuidof( mscorlib::_AppDomain ), (void**)&plugin->m_AppDomain );
	_AppDomain * appDomain;
	hr = appDomainUnknown->QueryInterface( __uuidof( mscorlib::_AppDomain ), (void**)&appDomain );
	if (FAILED(hr))	{
		error("Failed to retrieve _AppDomain: %s\n", getComErrorMsg(hr));
		return;
	}

	//plugin->m_AppDomain->Load_2( pwzDllName, &plugin->m_AppAssembly );
	//&m_AppDomain->Load_2( pwzDllName, &plugin->m_AppAssembly );
	//spAssembly = &plugin->m_AppAssembly;
	//// Load the .NET assembly.
	spDefaultAppDomain = appDomain;

	bstr_t bstrAssemblyName(pwzDllName);
	//bstr_t bstrAssemblyName(L"F:\\bin\\R\\library\\rClr\\libs\\x64\\ClrFacade.dll");
	//hr = spDefaultAppDomain->AppendPrivatePath(L"F:\\bin\\R\\library\\rClr\\libs\\x64");
	//BSTR baseDir;
	//hr = spDefaultAppDomain->get_BaseDirectory(&baseDir);
	// Load the plugin module into the AppDomain..
	hr = spDefaultAppDomain->Load_2(bstrAssemblyName, &spAssembly);
	if (FAILED(hr))	{
		error("Failed to load the assembly '%s' from disk: %s. App base dir is %s\n", bstr_to_c_string(&bstrAssemblyName), getComErrorMsg(hr), appBaseDir[0]);
		return;
	}
	hr = spAssembly->GetType_2(bstrClassName, &spTypeClrFacade);
	if (FAILED(hr))	{
		error("Getting the type of the 'CLR facade' failed with message: %s\n", getComErrorMsg(hr));
		return;
	}
}

HRESULT rclr_ms_get_facade_typeref(_Type ** spType) {
	*spType = _TypePtr(spTypeClrFacade);
	return S_OK;
}

char * bstr_to_c_string(bstr_t * src) {
#ifndef  UNICODE                     // r_winnt
	return src;
#else
	// Convert the wchar_t string to a char* string.
	// see http://msdn.microsoft.com/en-us/library/ms235631.aspx
	size_t origsize = wcslen(*src) + 1;
	size_t convertedChars = 0;
	const size_t newsize = origsize*2;
	char *nstring = new char[newsize];
	wcstombs_s(&convertedChars, nstring, newsize, *src, _TRUNCATE);
	return nstring;
#endif

}

char * getComErrorMsg(HRESULT hr) {
	bstr_t errMsg(_com_error(hr).ErrorMessage()); // FIXME: surely there is a way to retrieve the exception message, no?
	return bstr_to_c_string(&errMsg);
}

VARIANT * rclr_ms_create_safe_array(VARIANT ** params, int paramsArgLength) {
	SAFEARRAY * psaMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, paramsArgLength); 
	for (LONG i = 0; i < paramsArgLength; i++)
	{
		if(params[i] != NULL)
			SafeArrayPutElement(psaMethodArgs, &i, params[i]); // TOCHECK: defaults in the array are VT_EMPTY?
	}
	return rclr_ms_create_vt_array(psaMethodArgs, VT_ARRAY | VT_VARIANT);
}

// This is a workaround for https://r2clr.codeplex.com/workitem/70. 
// SafeArrayPutElement on a VT_ARRAY | VT_R8 for instance will trigger a deep copy; 
// safe but onerous with CPU and memory.
HRESULT add_params_as_new_safearray(SAFEARRAY * methodCallArg, LONG* rgIndices, VARIANT ** params, int paramsArgLength) {
	HRESULT hr;
	VARIANT * psafe_array;
	psafe_array = rclr_ms_create_safe_array(params, paramsArgLength);
	hr = SafeArrayPutElement(methodCallArg, rgIndices, psafe_array);
	VariantClear(psafe_array);
	return hr;
}

SAFEARRAY * rclr_ms_create_constructor_parameters(VARIANT * class_name, VARIANT ** params, int paramsArgLength )
{
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 2); 
	LONG index = 0;
	HRESULT hr = SafeArrayPutElement(psaStaticMethodArgs, &index, class_name);
	index = 1;
	hr = add_params_as_new_safearray(psaStaticMethodArgs, &index, params, paramsArgLength);
	return psaStaticMethodArgs;
}

SAFEARRAY * rclr_ms_create_common_call_parameters(VARIANT * obj_or_class_name, char * mnam, VARIANT ** params, int paramsArgLength )
{
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 3); 
	LONG index = 0;
	HRESULT hr = SafeArrayPutElement(psaStaticMethodArgs, &index, obj_or_class_name);
	index = 1;
	//create_BSTR(&vMname, mnam);
	variant_t vName(mnam);
	hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vName);
	index = 2;
	hr = add_params_as_new_safearray(psaStaticMethodArgs, &index, params, paramsArgLength);
	//clear_BSTR(&vMname);
	return psaStaticMethodArgs;
}

SAFEARRAY * rclr_ms_create_method_call_parameters(CLR_OBJ * objptr, char * mnam, VARIANT ** params, int paramsArgLength )
{
	return rclr_ms_create_common_call_parameters(objptr, mnam, params, paramsArgLength );
}

SAFEARRAY * rclr_ms_create_static_method_call_parameters(char * assemblyQualifiedTypeName, char * mnam, VARIANT ** params, int paramsArgLength )
{
	//VARIANT tname;
	SAFEARRAY * result;
	//create_BSTR(&tname, assemblyQualifiedTypeName);
	variant_t tname(assemblyQualifiedTypeName);
	result = rclr_ms_create_common_call_parameters(&tname, mnam, params, paramsArgLength );
	//clear_BSTR(&tname);
	return result;
}

VARIANT * rclr_ms_create_vt_array(SAFEARRAY * safeArray, VARTYPE vartype) {
	VARIANT * pvt = new variant_t(); // TODO: I suspect this leads to a memory leak if it ends up in an array itself. https://r2clr.codeplex.com/workitem/70
	VariantInit(pvt);
	pvt->vt = vartype;
	pvt->parray = safeArray;
	//_variant_t variant;
	// variant.Attach(vt);
	return pvt;
}

SAFEARRAY * create_array_one_string(char * strArg) {
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	LONG index = 0;
	bstr_t bstringArg(strArg);
	variant_t vtStringArg(bstringArg);
	HRESULT hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vtStringArg);
	return psaStaticMethodArgs;
}

SAFEARRAY * create_array_strings(char ** values, int length) {
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_BSTR, 0, length); 
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

		bstr_t * bstringArg = new bstr_t((const char *)values[i]);
		SafeArrayPutElement(psaStaticMethodArgs, &i, bstringArg->GetBSTR());
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

SAFEARRAY * create_safe_array(VARIANT ** values, int length) {
	create_safearray_oftype(VT_VARIANT, values, length);
	//SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, length); 
	//for (LONG i = 0; i < length; i++)
	//{
	//	SafeArrayPutElement(psaStaticMethodArgs, &i, values[i]);
	//}
	//return psaStaticMethodArgs;
}

SAFEARRAY * create_array_double( double * values, int length )
{
	create_safearray_oftype(VT_R8, values, length);
	//SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(VT_R8, 0, length); 
	//for (LONG i = 0; i < length; i++)
	//{
	//	variant_t vtArg = new variant_t(values[i]);
	//	SafeArrayPutElement(psaStaticMethodArgs, &i, &values[i]);
	//}
	//return psaStaticMethodArgs;
}

SAFEARRAY * create_array_dates(double * values, int length)
{
	create_safearray_oftype(VT_DATE, values, length);
}

SAFEARRAY * create_array_bytes(unsigned char * values, int length)
{
	create_safearray_oftype(VT_UI1, values, length);
}

SAFEARRAY * create_array_int_or_bool(int * values, int length, VARTYPE vtype)
{
	SAFEARRAY * psaStaticMethodArgs = SafeArrayCreateVector(vtype, 0, length); 
	for (LONG i = 0; i < length; i++)
	{
		//variant_t vtArg = vtype == VT_BOOL ? new variant_t((bool)values[i]) : new variant_t(values[i]) ;
		SafeArrayPutElement(psaStaticMethodArgs, &i, &values[i]);
	}
	return psaStaticMethodArgs;
}

SAFEARRAY * create_array_int( int * values, int length )
{
	return create_array_int_or_bool( values, length, VT_I4);
}

SAFEARRAY * create_array_bool( int * values, int length )
{
	return create_array_int_or_bool( values, length, VT_BOOL);
}


#endif
