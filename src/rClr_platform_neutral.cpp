/////////////////////////////////////////
// This file  contains the implementation of functions 
// that do not contain any conditional compilation
/////////////////////////////////////////

#include "rClr.h"

/////////////////////////////////////////
// Functions with R specific constructs, namely SEXPs
/////////////////////////////////////////


SEXP make_int_sexp( int n, int* values ) {
    SEXP result;
    long i = 0;
    int* int_ptr;
    PROTECT( result = NEW_INTEGER( n ) );
    int_ptr = INTEGER_POINTER( result );
    for (i = 0; i < n; i++) {
        int_ptr[i] = values[i];
    }
    UNPROTECT( 1 );
    return result;
}

SEXP make_numeric_sexp( int n, double* values ) {
    SEXP result;
    long i = 0;
    double* dbl_ptr;
    PROTECT( result = NEW_NUMERIC( n ) );
    dbl_ptr = NUMERIC_POINTER( result );
    for (i = 0; i < n; i++) {
        dbl_ptr[i] = values[i];
    }
    UNPROTECT( 1 );
    return result;
}

SEXP make_char_sexp( int n, char** values ) {
    SEXP result;
    long i = 0;
    PROTECT( result = NEW_CHARACTER( n ) );
    for (i = 0; i < n; i++) {
        SET_STRING_ELT( result, i, mkChar( (const char*)values[i] ) );
    }
    UNPROTECT( 1 );
    return result;
}

SEXP make_char_sexp_one( char* values ) {
    return make_char_sexp( 1, &values );
}

SEXP make_class_from_numeric( int n, double* values, int numClasses, char** classnames ) {
    SEXP result;
    result = make_numeric_sexp( n, values );
    PROTECT( result );
    setAttrib( result, R_ClassSymbol, make_char_sexp( numClasses, classnames ) );
    UNPROTECT( 1 );
    return result;
}


// FIXME probably DEPRECATED
SEXP make_date_sexp( int n, double* values ) {
    char* classname = "Date";
    return make_class_from_numeric( n, values, 1, &classname );
}

SEXP make_POSIXct_sexp( int n, double* values ) {
    SEXP result;
    char** classname = (char**)malloc( sizeof( char* ) * 2 );
    classname[0] = "POSIXct"; classname[1] = "POSIXt";
    result = make_class_from_numeric( n, values, 2, classname );
    if (NAMED( result ) == 2)
        PROTECT( result = duplicate( result ) ); // TOCHECK: why??
    else
        PROTECT( result );
    // FOR INFO: following creates an access violation
    //setAttrib(result, mkChar("tzone"), mkChar("UTC")); 
    setAttrib( result, make_char_sexp_one( "tzone" ), make_char_sexp_one( "UTC" ) );
    UNPROTECT( 1 );
    free( classname );
    return result;
}

SEXP make_uchar_sexp( int n, unsigned char* values ) {
    SEXP result;
    long i = 0;
    Rbyte* bPtr;
    PROTECT( result = NEW_RAW( n ) );
    bPtr = RAW_POINTER( result );
    for (i = 0; i < n; i++) {
        bPtr[i] = values[i];
    }
    UNPROTECT( 1 );
    return result;
}

SEXP make_char_single_sexp( const char* str ) {
    return mkString( str );
}

SEXP r_get_null_reference() {
    return R_MakeExternalPtr( 0, R_NilValue, R_NilValue );
}

SEXP r_show_args( SEXP args )
{
    const char* name;
    int i, j, nclass;
    SEXP el, names, klass;
    int nargs, nnames;
    args = CDR( args ); /* skip 'name' */

    nargs = Rf_length( args );
    for (i = 0; i < nargs; i++, args = CDR( args )) {
        name =
            isNull( TAG( args ) ) ? "<unnamed>" : CHAR( PRINTNAME( TAG( args ) ) );
        el = CAR( args );
        Rprintf( "[%d] '%s' R type %s, SEXPTYPE=%d\n", i + 1, name, type2char( TYPEOF( el ) ), TYPEOF( el ) );
        Rprintf( "[%d] '%s' length %d\n", i + 1, name, LENGTH( el ) );
        names = getAttrib( el, R_NamesSymbol );
        nnames = Rf_length( names );
        Rprintf( "[%d] names of length %d\n", i + 1, nnames );
        //for(j = 0; j < nnames; j++, names = CDR(names))
        //{
        //	name=CAR(names);
        //Rprintf("[%d] %s\n", i+1, CHAR(STRING_ELT(el, 0)), );
        //}
        klass = getAttrib( el, R_ClassSymbol );
        nclass = length( klass );
        for (j = 0; j < nclass; j++) {
            Rprintf( "[%d] class '%s'\n", i + 1, CHAR( STRING_ELT( klass, j ) ) );
        }
    }
    return(R_NilValue);
}

