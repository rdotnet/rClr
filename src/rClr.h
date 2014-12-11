#ifndef __RCLR_H__
#define __RCLR_H__

// #define RCLR_VER 0x000100 /* RCLR v0.1-0 */

#ifndef MS_CLR
#ifndef MONO_CLR
#define MS_CLR 1
#endif
#endif


/////////////////////////////////////////
// Imports and includes
/////////////////////////////////////////


#ifdef MONO_CLR
// define these to keep using booleans with MS CPP. Feels kludgy, but so long.
#define TRUE_BOOL 1;
#define FALSE_BOOL 0;

typedef int RCLR_BOOL;

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
	MonoType  *type;
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

#elif MS_CLR

#include <windows.h>
#include <fstream>
#include <cstdlib>
#include <metahost.h>
//#include "stdafx.h"
//#include "atlbase.h"
#include <comutil.h>
#include <tchar.h> 

// define these to keep using booleans with MS CPP. Feels kludgy, but so long.
typedef bool RCLR_BOOL;
#define TRUE_BOOL true;
#define FALSE_BOOL false;

#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h> // TODO: The usage seems more to be with S code. Consider try to stick to Rinternals only, if this is anyway a superset
#include <Rversion.h>
#include <R_ext/Callbacks.h>
#include <stdint.h>

#ifdef MS_CLR
#pragma comment(lib, "mscoree.lib")

#ifdef MS_CLR_TLB
//Import mscorlib.tlb (Microsoft Common Language Runtime Class Library).
#import <mscorlib.tlb> raw_interfaces_only				\
	high_property_prefixes("_get","_put","_putref")		\
	rename("ReportEvent", "InteropServices_ReportEvent")
#else
#import "mscorlib.tlh"
#endif
using namespace mscorlib;
typedef variant_t CLR_OBJ;

#endif

typedef struct {
	CLR_OBJ * objptr;
	uint32_t handle; // TOCHECK: is this useful for both Mono and ms.net?
} ClrObjectHandle;

#define GET_CLR_OBJ_FROM_EXTPTR(extptrsexp) ((ClrObjectHandle*)EXTPTR_PTR(extptrsexp))->objptr


/////////////////////////////////////////
// Exported methods (exported meaning on Windows platform)
/////////////////////////////////////////


#ifdef MS_CLR
extern "C" {
	SEXP rclr_ms_get_type_name(SEXP clrObj);
	SEXP rclr_ms_reflect_object(CLR_OBJ * objptr);
	SEXP clr_obj_ms_convert_to_SEXP(CLR_OBJ &pobj);
#endif
	SEXP r_create_clr_object( SEXP p );
	SEXP r_get_null_reference();
	SEXP r_reflect_on_object(SEXP clrobj);
	SEXP r_get_type_name(SEXP clrObj);
	SEXP r_call_static_method(SEXP p);
	SEXP r_diagnose_parameters(SEXP p);
	SEXP r_call_method(SEXP par);
	SEXP r_get_typename_externalptr(SEXP p);
	SEXP make_char_single_sexp(const char* str);

	/**
	 * \brief	Gets a SEXP, bypassing the custom data converters e.g. offered by RDotNetDataConverter. Solves issue rClr#33
	 *
	 * \return	a SEXP representing the object handled by the CLR conversion facade, if any.
	 */
	SEXP r_get_object_direct();
#ifdef MONO_CLR
	void ** build_method_parameters(SEXP largs);
	SEXP rclr_mono_reflect_object(CLR_OBJ * obj);
	SEXP rclr_mono_get_type_name(SEXP clrObj);
	SEXP rclr_mono_diagnose_method_parameters(SEXP methodParams);
	SEXP rclr_mono_call_static_method(char * ns_qualified_typename, const char *mnam, SEXP methodParams);
	SEXP rclr_mono_call_method(const char *mnam, CLR_OBJ * obj, SEXP methodParams);
	SEXP clr_obj_mono_convert_to_SEXP( CLR_OBJ * pobj);
#elif MS_CLR
	VARIANT ** build_method_parameters(SEXP largs);
#endif
	void get_ns_and_type( SEXP p, char ** name_space, char ** type_short_name );
	void get_FullTypeName( SEXP p, char ** tname);
	void rclr_load_assembly(char ** filename);
	void rclr_create_domain(char ** appBaseDir, char ** filename, int* mono_debug);
	int is_microsoft_clr();
	void rclr_cleanup();
	static void clr_object_finalizer(SEXP ref);
	int use_rdotnet = 0;
#ifdef MS_CLR
} // end of extern "C" block
#endif


