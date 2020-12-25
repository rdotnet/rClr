#ifdef MONO_CLR

#include "rClr.h"

// A vector to store transient CLR object handles that we need to clear on leaving the native interop layer.
std::vector<CLR_OBJ*> transientArgs;

// Initialise global variables here, not in the header
MonoDomain* domain = nullptr;
MonoAssembly* assembly = nullptr;
MonoImage* image = nullptr;
MonoClass* spTypeClrFacade = nullptr;


MonoDomain* get_domain() { return domain; }
MonoAssembly* get_assembly() { return assembly; }
MonoImage* get_image() { return image; }


MonoMethod* rclr_mono_get_method( MonoClass* klass, char* mnam, int param_counts ) {
    return mono_class_get_method_from_name( klass, mnam, param_counts );
}


CLR_OBJ* rclr_mono_create_object (MonoDomain* domain, MonoImage* image, char* ns_qualified_typename, SEXP ctorParams) {	
	MonoMethod* method = rclr_mono_get_method( spTypeClrFacade, "CreateInstance", 2);
	MonoArray* ctor_params = create_array_object(build_method_parameters(ctorParams), Rf_length(ctorParams));
	MonoObject* exception, *result;
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

CLR_OBJ* rclr_mono_invoke_method_stringarg( MonoMethod* method, char* meth_arg) { 
	CLR_OBJ* result;
	MonoObject* exception = NULL;
	MonoString* monoStrArg = create_mono_string(meth_arg);
	result = mono_runtime_invoke (method, NULL, (void**)(&monoStrArg), &exception);
	print_if_exception(exception);
	return result;
}

CLR_OBJ* create_object (MonoDomain* domain, MonoImage* image, char* name_space, char* type_short_name)
{
	MonoClass* klass;
	CLR_OBJ* obj;

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

CLR_OBJ* get_property_value( CLR_OBJ* obj, const char* property_name ) {

	MonoClass* klass;
	MonoMethod* method = NULL;
	MonoProperty* prop;

	klass = mono_object_get_class (obj);
	prop = mono_class_get_property_from_name (klass, property_name);
	return mono_property_get_value (prop, obj, NULL, NULL);
}

CLR_OBJ* rclr_mono_get_current_object_direct() {
	MonoMethod* method = rclr_mono_get_method( spTypeClrFacade, "get_CurrentObject", 0);
	MonoObject* exception;
	CLR_OBJ* result;
	result = mono_runtime_invoke(method, NULL, NULL, &exception);
	print_if_exception(exception);
	return result;
}


void print_to_R(MonoString* monoString, void f(const char* , ...), const char* msg_container)
{
	char* msg;
	msg = mono_string_to_utf8(monoString);
	f(msg_container, msg);
	mono_free(msg);
}

void print_as_error(MonoString* monoString, const char* msg_container)
{
	print_to_R(monoString, error, msg_container);
}

void print_as_warning(MonoString* monoString, const char* msg_container)
{
	print_to_R(monoString, warning, msg_container);
}

void print_exception(CLR_OBJ* exception, char* property_name)
{
	print_to_R((MonoString*)get_property_value(exception, property_name), Rprintf, "%s\n");
}

MonoString* rclr_mono_get_last_clr_exception() {
	MonoMethod* method = rclr_mono_get_method(spTypeClrFacade, "get_LastCallException", 0);
	MonoObject* exception;
	CLR_OBJ* result;
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

void print_if_exception( CLR_OBJ* exception )
{
	CLR_OBJ* inner_exception;
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

const char* getKlassName(CLR_OBJ* obj)
{
	return mono_class_get_name(mono_object_get_class(obj));
}

SEXP rclr_mono_call_method_with_exception(const char* mnam, CLR_OBJ* obj, MonoClass* klass, SEXP methodParams, CLR_OBJ* exception) {
	MonoMethod* methodCallInstanceMethod = rclr_mono_get_method( spTypeClrFacade, "CallInstanceMethod", 3);
	CLR_OBJ* result;
	//char* debug_test;
	int paramCount = Rf_length(methodParams);
	void** params = build_method_parameters(methodParams);
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

SEXP rclr_mono_call_method_with_exception_directcall(const char* mnam, CLR_OBJ* obj, MonoClass* klass, SEXP methodParams, CLR_OBJ* exception) {
	CLR_OBJ* result;
	MonoMethod* method = mono_class_get_method_from_name(klass, mnam, Rf_length(methodParams));
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

SEXP rclr_mono_call_method(const char* mnam, CLR_OBJ* obj, SEXP methodParams )
{
	SEXP result;
	CLR_OBJ* exception = NULL;
	//MonoMethod* method = NULL;
	MonoClass* klass;
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

double* create_array_double_from_monoarray( MonoArray* monoarray )
{
	create_array_from_monoarray(double, monoarray);
}

MonoArray* create_array_double( double* values, int length )
{
	create_array_oftype(double, mono_get_double_class, values, length);
}

MonoArray* create_array_int(int* values, int length)
{
	create_array_oftype(int, mono_get_int32_class, values, length);
}

MonoArray* create_array_bytes(unsigned char* values, int length)
{
	create_array_oftype(unsigned char, mono_get_byte_class, values, length);
}

MonoArray* create_array_bool( int* values, int length )
{
	create_array_oftype(int, mono_get_boolean_class, values, length);
}

MonoArray* create_array_object( void** values, int length )
{
	create_array_oftype(void*, mono_get_object_class, values, length);
}

MonoString* create_mono_string(char* str) {
	const char* str2 = STR_DUP(str);
	return mono_string_new (domain, str2);
}

MonoObject* create_mono_double(double* val) {
	return mono_value_box(domain, mono_get_double_class(), val);
}

MonoObject* create_mono_int32(int* val) {
	return mono_value_box(domain, mono_get_int32_class(), val);
}

MonoObject* create_mono_int64(long* val) {
	return mono_value_box(domain, mono_get_int64_class(), val);
}

MonoObject* create_mono_bool(int* val) {
	return mono_value_box(domain, mono_get_boolean_class(), val);
}

MonoObject* create_mono_intptr(size_t* val) {
	return mono_value_box(domain, mono_get_intptr_class(), val);
}

void release_transient_objects() {
	for (size_t i = 0; i < transientArgs.size(); i++) {
		delete transientArgs.at(i);
	}
	transientArgs.clear();
}

double* clr_datetimearray_obj_to_numeric(CLR_OBJ* datetimearray_ptr, MonoMethod* method) {
	MonoObject* exception;
	void** params;
	params = (void**) &(datetimearray_ptr);
	return create_array_double_from_monoarray((MonoArray*)mono_runtime_invoke(method, NULL, params, &exception));
}

double* clr_datetimearray_obj_to_r_date_numeric(CLR_OBJ* datetimearray_ptr) {
	return clr_datetimearray_obj_to_numeric( datetimearray_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDateDoubleRepresentations", 1));
}

double* clr_datetimearray_obj_to_r_posixct_numeric(CLR_OBJ* datetimearray_ptr) {
	return clr_datetimearray_obj_to_numeric( datetimearray_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDatePosixtcNumericValues", 1));
}

double* clr_datetime_obj_to_double(CLR_OBJ* datetime_ptr, MonoMethod* method) {
	MonoObject* exception;
	void* unboxed_obj;
	void** params;
	unboxed_obj = mono_object_unbox(datetime_ptr);
	params = (void**) &(unboxed_obj);
	return (double*)mono_object_unbox(mono_runtime_invoke(method, NULL, params, &exception));
}

double* clr_datetime_obj_to_r_date_numeric(CLR_OBJ* datetime_ptr) {
	return clr_datetime_obj_to_double( datetime_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDateDoubleRepresentation", 1));
	//char* mnam = "get_Ticks";
	//CLR_OBJ* nTicksObj = rclr_mono_call_inst_method(mnam, pobj, NULL, 0);
	//long long* nTicks = (long long*)mono_object_unbox(nTicksObj);
	//long long offset = (*nTicks)-nineteenHundsTicks;
	//double result = (double)offset / ticksPerDay;
	//return &result;
}

double* clr_datetime_obj_to_r_posixtc_numeric(CLR_OBJ* datetime_ptr) {
	return clr_datetime_obj_to_double( datetime_ptr, rclr_mono_get_method( spTypeClrFacade, "GetRDatePosixtcNumericValue", 1));
	//char* mnam = "get_Ticks";
	//CLR_OBJ* nTicksObj = rclr_mono_call_inst_method(mnam, pobj, NULL, 0);
	//long long* nTicks = (long long*)mono_object_unbox(nTicksObj);
	//long long offset = (*nTicks)-nineteenHundsTicks;
	//double result = (double)offset / ticksPerDay;
	//return &result;
}

SEXP clr_obj_mono_convert_to_SEXP( CLR_OBJ* pobj) {
	SEXP result = NULL;
	char* klassName;
	/*MonoTypeEnum*/ int type_il;
	int n, j;
	char* str;
	double* values;
	MonoArray* monoarray;
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
					double* valueInREpoch = clr_datetime_obj_to_r_posixtc_numeric(pobj);
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
				char** stringArray = (char**)malloc(sizeof(char*)*n);
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
				values = clr_datetimearray_obj_to_r_posixct_numeric((MonoObject*)monoarray);
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

SEXP rclr_mono_reflect_object(CLR_OBJ* obj) {
	SEXP result;
	int nmeth = 0, nfields = 0;
	int i;
	int counter = 0;

	void* iter;
	char** mnames = NULL; //= getNames(clrobj);
	MonoClass* klass;
	MonoClassField* field;
	MonoMethod* method = NULL, *m = NULL;
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
	CLR_OBJ* pbjptr = get_clr_object(clrObj);
	return make_char_single_sexp( get_type_full_name(pbjptr) );
}


/* This seems buggy.
void get_ns_and_type_from_fqtn(char* ns_qualified_typename, char** name_space, char** type_short_name)
{
char* tmp = STR_DUP(ns_qualified_typename);
char* long_type_name = strtok( tmp, ","); // My.Namespace.ShortTypeName,MyAssembly ==> My.Namespace.ShortTypeName
// MyNamespace.ShortTypeName
char* cand_short_type_name = (strrchr( long_type_name, (int)('.')) + 1); // get a pointer to 'ShortTypeName'
size_t count = (cand_short_type_name-1) - long_type_name; // 'a.b.c.d' : 6 - 1 - 0 = 5 characters, i.e. 'a.b.c'
char* n_s = (char*) malloc((count+1)*sizeof(char));
strncpy(n_s, long_type_name, count);
n_s[count] = '\0';
*name_space = n_s;
*type_short_name = cand_short_type_name;
free(tmp);
}
*/

SEXP rclr_mono_call_static_method(char* ns_qualified_typename, const char* mnam, SEXP methodParams) {
	CLR_OBJ* obj = NULL;
	CLR_OBJ* result = NULL;
	// char* name_space;
	//	char* type_short_name;
	//get_ns_and_type_from_fqtn(ns_qualified_typename, &name_space, &type_short_name);
	result = rclr_mono_call_static_method_tname(ns_qualified_typename, (char*)mnam, build_method_parameters(methodParams), Rf_length(methodParams));
	return clr_obj_mono_convert_to_SEXP(result);
}

SEXP rclr_mono_diagnose_method_parameters(SEXP methodParams) {
	CLR_OBJ* obj = NULL;
	MonoMethod* methodCallStaticMethod = rclr_mono_get_method(spTypeClrFacade, "DiagnoseMethodCall", 1);
	MonoObject* exception, *result;
	void** static_mparams = (void**)malloc(1* sizeof(void*));
	MonoArray* methParams = create_array_object(build_method_parameters(methodParams), Rf_length(methodParams));
	static_mparams[0] = methParams;
	result = mono_runtime_invoke(methodCallStaticMethod, NULL, static_mparams, &exception);
	print_if_exception(exception);
	free(static_mparams);
	return clr_obj_mono_convert_to_SEXP(result);
}

CLR_OBJ* rclr_mono_call_static_method_tname(char* ns_qualified_typename, char* mnam, void** params, int paramCount) {
	MonoMethod* methodCallStaticMethod = rclr_mono_get_method(spTypeClrFacade, "CallStaticMethodMono", 3);
	MonoObject* exception, *result;
	void** static_mparams = (void**)malloc(3* sizeof(void*)); // TOCHECK
	MonoArray* methParams = create_array_object(params, paramCount);
	static_mparams[0] = create_mono_string(ns_qualified_typename);
	static_mparams[1] = create_mono_string(mnam);
	static_mparams[2] = methParams;
	result = mono_runtime_invoke(methodCallStaticMethod, NULL, static_mparams, &exception);
	print_if_exception(exception);
	free(static_mparams);
	return result;
}

CLR_OBJ* rclr_mono_convert_element_rdotnet(SEXP el)
{
	// The idea here is that we create a SymbolicExpression in C#, that the C# code will intercept
	CLR_OBJ* obj = NULL;
	MonoMethod* method = 
		rclr_mono_get_method(spTypeClrFacade, "CreateSexpWrapper", 1);
		//rclr_mono_get_method(spTypeClrFacade, "CreateSexpWrapperMs", 1);
	MonoObject* exception, *result;
	void** static_mparams = (void**)malloc(1* sizeof(void*));
	// Note to self: it took me some trial and error to figure out what to pass in order to have things working. Not sure why &el directly, but this works...
	// MonoArray* methParams = create_array_object(el, 1);
	// static_mparams[0] = create_mono_intptr(el);
	// static_mparams[0] = create_mono_intptr(&el);
	// static_mparams[0] = create_mono_int64(&el);
	// static_mparams[0] = create_mono_int64(el);
	static_mparams[0] = &el;
	result = mono_runtime_invoke(method, NULL, static_mparams, &exception);
	print_if_exception(exception);
	transientArgs.push_back(result);
	free(static_mparams);
	return result;
}

// This calls the mono method directly on the static class, rather than going through the ClrFacade.
// To keep consistency with the MS.NET and allow for the interception of the result for custom data marshalling, usage is not recommended.
CLR_OBJ* rclr_mono_call_static_method_tname_directcall(char* ns_qualified_typename, char* mnam, void** params, int paramCount) {
	MonoMethod* methodGetType = rclr_mono_get_method( spTypeClrFacade, "GetType", 1);
	CLR_OBJ* type = rclr_mono_invoke_method_stringarg(methodGetType, ns_qualified_typename);
	MonoReflectionType* theType;
	MonoClass* klass;
	MonoMethod* method;
	MonoObject* exception, *result;
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

CLR_OBJ* rclr_mono_call_inst_method(const char* mnam, CLR_OBJ* obj, void** params, int param_count ) {
	MonoClass* klass;
	MonoMethod* method;
	CLR_OBJ* result = NULL;
	CLR_OBJ* exception = NULL;
	klass = mono_object_get_class(obj);
	method = mono_class_get_method_from_name(klass, mnam, param_count);
	result = mono_runtime_invoke (method, obj, params, &exception);
	print_if_exception(exception);
	return result;
}


/////////////////////////////////////////
// Functions without R specific constructs
/////////////////////////////////////////

void rclr_mono_create_domain( char* filename, int mono_debug) {
	if(domain != NULL)
	{
		warning("rclr_mono_create_domain called with argument %s, but the domain appears to already set. Exiting and ignoring this call.", filename);
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
			error("Failed to create the rClr MonoDomain. Requested runtime was %s", RCLR_DEFAULT_RUNTIME_VERSION);
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
		error("Failed to open the assembly containing the CLR facade. Mono assembly seems to be a null pointer");
		return;
	}
	image = mono_assembly_get_image (assembly);
	if (!image) {
		error("Failed to get an image of the assembly containing the CLR facade. Mono image seems to be a null pointer");
		return;
	}
	/*
	* mono_jit_exec() will run the Main() method in the assembly.
	* The return value needs to be looked up from
	* System.Environment.ExitCode.
	*/
	mono_jit_exec (domain, assembly, 1, &filename);

	spTypeClrFacade = mono_class_from_name( image, "Rclr", "ClrFacade");
	if (!spTypeClrFacade) {
		error("Failed to load the CLR facade: MonoClass pointer for Rclr.ClrFacade seems to still be a null pointer");
		return;
	}
}

void rclr_mono_load_assembly( char** filename )
{
	MonoMethod* method = rclr_mono_get_method( spTypeClrFacade, "LoadFrom", 1);
	MonoString* fname = create_mono_string(*filename);
	CLR_OBJ* result;
	MonoObject* exception = NULL;
	result = mono_runtime_invoke (method, NULL, (void**)(&fname), &exception);
	print_if_exception(exception);
}


const char* get_type_full_name(CLR_OBJ* objptr) {
	MonoClass* klass;
	char result[1024]; // Well, I know...
	klass = mono_object_get_class(objptr);
	if ( !klass ) return "";

	result[0] = 0;
	strcat(result,mono_class_get_namespace(klass));
	strcat(result,".");
	strcat(result,mono_class_get_name(klass));
	return STR_DUP(result);
}

#endif
