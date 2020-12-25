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

void rclr_create_domain(char** appBaseDir, char** filename, int* mono_debug) {

#ifdef MONO_CLR 
	rclr_mono_create_domain(filename[0], *mono_debug);
#elif MS_CLR
	start_ms_clr();
	rclr_ms_create_domain(appBaseDir);
#endif
}

void rclr_load_assembly(char** filename) {
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

void clr_object_finalizer(SEXP clrSexp) {
	ClrObjectHandle* clroh_ptr;
	if (TYPEOF(clrSexp)==EXTPTRSXP) {
#ifdef MONO_CLR
		clroh_ptr = (ClrObjectHandle*)R_ExternalPtrAddr(clrSexp);
		mono_gchandle_free(clroh_ptr->handle);
		free(clroh_ptr);
#elif MS_CLR
		CLR_OBJ* objptr;
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


/////////////////////////////////////////
// Functions with R specific constructs, namely SEXPs
/////////////////////////////////////////


#ifdef MONO_CLR
SEXP make_bool_sexp(int n, MonoBoolean* values) {
#elif MS_CLR
SEXP make_bool_sexp(int n, bool* values) {
#endif
	SEXP result ;
	long i = 0;
	int* intPtr;
	PROTECT(result = NEW_LOGICAL(n));
	intPtr = LOGICAL_POINTER(result);
	for(i = 0; i < n ; i++ ) {
		intPtr[i] = 
			// found many tests in the mono codebase such as below. Trying by mimicry then.
			// 		if (*(MonoBoolean* ) mono_object_unbox(res)) {
			values[i]; // See http://r2clr.codeplex.com/workitem/34;
	}
	UNPROTECT(1);
	return result;
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
	CLR_OBJ* objptr = NULL;
	objptr = rclr_mono_get_current_object_direct();
	result = clr_obj_mono_convert_to_SEXP(objptr);
#endif
	return result;
}

SEXP r_create_clr_object( SEXP p ) {
	SEXP methodParams, result;
	CLR_OBJ* objptr = NULL;
	char* ns_qualified_typename = NULL;
	R_len_t argLength;
	p=CDR(p); /* skip the first parameter: function name*/
	get_FullTypeName(p, &ns_qualified_typename); p=CDR(p);
	methodParams = p;
	argLength = Rf_length(methodParams);
#ifdef MS_CLR
	HRESULT hr;
	CLR_OBJ obj; // needs this: otherswise null pointer if using only objptr. 
	VARIANT** params = build_method_parameters(methodParams);
	hr = rclr_ms_create_object(ns_qualified_typename, params, argLength, &obj);
	free_variant_array(params, argLength);
	release_transient_objects();
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

SEXP r_get_type_name(SEXP clrObj) {
#ifdef MONO_CLR
	return rclr_mono_get_type_name(clrObj);
#elif MS_CLR
	return rclr_ms_get_type_name(clrObj);
#endif
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
	const char* mnam;
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
	VARIANT** params = build_method_parameters(methodParams);
	CLR_OBJ result;
	hr = rclr_ms_call_static_method_tname(ns_qualified_typename, (char* )mnam, params, argLength, &result);
	free_variant_array(params, argLength);
	release_transient_objects();
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
	const char* mnam = 0;
	CLR_OBJ* objptr=NULL;

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
	VARIANT** params = build_method_parameters(methodParams);
	CLR_OBJ result;
	rclr_ms_call_method(objptr, (char* )mnam, params, argLength, &result);
	free_variant_array(params, argLength);
	release_transient_objects();
	return clr_obj_ms_convert_to_SEXP(result);
#endif

}

// called by clrTypeNameExtPtr
SEXP r_get_typename_externalptr(SEXP p) {
	SEXP e;
	CLR_OBJ* objptr = NULL;
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

#ifdef MONO_CLR
void** build_method_parameters(SEXP largs) {
#elif MS_CLR
VARIANT** build_method_parameters(SEXP largs) {
#endif
	int i;
	int nargs = Rf_length(largs);
	int lengthArg = 0;
	SEXP args = largs;
	SEXP el;
	const char* name;

	if (nargs==0) {
		return NULL;
	}

#ifdef MONO_CLR
	void** mparams;
	mparams = (void**)malloc(nargs*sizeof(void*)); // TOCHECK
#elif MS_CLR
	VARIANT** mparams = new VARIANT*[nargs];
#endif

	for(i = 0; args != R_NilValue; i++, args = CDR(args)) {
		name = isNull(TAG(args)) ? "" : CHAR(PRINTNAME(TAG(args)));
		el = CAR(args);
		mparams[i] = rclr_convert_element(el);
	}
	return mparams;
}

CLR_OBJ* rclr_convert_element_rdotnet(SEXP el)
{
	// The idea here is that we create a SymbolicExpression in C#, that the C# code will intercept
#if MS_CLR
	return rclr_ms_convert_element_rdotnet(el);
#else
    return rclr_mono_convert_element_rdotnet(el);
    // error("%s", "Failure in rclr_convert_element_rdotnet: nor implemented yet on Mono");
#endif
}

CLR_OBJ* rclr_convert_element( SEXP el ) 
{
	int j;
	int lengthArg = 0;
	const char* val;
	double* values;
	int* intVals;
	Rbyte* rawVals;
	Rcomplex* cpl;
	int element_type;
	const char* tzone;
	RCLR_BOOL is_date;
	RCLR_BOOL is_POSIXct;
#if MS_CLR
	double dval;
	VARIANT* result;
	VARTYPE vtype = VT_I4;
#elif MONO_CLR
	CLR_OBJ* parameter;
	void* result;
#endif

	element_type = TYPEOF(el);
	switch (element_type) {
	case S4SXP:
		result = get_clr_object(el);
		return (CLR_OBJ* )result;
	case VECSXP:
		// If this is a list of S4 objects CLR objects, then either we create a safe array of variants, 
		// or we call a C# function that creates an array of objects.
		//SAFEARRAY* safeArray = create_array_double(stringArray, LENGTH(el));
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
	if (uses_rdotnet_for_conversion)
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
				// to notice that be it a DateTime or a Double, what you want to pass as parameters (void** params)
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
			double* dateval_offset;
			if (vtype == VT_DATE )
			{
				dateval_offset = (double*)malloc(sizeof(double)*nVals);
				for (j = 0; j < nVals; j++)
				{
					dateval_offset[j] = is_date ? DateRToCOMdatetime(values[j]) : (is_POSIXct ? PosixCtToCOMdatetime(values[j]) : values[j] );
				}
			}
			SAFEARRAY* safeArray = vtype == VT_DATE ? create_array_dates(dateval_offset, nVals) : create_array_double(values, nVals);
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
			SAFEARRAY* safeArray = create_array_bool(intVals, nVals);
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
			SAFEARRAY* safeArray = create_array_int(intVals, nVals);
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
			SAFEARRAY* safeArray = create_array_bytes(rawVals, nVals);
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
			char** stringArray = (char**)malloc(sizeof(char*)*lengthArg);
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
			SAFEARRAY* safeArray = create_array_strings(stringArray, LENGTH(el));
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
		//SAFEARRAY* safeArray = create_array_double(stringArray, LENGTH(el));
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
	return (CLR_OBJ* ) result;
}

void use_rdotnet_for_conversions( int use_it ) {
    uses_rdotnet_for_conversion = use_it;
}

CLR_OBJ* rclr_create_array_objects( SEXP s ) {
	int nElements = Rf_length(s), i;
#ifdef MS_CLR
	VARIANT** result = new VARIANT*[nElements];
#elif MONO_CLR
	MonoArray* result_arr;
	void** result = (void**)malloc(nElements*sizeof(void*)); // TOCHECK
#endif
	for (i = 0; i < nElements; i++) {
		result[i] = rclr_convert_element(VECTOR_ELT(s, i));
	}
#ifdef MS_CLR
	SAFEARRAY* safeArray = create_safe_array(result, nElements);
	return new CLR_OBJ(rclr_ms_create_vt_array(safeArray, VT_ARRAY | VT_VARIANT));
#elif MONO_CLR
	result_arr = create_array_object(result, nElements);
	free(result);
	return (CLR_OBJ*)result_arr;
#endif
}

SEXP clr_object_to_SEXP(CLR_OBJ* objptr) {
	SEXP result;
	//ClrObjectHandle clroh;
	ClrObjectHandle* clroh_ptr;
	if(objptr == NULL)
		return R_NilValue;
	clroh_ptr=(ClrObjectHandle* )malloc(sizeof(ClrObjectHandle));
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