#ifdef MONO_CLR

SEXP rclr_mono_call_method_with_exception(const char * mnam, CLR_OBJ * obj, MonoClass * klass, SEXP p, CLR_OBJ * exception);
void rclr_mono_load_assembly( char ** filename );

#endif

CLR_OBJ * rclr_convert_element( SEXP el );
CLR_OBJ * rclr_mono_convert_element_rdotnet(SEXP el);
CLR_OBJ * rclr_create_array_objects( SEXP s );
CLR_OBJ * rclr_wrap_data_frame( SEXP s );
CLR_OBJ * get_clr_object( SEXP clrObj );
CLR_OBJ * create_clr_complex_direct(Rcomplex * complex, int length);
SEXP clr_object_to_SEXP(CLR_OBJ *o);
const char * get_type_full_name(CLR_OBJ *objptr);



#ifndef  __cplusplus
#define STR_DUP strdup
#else
#define STR_DUP _strdup
#endif



// Getting the offset for VT_DATE between R and .NET: the information is contradictory. Following the SECOND seems to work.
		
// SEEMS INCORRECT:
// http://msdn.microsoft.com/en-us/library/aa908601.aspx: midnight, January 1, 1900 is 2.0, January 2, 1900 is 3.0, and so on
// > as.numeric(as.Date('1970-01-01'))  returns [1] 0
//as.numeric(as.Date('1900-01-01'))     [1] -25567

// SEEMS CORRECT:
//HOWEVER in http://msdn.microsoft.com/en-us/library/vstudio/system.datetime%28v=vs.100%29.aspx
//If you round-trip only a time, such as 3 P.M., the final date and time is December 30, 1899 C.E. at 3:00 P.M., 
//instead of January, 1, 0001 C.E. at 3:00 P.M. This happens because the .NET Framework and COM assume a default date 
//when only a time is specified. However, the COM system assumes a base date of December 30, 1899 C.E. while the .NET
//Framework assumes a base date of January, 1, 0001 C.E. 
//> as.numeric(as.Date('1899-12-30'))
//[1] -25569
const double OFFSET_DATE_NUMERIC = 25569;
#define COMdatetimeToDateR(cdt) (cdt - OFFSET_DATE_NUMERIC)
#define DateRToCOMdatetime(rdt) (rdt+OFFSET_DATE_NUMERIC)

//> as.numeric(as.POSIXct('1899-12-30 00:00:00', tz='UTC'))
//[1] -2209161600
const double OFFSET_DATE_POSIX_CT = 2209161600;
const double SECONDS_PER_DAY = 86400;

// handle the peculiarities of OLEAUT dates before/after '1899-12-30 00:00:00'
// http://blogs.msdn.com/b/ericlippert/archive/2003/09/16/eric-s-complete-guide-to-vt-date.aspx
inline double oleautdate_to_linear(double oleaut_date) { 
	// So what is 1.75? That's 6PM , 31 Dec 1899. What about -1.75? That's 6 PM 29 Dec 1899	
	// 0.75 and -0.75.  those are zero and "minus zero" days from 30 December 1899, 6 PM
	// For a conversion to POSIXct I want 
	// 1899-12-31 18:00:00 to be 1.75	i.e. result from conversion of oleaut 1.75
	// 1899-12-30 18:00:00 to be 0.75	i.e. result from conversion of oleaut 0.75 and -0.75
	// 1899-12-30 00:00:00 to be zero   i.e. result from conversion of oleaut 0
	// 1899-12-29 18:00:00 to be -0.25	i.e. result from conversion of oleaut -1.75
	// 1899-12-28 18:00:00 to be -1.25	i.e. result from conversion of oleaut -2.75  (-1.25 = (-2) + 0.75 = (ceiling(-2.75)+ abs(-2.75)- floor(abs(-2.75)))
	double absval = abs(oleaut_date); 
	if ( oleaut_date < 0.0 ) {
		absval = abs(oleaut_date); 
		return (ceil(oleaut_date) + absval - floor(absval));
	}
	else 
		return oleaut_date;
	// anything positive unchanged
	// 0 -> 0 + 0 - 0 = 0
	// -0.75 -> 0 + 0.75 - 0 = 0.75
	// -1    -> -1 + 1 - 1 = -1
	// -1.75 -> -1 + 1.75 - 1 = -2+1.75=-0.25 
	// -2.75 -> -2 + 2.75 - 2 = -4+2.75=-1.25 
}

