#pragma once

// #define RCLR_VER 0x000100 /* RCLR v0.1-0 */

<<<<<<< HEAD
=======
// 202005 Temporarily to faciliate debugging from VSCode
#ifndef MONO_CLR
#ifdef __linux__
#define MONO_CLR
#endif 
#endif 

#ifndef MS_CLR
#ifndef MONO_CLR
#define MS_CLR 1
#endif
#endif


>>>>>>> 7c74638017417478fa3772052b4eff6df5c42a15
/////////////////////////////////////////
// Imports and includes
/////////////////////////////////////////

#ifdef MS_CLR
#include "rClr_windows_dotnet.h"
#elif defined MONO_CLR
#include "rClr_mono.h"
#endif

/////////////////////////////////////////
// Elsewhere in this file are functions declarations 
// that are or at least "look" platform independent
/////////////////////////////////////////

typedef struct {
    CLR_OBJ* objptr;
    uint32_t handle; // TOCHECK: is this useful for both Mono and ms.net?
} ClrObjectHandle;

#define GET_CLR_OBJ_FROM_EXTPTR(extptrsexp) ((ClrObjectHandle*)EXTPTR_PTR(extptrsexp))->objptr

// define these to keep using booleans with MS CPP. Feels kludgy, but so long.
typedef bool RCLR_BOOL;
#define TRUE_BOOL true;
#define FALSE_BOOL false;

/////////////////////////////////////////
// Exported methods (exported meaning on Windows platform)
/////////////////////////////////////////


#ifdef __cplusplus
extern "C" {
#endif

    SEXP r_create_clr_object( SEXP p );
    SEXP r_get_null_reference();
    SEXP r_get_type_name( SEXP clrObj );
    SEXP r_call_static_method( SEXP p );
    SEXP r_diagnose_parameters( SEXP p );
    SEXP r_call_method( SEXP par );
    SEXP r_get_typename_externalptr( SEXP p );
    SEXP make_char_single_sexp( const char* str );

    /**
    * \brief	Gets a SEXP, bypassing the custom data converters e.g. offered by RDotNetDataConverter. Solves issue rClr#33
    *
    * \return	a SEXP representing the object handled by the CLR conversion facade, if any.
     */
    SEXP r_get_object_direct();
    SEXP clr_object_to_SEXP( CLR_OBJ* o );
    void get_FullTypeName( SEXP p, char** tname );
    void rclr_load_assembly( char** filename );
    void rclr_create_domain( char** appBaseDir, char** filename, int* mono_debug );
    int is_microsoft_clr();
    void clr_object_finalizer( SEXP ref );
    void use_rdotnet_for_conversions(int use_it);

    // These remaining are obsolete or not yet implemented.
    //SEXP r_reflect_on_object( SEXP clrobj );
    //void get_ns_and_type( SEXP p, char** name_space, char** type_short_name );
    //void rclr_cleanup();

#ifdef __cplusplus
} // end of extern "C" block
#endif

static int uses_rdotnet_for_conversion = 0;

CLR_OBJ* rclr_convert_element( SEXP el );
CLR_OBJ* rclr_mono_convert_element_rdotnet( SEXP el );
CLR_OBJ* rclr_create_array_objects( SEXP s );
CLR_OBJ* get_clr_object( SEXP clrObj );
CLR_OBJ* create_clr_complex_direct( Rcomplex* complex, int length );
const char* get_type_full_name( CLR_OBJ* objptr );

CLR_OBJ* rclr_convert_element_rdotnet( SEXP el );
CLR_OBJ* rclr_convert_element( SEXP el );
CLR_OBJ* rclr_create_array_objects( SEXP s );
SEXP clr_object_to_SEXP( CLR_OBJ* objptr );

RCLR_BOOL check_POSIXct_has_simple_timezone( SEXP p, const char* tzchar );
RCLR_BOOL r_has_class( SEXP s, const char* classname );
RCLR_BOOL r_is_date( SEXP s );
RCLR_BOOL r_is_POSIXlt( SEXP s );
RCLR_BOOL r_is_POSIXct( SEXP s );

SEXP make_numeric_sexp( int n, double* values );
SEXP make_POSIXct_sexp( int n, double* values );
SEXP make_int_sexp( int n, int* values );
SEXP make_char_sexp( int n, char** values );
SEXP make_uchar_sexp( int n, unsigned char* values );

#ifdef MONO_CLR
<<<<<<< HEAD
SEXP make_bool_sexp( int n, MonoBoolean* values );
=======
#define STR_DUP strdup
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

// A vector to store transient CLR object handles that we need to clear on leaving the native interop layer.
std::vector<CLR_OBJ*> transientArgs;

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

>>>>>>> 7c74638017417478fa3772052b4eff6df5c42a15
#elif MS_CLR
SEXP make_bool_sexp( int n, bool* values );
#endif

// Obsolete or not yet implemented:
//CLR_OBJ* rclr_wrap_data_frame( SEXP s );
