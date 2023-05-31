#ifndef __RCLR_H__
#define __RCLR_H__

// #define RCLR_VER 0x000100 /* RCLR v0.1-0 */

/////////////////////////////////////////
// Imports and includes
/////////////////////////////////////////

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



#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h> // TODO: The usage seems more to be with S code. Consider try to stick to Rinternals only, if this is anyway a superset
#include <Rversion.h>
#include <R_ext/Callbacks.h>
#include <stdint.h>
#include <vector>

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



#ifdef __cplusplus
extern "C" {
#endif

	SEXP rclr_ms_get_type_name(SEXP clrObj);
	SEXP rclr_ms_reflect_object(CLR_OBJ * objptr);
	SEXP clr_obj_ms_convert_to_SEXP(CLR_OBJ &pobj);
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
	VARIANT ** build_method_parameters(SEXP largs);
    SEXP clr_object_to_SEXP(CLR_OBJ *o);
	void get_ns_and_type( SEXP p, char ** name_space, char ** type_short_name );
	void get_FullTypeName( SEXP p, char ** tname);
	void rclr_load_assembly(char ** filename);
	void rclr_create_domain(char ** appBaseDir, char ** filename, int* mono_debug);
	int is_microsoft_clr();
	void rclr_cleanup();
	static void clr_object_finalizer(SEXP ref);
	int use_rdotnet = 0;

#ifdef __cplusplus
} // end of extern "C" block
#endif

CLR_OBJ * rclr_convert_element( SEXP el );
CLR_OBJ * rclr_mono_convert_element_rdotnet(SEXP el);
CLR_OBJ * rclr_create_array_objects( SEXP s );
CLR_OBJ * rclr_wrap_data_frame( SEXP s );
CLR_OBJ * get_clr_object( SEXP clrObj );
CLR_OBJ * create_clr_complex_direct(Rcomplex * complex, int length);
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
	double absval = std::abs(oleaut_date); 
	if ( oleaut_date < 0.0 ) {
		absval = std::abs(oleaut_date); 
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
		absval = std::abs(linear_date); 
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
// MS.NET specific implementation, not dependend on R constructs
/////////////////////////////////////////


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

// A vector to store transient CLR object handles that we need to clear on leaving the native interop layer.
std::vector<VARIANT*> transientArgs;


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
void release_transient_objects();
void rclr_ms_fill_array_from_index_two(SAFEARRAY * psaStaticMethodArgs, VARIANT ** params, int paramsArgLength);

