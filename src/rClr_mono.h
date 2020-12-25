#pragma once

#ifdef MONO_CLR

// Using v4.0.30319 which is the runtime info for the version 4.5 of the CLR.
// This may be required even if the assemblies are compiled to target the 4.0 runtime.
// Problem arise if 'forcing' the 4.0 runtime if there are extension methods in use.
// See http://r2clr.codeplex.com/workitem/26 and http://lists.ximian.com/pipermail/mono-devel-list/2013-January/040009.html
#define RCLR_DEFAULT_RUNTIME_VERSION "v4.0.30319"

/////////////////////////////////////////
// Imports and includes
/////////////////////////////////////////
#include "common_imports.h"

// stuff to have the mono debugger available. Hopefully.
static const char* options[] = {
      "--soft-breakpoints",
      "--debugger-agent=transport=dt_socket,address=127.0.0.1:10000"
};

#ifdef MONO_INST
#include <mono/jit/jit.h> 
#include <glib.h>  // to get typedef gpointer
// If building against the distributed mono, the following struct declaration is needed. 
// Cannot find it in the header files included.
/* This corresponds to System.Type */
struct _MonoReflectionType {
    MonoObject object;
    MonoType * type;
};
#else
#include <mono/mini/jit.h> // if using mono built with VS from source
#include <mono/metadata/object-internals.h>
#endif

#include <mono/metadata/mono-debug.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/class.h>
//#include <mono/metadata/object-internals.h>

typedef MonoObject CLR_OBJ;


// Import R headers after mono's, to limit the risk of 
// defined macros with generic names (Realloc and co) override function names. Was an issue with WinSDK
#include "r_imports.h"

/////////////////////////////////////////
// Exported methods, specific to hosting Mono
/////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    void** build_method_parameters(SEXP largs);
    SEXP rclr_mono_reflect_object(CLR_OBJ* obj);
    SEXP rclr_mono_get_type_name(SEXP clrObj);
    SEXP rclr_mono_diagnose_method_parameters(SEXP methodParams);
    SEXP rclr_mono_call_static_method(char* ns_qualified_typename, const char* mnam, SEXP methodParams);
    SEXP rclr_mono_call_method(const char* mnam, CLR_OBJ* obj, SEXP methodParams);
    SEXP clr_obj_mono_convert_to_SEXP(CLR_OBJ* pobj);

#ifdef __cplusplus
} // end of extern "C" block
#endif


#define STR_DUP strdup

/////////////////////////////////////////
// Mono specific methods, not dependend on R constructs
/////////////////////////////////////////

SEXP rclr_mono_call_method_with_exception(const char* mnam, CLR_OBJ* obj, MonoClass* klass, SEXP p, CLR_OBJ* exception);
void rclr_mono_load_assembly(char** filename);

#define INIT_CLR_FROM_FILE FALSE
extern MonoDomain* domain;
extern MonoAssembly* assembly;
extern MonoImage* image;
extern MonoClass* spTypeClrFacade;

MonoDomain* get_domain();
MonoAssembly* get_assembly();
MonoImage* get_image();

void rclr_mono_create_domain(char* filename, int mono_debug);
CLR_OBJ* create_object(MonoDomain* domain, MonoImage* image, char* name_space, char* type_short_name);
MonoString* create_mono_string(char* str);
MonoObject* create_mono_double(double* val_ptr);
MonoObject* create_mono_intptr(size_t* val_ptr);
MonoArray* create_array_double(double* values, int length);
MonoArray* create_array_object(void** values, int length);
double* create_array_double_from_monoarray(MonoArray* monoarray);

CLR_OBJ* get_property_value(CLR_OBJ* obj, const char* property_name);
void print_exception(CLR_OBJ* exception, char* property_name);
void print_if_exception(CLR_OBJ* exception);
CLR_OBJ* rclr_mono_create_object( MonoDomain* domain, MonoImage* image, char* ns_qualified_typename, SEXP ctorParams );
CLR_OBJ* rclr_mono_invoke_method_stringarg(MonoMethod* method, char* meth_arg);
CLR_OBJ* rclr_mono_call_static_method_tname(char* ns_qualified_typename, char* mnam, void** params, int paramCount);
CLR_OBJ* rclr_mono_call_static_method_tname_directcall(char* ns_qualified_typename, char* mnam, void** params, int paramCount);
CLR_OBJ* rclr_mono_call_inst_method(const char* mnam, CLR_OBJ* obj, void** params, int param_count);
CLR_OBJ* rclr_mono_get_current_object_direct();
double* clr_datetime_obj_to_r_date_numeric( CLR_OBJ* pobj );

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
	type* values; \
	int length = mono_array_length(monoarray); \
	values = (type*)malloc(length*sizeof(type)); \
	for (j = 0; j < length; j++) \
{ \
	values[j] = mono_array_get(monoarray, type, j); \
} \
	return values; \
	} while (0)

double* create_array_double_from_monoarray( MonoArray* monoarray );
MonoArray* create_array_double( double* values, int length );

MonoArray* create_array_int( int* values, int length );

MonoArray* create_array_bytes( unsigned char* values, int length );

MonoArray* create_array_bool( int* values, int length );

MonoArray* create_array_object( void** values, int length );

MonoString* create_mono_string( char* str );

MonoObject* create_mono_double( double* val );

MonoObject* create_mono_int32( int* val );

MonoObject* create_mono_int64( long* val );

MonoObject* create_mono_bool( int* val );

MonoObject* create_mono_intptr( size_t* val );

#endif // MONO_CLR
