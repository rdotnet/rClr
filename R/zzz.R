# An internal variable that is set ot the name of the native library depending on its use of the Mono or MS.NET CLR
nativePkgName <- ''

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
#' @param libname the path to the library from which the package is loaded
#' @param pkgname the name of the package.
#' @rdname dotOnLoad
#' @name dotOnLoad
.onLoad <- function(libname='~/R', pkgname='rClr') {
  rclr_env=Sys.getenv('RCLR')
  monoexepath <- Sys.which('mono')
  ext <- .Platform$dynlib.ext
  nativeLibsNames <- paste(c('rClrMono', 'rClrMs'), ext, sep='')
  monoDll <- nativeLibsNames[1]
  msDll <- nativeLibsNames[2]
  getFnameNoExt <- function(x) {strsplit(x, '\\.')[[1]][1]}
  rClrPkgDir <- file.path(libname, pkgname)
  archLibPath <- file.path(rClrPkgDir, 'libs', Sys.getenv('R_ARCH'))
  srcPkgLibPath <- NULL
  if(!file.exists(archLibPath)) {
    # It may be because this is loaded through the 'document' and 'load_all' functions from devtools,
    # in which case libname is something like "f:/codeplex"
    # try to cater for load_all behavior.
    if( 'rclr' %in% tolower(list.files(libname))) {
      libname <- file.path(rClrPkgDir, 'inst')
      archLibPath <- file.path(rClrPkgDir, 'inst/libs', Sys.getenv('R_ARCH'))
      srcPkgLibPath <- archLibPath
      if(!file.exists(archLibPath)) {stop(paste('Looked like rClr source code directory, but directory not found:', archLibPath))}
    } else {
      stop(paste("Trying to work around devtools, but could not find a folder with lowercase name 'rclr' under ", archLibPath))
    }
  }
  dlls <- list.files(archLibPath, pattern=ext)
  if ( Sys.info()[['sysname']] == 'Windows') {
    if ( rclr_env!='Mono') {
      msvcrFileName <- 'msvcp140.dll'
      if( Sys.which(msvcrFileName) == '') {
        stop(paste(msvcrFileName, "was not found on this Windows system.",
          "You are probably missing the Visual C++ Redistributable for Visual Studio 2019.",
          "Go to https://visualstudio.microsoft.com/downloads/ and dowload 'Microsoft Visual C++ Redistributable for Visual Studio 2019'",
          sep="\n"))
      }
      if(!(msDll %in% dlls)) {
        stop(paste('rClr library .NET framework not found - looked under', archLibPath, 'but not found in', paste(dlls, collapse=',')))
      }
      appendStartupMsg('Loading the dynamic library for Microsoft .NET runtime...')
      chname <- getFnameNoExt(msDll)
      print("before loadAndInit")
      loadAndInit(chname, pkgname, libname, srcPkgLibPath)
    }
  } else { # not on Windows.
    appendStartupMsg('Loading the dynamic library for Mono runtime...')
    chname <- "rClr"
    loadAndInit(chname, pkgname, libname, srcPkgLibPath)
  }
}

loadAndInit <- function(chname, pkgname, libname, srcPkgLibPath=NULL) {
  assign("nativePkgName", chname, inherits=TRUE)
  # cater for devtools 'load_all'; library.dynam fails otherwise.
  if(!is.null(srcPkgLibPath)) {
    ext <- .Platform$dynlib.ext
    # srcPkgLibPath ends with platform separator (e.g. '/')
    f <- file.path(srcPkgLibPath, paste0(chname, ext))
    dyn.load(f)
  } else {
    library.dynam(chname, pkgname, libname)
  }
  # should the init of the mono runtime try to attach to a Monodevelop debugger?
  debug_flag=Sys.getenv('RCLR_DEBUG')
  clrInit(debug_flag!="")
  print("after clrInit")
  appendStartupMsg(paste('Loaded Common Language Runtime version', getClrVersionString()))
  print("before setRDotNet")
  setRDotNet(TRUE)
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
  print("before getClrVersionString")
  v <- clrGet('System.Environment', 'Version')
  print(v)
  print("we got v")
  clrCall(v, 'ToString')
}

#' rClr .onAttach
#'
#' Print startup messages from package onLoad
#'
#' Print startup messages from package onLoad (prevents a 'NOTE' on package check)
#'
#' @rdname dotOnAttach
#' @name dotOnAttach
#' @param libname the path to the library from which the package is loaded
#' @param pkgname the name of the package.
.onAttach <- function(libname='~/R', pkgname='rClr') {
  if(startupMsg!='') {
    packageStartupMessage(startupMsg)
  }
}
