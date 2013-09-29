#' Full type name of the reflection helper the interop code written in C#
reflectionHelperTypeName <- 'Rclr.ReflectionHelper'

#' Full type name of the main facade to the interop code written in C#
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
#' @param extPtr the external pointer.
#' @return a cobjRef S4 object if the argument is indeed an external pointer, 
#' otherwise returned unchanged if null of not an external pointer.
#' @import methods
createReturnedObject <- function(extPtr) {
  if( is.null(extPtr) == TRUE ) {
    return(NULL)
  } else if ("externalptr" %in% class(extPtr)) {
    typename <- clrTypeNameExtPtr(extPtr)
		return(new("cobjRef", clrobj=extPtr, clrtype=typename))
  } else {
    return(extPtr)
  }
}

