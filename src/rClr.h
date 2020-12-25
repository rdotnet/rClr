#pragma once

// #define RCLR_VER 0x000100 /* RCLR v0.1-0 */

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
SEXP make_bool_sexp( int n, MonoBoolean* values );
#elif MS_CLR
SEXP make_bool_sexp( int n, bool* values );
#endif

// Obsolete or not yet implemented:
//CLR_OBJ* rclr_wrap_data_frame( SEXP s );
