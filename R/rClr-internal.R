# Full type name of the reflection helper the interop code written in C#
reflectionHelperTypeName <- 'Rclr.ReflectionHelper'

# Full type name of the main facade to the interop code written in C#
clrFacadeTypeName <- 'Rclr.ClrFacade'

#' Initialize a new CLR application domain
#'
#' Initialize a new CLR application domain
#'
#' @param debug If using Mono, should the CLR be initialised to try to hook up to the mono soft debugger in MonoDevelop. This parameter has no effect if using MS.NET.
#' @return nothing is returned by this function
#' @examples
#' \dontrun{
#' library(rClr)
#' }
clrInit <- function(debug=FALSE) {
  pkgLibsDir <- getLibsPath('rClr')
  f <- file.path(pkgLibsDir, 'ClrFacade.dll')
  f <- path.expand(f)
  stopifnot( file.exists(f) )
  result <- .C("rclr_create_domain", pkgLibsDir, f, as.integer(debug), PACKAGE=nativePkgName)
}

checkIsExtPtr <- function(extPtr) {
  stopifnot("externalptr" %in% class(extPtr))
}

getLibsPath <- function(pkgName) {
  libLocation<-system.file(package=pkgName)
  file.path(libLocation, 'libs')
}

#' Create if possible an S4 CLR object.
#'
#' Create if possible and adequate the S4 object that wraps the external pointer to a CLR object.
#' Currently not exported, as this is unlikely to be recommended for use outside of unit tests and internal to rClr.
#'
#' @param obj the presumed external pointer.
#' @param clrtype character; the name of the CLR type for the object. If NULL, rClr retrieves the type name.
#' @return a cobjRef S4 object if the argument is indeed an external pointer,
#' otherwise returned unchanged if null or not an external pointer.
#' @import methods
mkClrObjRef <- function(obj, clrtype=NULL) {
  if(is(obj, 'cobjRef')) return(obj)
  if( is.null(obj) == TRUE ) {
    return(NULL)
  } else if ("externalptr" %in% class(obj)) {
    if(is.null(clrtype)) { clrtype <- clrTypeNameExtPtr(obj) }
		return(new("cobjRef", clrobj=obj, clrtype=clrtype))
  } else {
    return(obj)
  }
}