inline double linear_to_oleautdate(double linear_date) { 
	double absval;
	double decimal_abs, floorval;
	if ( linear_date < 0.0 ) {
		absval = abs(linear_date); 
		decimal_abs = absval - floor(absval);
		floorval = floor(linear_date);
		return floorval -(ceil(linear_date)-floorval) + decimal_abs; 
	}
	else
		return linear_date;
	// -0.25 -> -1 -(0-(-1)) + 0.25 = -1.75
	// -1    -> -1 -((-1)-(-1)) + 0 = -1
	// -0.75 -> -1 -(0-(-1)) + 0.75 = -1.25
	// -1.25 -> -2 -((-1)-(-2)) + 0.25 = -2 -1 + 0.25 = -2.75
}

#define COMdatetimeToPosixCt(cdt) (oleautdate_to_linear(cdt)*SECONDS_PER_DAY - OFFSET_DATE_POSIX_CT)
// COMdatetimeToPosixCt(0) is equal to -OFFSET_DATE_POSIX_CT
// COMdatetimeToPosixCt(1) is equal to:
//> as.numeric(as.POSIXct('1899-12-31'))
//[1] -2209111200
#define PosixCtToCOMdatetime(pdt) (linear_to_oleautdate((pdt+OFFSET_DATE_POSIX_CT)/SECONDS_PER_DAY))


/////////////////////////////////////////
// Mono/MS.NET specific implementation, not dependend on R constructs
/////////////////////////////////////////


#ifdef MONO_CLR

// Using v4.0.30319 which is the runtime info for the version 4.5 of the CLR.
// This may be required even if the assemblies are compiled to target the 4.0 runtime.
// Problem arise if 'forcing' the 4.0 runtime if there are extension methods in use.
// See http://r2clr.codeplex.com/workitem/26 and http://lists.ximian.com/pipermail/mono-devel-list/2013-January/040009.html
#define RCLR_DEFAULT_RUNTIME_VERSION "v4.0.30319"
#define INIT_CLR_FROM_FILE FALSE
MonoDomain *domain = NULL;
MonoAssembly *assembly;
MonoImage *image;
MonoClass * spTypeClrFacade = NULL;

MonoDomain * get_domain();
MonoAssembly * get_assembly();
MonoImage * get_image();

void rclr_mono_create_domain( char* filename, int mono_debug);
CLR_OBJ * create_object (MonoDomain *domain, MonoImage *image, char * name_space, char * type_short_name);
MonoString * create_mono_string(char * str);
MonoObject * create_mono_double(double * val_ptr);
MonoObject * create_mono_intptr(size_t * val_ptr);
MonoArray * create_array_double( double * values, int length );
MonoArray * create_array_object( void ** values, int length );
double * create_array_double_from_monoarray( MonoArray* monoarray );

CLR_OBJ * get_property_value( CLR_OBJ * obj, const char * property_name );
void print_exception( CLR_OBJ * exception, char * property_name);
void print_if_exception( CLR_OBJ * exception );
CLR_OBJ * rclr_mono_invoke_method_stringarg( MonoMethod * method, char * meth_arg);
CLR_OBJ * rclr_mono_call_static_method_tname(char * ns_qualified_typename, char * mnam, void ** params, int paramCount);
CLR_OBJ * rclr_mono_call_static_method_tname_directcall(char * ns_qualified_typename, char * mnam, void ** params, int paramCount);
CLR_OBJ * rclr_mono_call_inst_method(const char *mnam, CLR_OBJ * obj, void ** params, int param_count );
double * clr_datetime_obj_to_r_date_numeric(CLR_OBJ * pobj);

#elif MS_CLR

