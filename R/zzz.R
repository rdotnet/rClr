# An internal variable that is set ot the name of the native library depending on its use of the Mono or MS.NET CLR 
rclr_pkg_name <- ''

# An internal variable to buffer startup messages
startupMsg <- ''

#' rClr .onLoad
#' 
#' Function called when loading the rClr package with 'library'. 
#' 
#' The function looks by default for the rClr native library for the Mono runtime. 
#' If the platform is Linux, this is the only option. If the platform is Windows, using the 
#' Microsoft .NET runtime is an option. If the rClr native library for MS.NET is detected, 
#' the Microsoft .NET runtime is loaded in preference to Mono.
#' 
#' @rdname dotOnLoad
#' @param libname the path to the library from which the package is loaded
#' @param pkgname the name of the package.
.onLoad <- function(libname='~/R', pkgname='rClr') {
  rclr_env=Sys.getenv('RCLR')
  monoexepath <- Sys.which('mono')
  ext <- .Platform$dynlib.ext
  nativeLibsNames <- paste(c('rClrMono', 'rClrMs'), ext, sep='')
  msDll <- nativeLibsNames[2]
  monoDll <- nativeLibsNames[1]
  getFnameNoExt <- function(x) {strsplit(x, '\\.')[[1]][1]}
  rClrPkgDir <- file.path(libname, pkgname)
  archLibPath <- file.path(rClrPkgDir, 'libs', Sys.getenv('R_ARCH'))
  srcPkgLibPath <- NULL
  if(!file.exists(archLibPath)) {
    # It may be because this is loaded through the 'document' and 'load_all' functions from devtools, 
    # in which case libname is something like "f:/codeplex/r2clr/packages"
    # try to cater for load_all behavior.
    if(grep('r2clr/packages$', libname) == 1) {
      libname <- file.path(rClrPkgDir, 'inst')
      archLibPath <- file.path(rClrPkgDir, 'inst/libs', Sys.getenv('R_ARCH'))
      srcPkgLibPath <- archLibPath
      if(!file.exists(archLibPath)) {stop(paste('Looked like rClr source code directory, but directory not found:', archLibPath))}
    } else {
      stop(paste('Directory not found:', archLibPath))
    }
  }
  dlls <- list.files(archLibPath, pattern=ext)
  if ( Sys.info()[['sysname']] == 'Windows') {
    if ( msDll %in% dlls && rclr_env!='Mono') {
      if( Sys.which('msvcr110.dll') == '') {
        stop(paste("'msvcr110.dll' was not found on this Windows system.",
          "You are probably missing the Visual C++ Redistributable for Visual Studio 2012.",
          "Check instructions at https://r2clr.codeplex.com/wikipage?title=Installing%20R%20packages&referringTitle=Documentation", 
          sep="\n"))
      }
      appendStartupMsg('Loading the dynamic library for Microsoft .NET runtime...')
      chname <- getFnameNoExt(msDll) 
      loadAndInit(chname, pkgname, libname, srcPkgLibPath) 
    } else {
      appendStartupMsg('Loading the dynamic library for Mono runtime...')
      if(!(monoDll %in% dlls)) {
        stop(paste('rClr library for Mono not found - looked under', archLibPath)) 
      } else if (monoexepath=='') {
        stop("mono.exe was not found by 'Sys.which'. You must add the mono bin directory (e.g. c:\\Program Files\\Mono-3.0.6\\bin) to your PATH environment variable")
      } else {
        chname <- getFnameNoExt(monoDll)
        loadAndInit(chname, pkgname, libname, srcPkgLibPath) 
      }
    }
  } else {
    appendStartupMsg('Loading the dynamic library for Microsoft .NET runtime...')
    chname <- getFnameNoExt(monoDll) 
    loadAndInit(chname, pkgname, libname, srcPkgLibPath)
  }
}

loadAndInit <- function(chname, pkgname, libname, srcPkgLibPath=NULL) {
  assign("rclr_pkg_name", chname, inherits=TRUE) 
  # cater for devtools 'load_all'; library.dynam fails otherwise. 
  if(!is.null(srcPkgLibPath)) {
    ext <- .Platform$dynlib.ext
    f <- paste(file.path(srcPkgLibPath, chname), ext, sep='')
    dyn.load(f)
  } else {
    library.dynam(chname, pkgname, libname)
  }
  # should the init of the mono runtime try to attach to a Monodevelop debugger?
  debug_flag=Sys.getenv('RCLR_DEBUG')
  clrInit(debug_flag!="")
  appendStartupMsg(paste('Loaded Common Language Runtime version', getClrVersionString()))
}

appendStartupMsg <- function(msg) {
  startupMsg <<- paste0(startupMsg, msg, '\n')
}

#' Gets the version of the common language runtime in use
#'
#' Gets the version of the common language runtime in use. 
#'
#' @return the version of the common language runtime in use
#' @export
getClrVersionString <- function() {
  v <- clrGet('System.Environment', 'Version')
  clrCall(v, 'ToString')
}

#' rClr .onAttach
#' 
#' Print startup messages from package onLoad 
#' 
#' Print startup messages from package onLoad (prevents a 'NOTE' on package check)
#' 
#' @rdname dotOnAttach
#' @param libname the path to the library from which the package is loaded
#' @param pkgname the name of the package.
.onAttach <- function(libname='~/R', pkgname='rClr') {
  if(startupMsg!='') {
    packageStartupMessage(startupMsg)
  }
}