SEXP r_get_sexp_type( SEXP par ) {
    SEXP p = par, e;
    int typecode;
    p = CDR( p ); e = CAR( p );
    typecode = TYPEOF( e );
    return make_int_sexp( 1, &typecode );
}

void get_FullTypeName( SEXP p, char** tname ) {
    SEXP e;
    e = CAR( p ); /* second is the namespace */
    if (TYPEOF( e ) != STRSXP || LENGTH( e ) != 1)
        error( "get_FullTypeName: cannot parse type name: need a STRSXP of length 1" );
    (*tname) = STR_DUP( CHAR( STRING_ELT( e, 0 ) ) ); // is all this really necessary? I recall trouble if using less, but this still looks over the top.
}

RCLR_BOOL check_POSIXct_has_simple_timezone( SEXP p, const char* tzchar ) {
    SEXP t; const char* c;
    SEXP tzone = getAttrib( p, mkChar( "tzone" ) );
    if (length( tzone ) > 0) {
        t = tzone;
        if (length( tzone ) > 1) {
            // I actually did see a POSIXt with 3 elements c("","EST","EST") after class conversions. Not sure whether makes sense...
            error( "Time zone on date-time object has more than one elements!" );
            return FALSE_BOOL;
        }
        else {
            c = CHAR( STRING_ELT( t, 0 ) );
            //if ( (strcmp(c, "") == 0) || (strcmp(c, "GMT") == 0) || ( strcmp(c, "UTC") == 0) ) { // TOCHECK: is it case sensitive in R??
            if ((strcmp( c, "" ) == 0) || (strcmp( c, "GMT" ) == 0) || (strcmp( c, "UTC" ) == 0)) {
                tzchar = c;
            }
            else {
                tzchar = "";
                error( "Sorry, only UTC and GMT POSIXt is supported, not '%s'", c );
                return FALSE_BOOL;
            }
        }
    }
    return TRUE_BOOL;
}

RCLR_BOOL r_has_class( SEXP s, const char* classname ) {
    SEXP klasses;
    R_xlen_t j;
    int n_classes;
    klasses = getAttrib( s, R_ClassSymbol );
    n_classes = length( klasses );
    if (n_classes > 0) {
        for (j = 0; j < n_classes; j++)
            if (strcmp( CHAR( STRING_ELT( klasses, j ) ), classname ) == 0)
                return TRUE_BOOL;
    }
    return FALSE_BOOL;
}

RCLR_BOOL r_is_date( SEXP s ) {
    return r_has_class( s, "Date" );
}

RCLR_BOOL r_is_POSIXlt( SEXP s ) {
    return r_has_class( s, "POSIXlt" );
}

RCLR_BOOL r_is_POSIXct( SEXP s ) {
    return r_has_class( s, "POSIXct" );
}

CLR_OBJ* create_clr_complex_direct( Rcomplex* complex, int length )
{
    return NULL;
}

CLR_OBJ* get_clr_object( SEXP clrObj ) {
    SEXP a, clrobjSlotName;
    SEXP s4classname = getAttrib( clrObj, R_ClassSymbol );
    if (strcmp( CHAR( STRING_ELT( s4classname, 0 ) ), "cobjRef" ) == 0)
    {
        PROTECT( clrobjSlotName = NEW_CHARACTER( 1 ) );
        SET_STRING_ELT( clrobjSlotName, 0, mkChar( "clrobj" ) );
        a = getAttrib( clrObj, clrobjSlotName );
        UNPROTECT( 1 );
        if (a != NULL && a != R_NilValue && TYPEOF( a ) == EXTPTRSXP) {
            return GET_CLR_OBJ_FROM_EXTPTR( a );
        }
        else
            return NULL;
    }
    else
    {
        error( "Incorrect type of S4 Object: Not of type 'cobjRef'" );
        return NULL;
    }
}