ICLRMetaHost *pMetaHost = NULL;
ICLRRuntimeInfo *pRuntimeInfo = NULL;
ICLRRuntimeHost *pClrRuntimeHost = NULL;
_AssemblyPtr spAssembly = NULL;
PCWSTR pszVersion = L"v4.0.30319";
_AppDomainPtr spDefaultAppDomain = NULL;
DWORD domainId = NULL;
FExecuteInAppDomainCallback callback = NULL; 
PCWSTR pwzDllName = L"ClrFacade";
#ifdef USE_COR_RUNTIME_HOST
ICorRuntimeHost *pCorRuntimeHost = NULL;
#else
ICLRRuntimeHost    *pRuntimeHost    = nullptr;
#endif

IUnknownPtr spAppDomainThunk = NULL;
bstr_t bstrClassName(L"Rclr.ClrFacade");
_TypePtr spTypeClrFacade = NULL;
variant_t vtEmpty;

char * bstr_to_c_string(bstr_t * src);
char * getComErrorMsg(HRESULT hr);
void ms_rclr_cleanup();
void start_ms_clr();
void rclr_ms_create_domain(char ** appBaseDir);

HRESULT rclr_ms_call_static_method_facade(char * methodName, CLR_OBJ * objptr, VARIANT * result);
HRESULT rclr_ms_load_assembly(char ** filename, VARIANT * vtResult);
HRESULT rclr_ms_call_static_method_stringarg(_TypePtr spType, bstr_t * bstrStaticMethodNamePtr, char * strArg, VARIANT * vtResult);
HRESULT rclr_ms_call_method_stringarg(CLR_OBJ * obj, const char *mnam, char * strArg, VARIANT * vtResult);
HRESULT rclr_ms_create_object(char * longtypename, VARIANT ** params, int argLength, VARIANT * vtResult);
HRESULT rclr_ms_call_static_method(_TypePtr spType, bstr_t * bstrStaticMethodNamePtr, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult);
HRESULT rclr_ms_get_facade_typeref(_Type ** spType);
HRESULT rclr_ms_call_method_argless(CLR_OBJ * obj, const char *mnam, VARIANT * vtResult);
HRESULT rclr_ms_call_method_interface(CLR_OBJ * obj, const char *interfaceName, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult);
HRESULT rclr_ms_call_method_type(CLR_OBJ * obj, _TypePtr pType, const char *mnam, SAFEARRAY * psaStaticMethodArgs, VARIANT * vtResult);
HRESULT rclr_ms_gettype(CLR_OBJ * obj, _Type ** pType);
HRESULT rclr_ms_call_method(CLR_OBJ * objptr, char * methodName, VARIANT ** params, int argLength, VARIANT * vtResult);
HRESULT rclr_ms_get_raw_object(CLR_OBJ * obj, _Object ** ppObject);
HRESULT rclr_ms_call_static_method_tname(char * ns_qualified_typename, char * mnam, VARIANT ** params, int argLength, VARIANT * result);
HRESULT rclr_ms_get_current_object_direct(VARIANT * vtResult);

SAFEARRAY * rclr_ms_create_method_call_parameters(CLR_OBJ * objptr, char * mnam, VARIANT ** params, int paramsArgLength );
SAFEARRAY * rclr_ms_create_static_method_call_parameters(char * assemblyQualifiedTypeName, char * mnam, VARIANT ** params, int paramsArgLength );
SAFEARRAY * rclr_ms_create_constructor_parameters(VARIANT * class_name, VARIANT ** params, int paramsArgLength );
SAFEARRAY * create_array_one_string(char * strArg);
SAFEARRAY * create_array_double(double * values, int length);
SAFEARRAY * create_array_dates( double * values, int length );
SAFEARRAY * create_array_int( int * values, int length );
SAFEARRAY * create_array_bool( int * values, int length );
SAFEARRAY * create_array_strings(char ** values, int length);
SAFEARRAY * create_array_bytes(unsigned char * values, int length);
SAFEARRAY * create_safe_array(VARIANT ** values, int length);
VARIANT * rclr_ms_create_vt_array(SAFEARRAY * safeArray, VARTYPE vartype);

void free_variant_array(VARIANT ** a, int size);
void rclr_ms_fill_array_from_index_two(SAFEARRAY * psaStaticMethodArgs, VARIANT ** params, int paramsArgLength);

#endif


#endif
