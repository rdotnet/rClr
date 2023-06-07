#' Shuts down the current runtime.
#'
#' Shuts down the current runtime.
#'
#' @return nothing is returned by this function
#' @export
clrShutdown <- function() { # TODO: is this even possible given runtime's constraints?
  result <- .C("rclr_shutdown_clr", PACKAGE=nativePkgName)
}

#' Turn on/off R.NET
#'
#' Turn on or off the usage of the R.NET assemblies to convert CLR objects to R data structures. As of version 0.7.0, R.NET is the preferred way to convert data and is enabled by default.
#'
#' @param setit if true enable, otherwise disable
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' ## R.NET is currently used to convert complicated CLR types to sensible R equivalents
#' setRDotNet()
#' cTypename <- "Rclr.TestCases"
#' clrCallStatic(cTypename, "CreateStringDictionary")
#' setRDotNet(FALSE)
#' clrCallStatic(cTypename, "CreateStringDictionary")
#' }
setRDotNet <- function(setit=TRUE) {
  print("during setRDotNet")
  invisible(clrCallStatic('Rclr.RDotNetDataConverter', 'SetRDotNet', setit))
}

#' Turn on/off the conversion of advanced data types with R.NET
#'
#' Turn on/off the conversion of advanced data types with R.NET. This will turn off the conversion of classes such as dictionaries into R lists,
#' as these are not bidirectional and you may want to see and manipulate external pointers to dictionaries in some circumstances.
#'
#' @param enable if true enable, otherwise disable
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' cTypename <- "Rclr.TestCases"
#' clrCallStatic(cTypename, "CreateStringDictionary")
#' setConvertAdvancedTypes(FALSE)
#' clrCallStatic(cTypename, "CreateStringDictionary")
#' }
setConvertAdvancedTypes <- function(enable=TRUE) {
  invisible(clrCallStatic('Rclr.RDotNetDataConverter', 'SetConvertAdvancedTypes', enable))
}

#' Loads a Common Language assembly.
#'
#' Loads an assembly. Note that this is loaded in the single application domain that is created by rClr, not a separate application domain.
#'
#' @param name a character vector of length one. It can be the full file name of the assembly to load, or a fully qualified assembly name, or as a last resort a partial name.
#' @seealso \code{\link{.C}} which this function wraps
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' clrGetLoadedAssemblies()
#' f <- file.path('SomeDirectory', 'YourDotNetBinaryFile.dll')
#' f <- path.expand(f)
#' stopifnot( file.exists(f) )
#' clrLoadAssembly(f)
#' # Load an assembly from the global assembly cache (GAC)
#' clrLoadAssembly("System.Windows.Presentation,
#'   Version=3.5.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")
#' # The use of partial assembly names is discouraged; nevertheless it is supported
#' clrLoadAssembly("System.Web.Services")
#' clrGetLoadedAssemblies()
#' }
clrLoadAssembly <- function(name) {
  # if( !file.exists(name) ) stop(paste("File not found: ", name))
  result <- .C("rclr_load_assembly", name, PACKAGE=nativePkgName)
}

#' Gets the inner name used for the package
#'
#' Gets the inner name used for the package (rClrMono or rClrMs). This is not intented for use by most users.
#'
#' @return the short name of the library currently loaded, depending on the runtime used (Mono or Microsoft .NET)
#' @export
clrGetInnerPkgName <- function() { nativePkgName }

#' List the instance members of a CLR object
#'
#' List the instance members of a CLR object, i.e. its methods, fields and properties.
#'
#' @param clrobj CLR object
#' @return a list of methods, fields and properties of the CLR object
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrReflect(testObj)
#' }
clrReflect <- function( clrobj ) {
  # .Call("r_reflect_on_object", clrobj@clrobj, silent=FALSE, PACKAGE="rClr")
  list(Methods=clrGetMethods(clrobj), Fields=clrGetFields(clrobj), Properties=clrGetProperties(clrobj))
}

#' Calls the ToString method of an object
#'
#' Calls the ToString method of an object as represented in the CLR.
#' This function is here to help quickly test object equivalence from the R interpreter, for instance on the tricky topic of date-time conversions
#'
#' @param x any R object, which is converted to a CLR object on which to call ToString
#' @return the string representation of the object in the CLR
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' dt <- as.POSIXct('2001-01-01 02:03:04', tz='UTC')
#' clrToString(dt)
#' }
clrToString <- function(x) {
  return(clrCallStatic(clrFacadeTypeName, 'ToString',x))
}


#' Prints the last CLR exception
#'
#' Prints the last CLR exception. This is roughly the equivalent of the traceback function of R.
#'
#' @export
#' @examples
#' \dontrun{
#' clrCallStatic("Rclr.TestCases", "ThrowException", 10L) # will be truncated by the Rf_error API
#' clrTraceback() # prints the full stack trace
#' }
clrTraceback <- function() {
  cat(clrGet(clrFacadeTypeName, 'LastException'))
  invisible(NULL)
}

#' List the names of loaded CLR assemblies
#'
#' List the names of loaded CLR assemblies
#'
#' @param fullname should the full name of the assemblies be returned
#' @param filenames if TRUE, return a data frame where the second column is the URI (usually file path) of the loaded assembly.
#' @return the names of loaded CLR assemblies
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' clrGetLoadedAssemblies()
#' }
clrGetLoadedAssemblies <- function(fullname=FALSE, filenames=FALSE) {
  assNames <- clrCallStatic(reflectionHelperTypeName, 'GetLoadedAssemblyNames', fullname)
  if(filenames) {
    data.frame(AssemblyName=assNames, URI=clrCallStatic(reflectionHelperTypeName, 'GetLoadedAssemblyURI', assNames))
  } else {
    assNames
  }
}

#' Get a list of CLR type names exported by an assembly
#'
#' Get a list of CLR type names exported by an assembly
#'
#' @param assemblyName the name of the assembly
#' @return the names of the types exported by the assembly
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' clrGetLoadedAssemblies()
#' clrGetTypesInAssembly('ClrFacade')
#' }
clrGetTypesInAssembly <- function(assemblyName) {
  clrCallStatic(reflectionHelperTypeName, 'GetTypesInAssembly', assemblyName)
}

#' List the instance fields of a CLR object
#'
#' List the instance fields of a CLR object
#'
#' @param clrobj CLR object
#' @param contains a string that the field names returned must contain
#' @return a list of names of the fields of the CLR object
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrGetFields(testObj)
#' clrGetFields(testObj, 'ieldInt')
#' }
clrGetFields <- function( clrobj, contains = '') {
  clrCallStatic(reflectionHelperTypeName, 'GetInstanceFields', clrobj, contains)
}

#' List the instance properties of a CLR object
#'
#' List the instance properties of a CLR object
#'
#' @param clrobj CLR object
#' @param contains a string that the property names returned must contain
#' @return a list of names of the properties of the CLR object
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrGetProperties(testObj)
#' clrGetProperties(testObj, 'One')
#' }
clrGetProperties <- function( clrobj, contains = '') {
  clrCallStatic(reflectionHelperTypeName, 'GetInstanceProperties', clrobj, contains)
}

#' List the instance methods of a CLR object
#'
#' List the instance methods of a CLR object
#'
#' @param clrobj CLR object
#' @param contains a string that the methods names returned must contain
#' @return a list of names of the methods of the CLR object
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrGetMethods(testObj)
#' clrGetMethods(testObj, 'Get')
#' }
clrGetMethods <- function( clrobj, contains = '') {
  clrCallStatic(reflectionHelperTypeName, 'GetInstanceMethods', clrobj, contains)
}

#' List the public constructors of a CLR Type
#'
#' List the public constructors of a CLR Type
#'
#' @param type CLR Type, or a (character) type name that can be successfully parsed
#' @return a list of constructor signatures
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' clrGetConstructors(testClassName)
#' }
clrGetConstructors <- function( type ) {
  clrCallStatic(reflectionHelperTypeName, 'GetConstructors', type)
}

#' Gets the signature of a CLI object member
#'
#' Gets a string representation of the signature of a member (i.e. field, property, method).
#' Mostly used to interactively search for what arguments to pass to a method.
#'
#' @param clrobj CLR object
#' @param memberName The exact name of the member (i.e. field, property, method) to search for
#' @return a character vector with summary information on the method/member signatures
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrReflect(testObj)
#' clrGetMemberSignature(testObj, 'set_PropertyIntegerOne')
#' clrGetMemberSignature(testObj, 'FieldIntegerOne')
#' clrGetMemberSignature(testObj, 'PropertyIntegerTwo')
#' }
clrGetMemberSignature <- function( clrobj, memberName ) {
  clrCallStatic(reflectionHelperTypeName, 'GetSignature', clrobj, memberName)
}

#' Create a new CLR object
#'
#' @param typename type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param ... additional method arguments passed to the object constructor via the call to .External
#' @return a CLR object
#' @export
#' @import methods
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' (testObj <- clrNew(testClassName))
#' # object with a constructor that has parameters
#' (testObj <- clrNew(testClassName, as.integer(123)))
#' clrLoadAssembly("System.Windows.Forms, Version=2.0.0.0,
#'   Culture=neutral, PublicKeyToken=b77a5c561934e089")
#' f <- clrNew('System.Windows.Forms.Form')
#' clrSet(f, 'Text', "Hello from '.NET'")
#' clrCall(f, 'Show')
#' }
clrNew <- function(typename, ...)
{
  o<-.External("r_create_clr_object", typename, ..., PACKAGE=nativePkgName)
  if (is.null(o)) {
    stop("Failed to create instance of type '",typename,"'")
  }
  mkClrObjRef(o, clrtype=typename)
}

#' System function to get a direct access to an object
#'
#' This function needs to be exported, but is highly unlikely to be of any use to an end user, even an advanced one.
#' This is indirectly needed to unlock the benefits of using R.NET convert data structures between R and .NET.
#' Using this function is a critical part of solving the rather complicated issue rClr #33.
#'
#' @return a CLR object
#' @export
getCurrentConvertedObject <- function()
{
  o <-.External("r_get_object_direct", PACKAGE=nativePkgName)
  mkClrObjRef(o)
}

#' Check whether an object is of a certain type
#'
#' Check whether an object is of a certain type. This function is meant to match the behavior of the 'is' keyword in C#.
#'
#' @param obj an object
#' @param type the object type to check for. It can be a character, of a object of CLR type System.RuntimeType
#' @return TRUE or FALSE
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' (testObj <- clrNew(testClassName))
#' clrIs(testObj, testClassName)
#' clrIs(testObj, 'System.Object')
#' clrIs(testObj, 'System.Double')
#' (testObj <- clrNew('Rclr.TestMethodBinding'))
#' # Test for interface definitions
#' clrIs(testObj, 'Rclr.ITestMethodBindings')
#' clrIs(testObj, clrGetType('Rclr.ITestMethodBindings'))
#' clrIs(testObj, clrGetType('Rclr.TestMethodBinding'))
#' clrIs(testObj, clrGetType('System.Reflection.Assembly'))
#' }
clrIs <- function(obj, type) {
  if(is.character(type)) {
    tmpType <- clrGetType(type)
    if(is.null(tmpType)) {stop(paste('Unrecognized type name', type))} else {type <- tmpType}
  }
  if(!is(type, 'cobjRef')) {
    stop(paste('argument "type" must be a CLR type name or a Type'))
  } else {
    typetypename <- clrGet(clrCall(type, 'GetType'), 'Name')
    if(!(typetypename %in% c('RuntimeType', 'MonoType'))) {
      stop(paste('argument "type" must be a CLR Type. Got a', typetypename))
    }
  }
  objType <- clrGetType(obj)
  return(clrCall(type, 'IsAssignableFrom', objType))
}

#' Call a method on an object
#'
#' @param obj an object
#' @param methodName the name of a method of the object
#' @param ... additional method arguments passed to .External
#' @return an object resulting from the call. May be a CLR object, or a native R object for common types. Can be NULL.
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' (testObj <- clrNew(testClassName))
#' clrCall(testObj, 'GetFieldIntegerOne')
#' ## derived from unit test for matching the right method (function) to call.
#' f <- function(...){ paste( 'This called a method with arguments:',
#'   paste(clrCallStatic('Rclr.TestMethodBinding', 'SomeStaticMethod', ...), collapse=', ')) }
#' f(1:3)
#' f(3)
#' f('a')
#' f('a', 3)
#' f(3, 'a')
#' f(list('a', 3))
#' }
clrCall <- function(obj,methodName,...)
{
  interface="r_call_method"
  result <- NULL
  result <-.External(interface, obj@clrobj, methodName, ..., PACKAGE=nativePkgName)
  return(mkClrObjRef(result))
}

#' Gets the value of a field or property of an object or class
#'
#' @param objOrType a CLR object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param name the name of a field/property  of the object
#' @return an object resulting from the call. May be a CLR object, or a native R object for common types. Can be NULL.
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrReflect(testObj)
#' clrGet(testObj, 'FieldIntegerOne')
#' clrGet(testClassName, 'StaticPropertyIntegerOne')
#' }
clrGet <- function(objOrType,name)
{
  print("HERE IT FAILS")
  return(clrCallStatic(clrFacadeTypeName, 'GetFieldOrProperty',objOrType, name))
}

#' Sets the value of a field or property of an object or class
#'
#' Sets the value of a field or property of an object or class
#'
#' @param objOrType a CLR object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param name the name of a field/property of the object
#' @param value the value to set the field with
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrReflect(testObj)
#' clrSet(testObj, 'FieldIntegerOne', 42)
#' clrSet(testClassName, 'StaticPropertyIntegerOne', 42)
#'
#' # Using 'good old' Windows forms to say hello:
#' clrLoadAssembly("System.Windows.Forms, Version=2.0.0.0,
#'   Culture=neutral, PublicKeyToken=b77a5c561934e089")
#' f <- clrNew('System.Windows.Forms.Form')
#' clrSet(f, 'Text', "Hello from '.NET'")
#' clrCall(f, 'Show')
#' }
clrSet <- function(objOrType, name, value)
{
  invisible(clrCallStatic(clrFacadeTypeName, 'SetFieldOrProperty',objOrType, name, value))
}

#' Gets the names of a CLR Enum value type
#'
#' @param enumType a CLR object, System.Type or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @return a character vector of the names for the enum
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' clrGetEnumNames('Rclr.TestEnum')
#' }
clrGetEnumNames <- function(enumType)
{
  return(clrCallStatic(reflectionHelperTypeName, 'GetEnumNames',enumType))
}

#' Sets the value of an enum field or property of an object or class
#'
#' Sets the value of an enum field or property of an object or class
#'
#' @param objOrType a CLR object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param name the name of a field/property of the object
#' @param enumtype type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param enumval the value to set the field with
#' @export
clrSetEnumProperty <- function(objOrType, name, enumtype, enumval)
{
  stop('Not yet implemented')
  return(clrCallStatic(reflectionHelperTypeName, 'SetEnumValue',objOrType, name, enumtype, enumval))
}

#' Gets the external pointer CLR object.
#'
#' Gets the external pointer CLR object. End user are unlikely to need this.
#'
#' @param clrObject a S4 object of class clrobj
#' @return the external pointer to the CLR object
#' @export
clrGetExtPtr <- function(clrObject) {
  clrObject@clrobj
}

#' Gets the type name of an object
#'
#' Gets the type name of an object, given the SEXP external pointer to this CLR object.
#'
#' @param extPtr external pointer to a CLR object (not a cobjRef S4 object)
#' @return a character string, the type name
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrTypeNameExtPtr(clrGetExtPtr(testObj))
#' }
clrTypeNameExtPtr <- function(extPtr) {
  checkIsExtPtr(extPtr)
  .External("r_get_typename_externalptr", extPtr, PACKAGE=nativePkgName)
}


#' Gets the type name of an object
#'
#' Gets the CLR type name of an object, given an S4 clrobj object
#'
#' @param clrobj CLR object
#' @return type name
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrTypename(testObj)
#' }
clrTypename <- function(clrobj) {
  name <- .Call("r_get_type_name", clrobj, PACKAGE=nativePkgName)
  name
}


#' Gets the name of the native library currently loaded.
#'
#' Gets the name of the native library currently loaded. Used only for unit tests.
#'
#' @return the name of the native library currently loaded: rClrMs or rClrMono
#' @export
clrGetNativeLibName <- function() {
  nativePkgName
}

#' Call a static method on a CLR type
#'
#' @param typename type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param methodName the name of a static method of the type
#' @param ... additional method arguments passed to .External
#' @return an object resulting from the call. May be a CLR object, or a native R object for common types. Can be NULL.
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' cTypename <- "Rclr.TestCases"
#' clrCallStatic(cTypename, "IsTrue", TRUE)
#' }
clrCallStatic <- function(typename, methodName,...)
{
  print("trying go get the pointer")
  print(paste0("typename: ", typename))
  print(paste0("methodName: ", methodName))
  print(paste0("PACKAGE: ", nativePkgName))
  print(paste0("additional args: ", list(...)))
  extPtr <-.External("r_call_static_method", typename, methodName,..., PACKAGE=nativePkgName)
  print("pointer is:")
  print(extPtr)
  return(mkClrObjRef(extPtr))
}

#' Peek into the types of CLR objects arguments are converted to by rClr
#'
#' Advanced use only, to diagnose unexpected conditions in CLR method calls. Most users would not ever need it.
#'
#' @param ... method arguments passed to .External
#' @return a character message with type information about each argument.
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' peekClrArgs("a", numeric(0))
#' }
peekClrArgs <- function(...)
{
  extPtr <-.External("r_diagnose_parameters", ..., PACKAGE=nativePkgName)
  return(mkClrObjRef(extPtr))
}

#' Gets the static members for a type
#'
#' Gets the static members for a type
#'
#' @param objOrType a CLR object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @export
#' @examples
#' \dontrun{
#' library(rClr)
#' cTypename <- "Rclr.TestCases"
#' clrGetStaticMembers(cTypename)
#' testClassName <- "Rclr.TestObject";
#' testObj <- clrNew(testClassName)
#' clrGetStaticMembers(testObj)
#' }
clrGetStaticMembers <- function(objOrType)
{
  list(Methods=clrGetStaticMethods(objOrType), Fields=clrGetStaticFields(objOrType), Properties=clrGetStaticProperties(objOrType))
}

#' Gets the static fields for a type
#'
#' Gets the static fields for a type
#'
#' @param objOrType a CLR object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param contains a string that the property names returned must contain
#' @export
clrGetStaticFields <- function( objOrType, contains = '') {
  clrCallStatic(reflectionHelperTypeName, 'GetStaticFields', objOrType, contains)
}

#' Gets the static members for a type
#'
#' Gets the static members for a type
#'
#' @inheritParams clrGetStaticFields
#' @export
clrGetStaticProperties <- function( objOrType, contains = '') {
  clrCallStatic(reflectionHelperTypeName, 'GetStaticProperties', objOrType, contains)
}

#' Gets the static members for a type
#'
#' @inheritParams clrGetStaticFields
#' @export
clrGetStaticMethods <- function( objOrType, contains = '') {
  clrCallStatic(reflectionHelperTypeName, 'GetStaticMethods', objOrType, contains)
}

#' Gets the signature of a static member of a type
#'
#' @param typename type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param memberName The exact name of the member (i.e. field, property, method) to search for
#' @export
clrGetStaticMemberSignature <- function( typename, memberName ) {
  clrCallStatic(reflectionHelperTypeName, 'GetSignature', typename, memberName)
}

#' Architecture dependent path to the rClr native library
#'
#' Guess the directory where to expect the architecture dependent native library of a specified package
#' e.g. for the package rClr, ${R_HOME}/library/rClr/libs/x64
#' This is a utility that is not specific to the rClr package
#'
#' @param pkgName the name of a package, e.g. 'rClr'
#' @return the prospective path in which a native library would be found, e.g. c:/R/library/rClr/libs/x64
#' @export
getNativeLibsPath <- function(pkgName) {
  r_arch=Sys.getenv("R_ARCH")
  arch <- sub( '/', '', r_arch)
  file.path(getLibsPath(pkgName), arch)
  #rlibs <- getNativeLibsPathRlibs(pkgName)
  #rhome <- getNativeLibsPathRhome(pkgName)
  #ifelse(file.exists(rlibs), rlibs, rhome)
}

#' Get the type code for a SEXP
#'
#' Get the type code for a SEXP, as returned by the TYPEOF macro
#'
#' @param sexp an R object
#' @return the type code, an integer, as defined in Rinternals.h
#' @export
getSexpType <- function(sexp) {
  extPtr <-.External("r_get_sexp_type", sexp, PACKAGE=nativePkgName)
  return(mkClrObjRef(extPtr))
}

#' Peek into the structure of R objects 'as seen from C code'
#'
#' Inspect one or more R object to get information on its representation in the engine.
#' This function is mostly useful for R/rClr developers. It is derived from the 'showArgs'
#' example in the R extension manual
#'
#' @param ... one or more R objects
#' @return NULL. Information is printed, not returned.
#' @export
inspectArgs <- function(...) {
  extPtr <-.External("r_show_args", ..., PACKAGE=nativePkgName)
  # return(mkClrObjRef(extPtr))
}

#' Get the COM variant type of a CLR object
#'
#' Get the COM variant type of a CLR object, e.g. "VT_ARRAY | VT_I8". This function only works when run on the Microsoft implementation of the CLR.
#' This function is useful for advanced diagnosis; most users should never have to use it.
#'
#' @param objOrType a CLR object, or type name, possibly namespace and assembly qualified type name, e.g. 'My.Namespace.MyClass,MyAssemblyName'.
#' @param methodName the name of the method called on the object or class specified with objOrType
#' @param ... one or more arguments to pass to the function call
#' @examples
#' \dontrun{
#' library(rClr)
#' cTypename <- "Rclr.TestCases"
#' #         public static bool IsTrue(bool arg)
#' clrVT(cTypename, 'IsTrue', TRUE)
#' clrVT('System.Convert', 'ToInt64', 123L)
#' clrVT('System.Convert', 'ToUInt64', 123L)
#' }
#' @return A string
#' @export
clrVT <- function(objOrType, methodName, ...) {
  if(nativePkgName!='rClrMs') {stop("The CLR is not Microsoft's. This function is CLR specific")}
  return(clrCallStatic('Rclr.DataConversionHelper', 'GetReturnedVariantTypename',objOrType, methodName, ...))
  # return(mkClrObjRef(extPtr))
}

#' Gets the type of a CLR object resulting from converting an R object
#'
#' Gets the type of a CLR object resulting from converting an R object. This function is mostly for documentation purposes, but may be of use to end users.
#'
#' @param x An R objects
#' @return A list, with columns including mode, type,class,length and the string of the corresponding CLR type.
#' @export
rToClrType <- function(x) {
  list(
    # what = str(x),
    mode = mode(x),
    type = typeof(x),
    class = class(x),
    length=length(x),
    clrType = clrCallStatic('Rclr.ClrFacade', 'GetObjectTypeName', x)
  )
}

#' Gets the type of a CLR object given its type name
#'
#' Gets the type of a CLR object given its type name
#'
#' @param objOrTypename a character vector of length one. It can be the full file name of the assembly to load, or a fully qualified assembly name, or as a last resort a partial name.
#' @return the CLR Type.
#' @export
clrGetType <- function(objOrTypename) {
  if(is.character(objOrTypename))
    return(clrCallStatic(clrFacadeTypeName, 'GetType',objOrTypename))
  else if('cobjRef' %in% class(objOrTypename))
    return(clrCall(objOrTypename, 'GetType'))
  else
    stop('objOrTypename is neither a cobjRef object nor a character vector')
}


#' Create a reference object wrapper around a CLR object
#'
#' (EXPERIMENTAL) Create a reference object wrapper around a CLR object
#'
#' @param obj an object of S4 class clrObj
#' @param envClassWhere environment where the new generator is created.
#' @return the reference object.
clrCobj <- function(obj, envClassWhere=.GlobalEnv) {
  refgen <- setClrRefClass(obj@clrtype, envClassWhere)
  refgen$new(ref=obj)
}

#' Create reference classes for an object hierarchy
#'
#' EXPERIMENTAL Create reference classes for an object hierarchy. Gratefully acknowledge Peter D. and its rJavax work.
#'
#' @param typeName a CLR type name, recognizable by clrGetType
#' @param env environment where the new generator is created.
#' @return the object generator function
setClrRefClass <- function(typeName,
                            env=topenv(parent.frame()))
{
  isAbstract <- function(type) { clrGet(type, 'IsAbstract' ) }
  isInterface <- function(type) { clrGet(type, 'IsInterface' ) }

  tryCatch(getRefClass(typeName),
        error=function(e) {
          type <- clrGetType(typeName)
          if(is.null(type)) stop(paste('CLR type not found for type name', typeName))

          baseType <- clrGet(type, 'BaseType')
          baseTypeName <- NULL
          if (!is.null(baseType)) {
            baseTypeName <- clrGet(baseType, 'FullName')
            setClrRefClass(baseTypeName, env)
          }

          # interfaces <- Map(function(interface) interface$getName(),
                           # as.list(class$getInterfaces()))

          # If the type is the type for an interface, then GetInterfacesFullnames will not return 'itself', so no need to deal with infinite recursion here.
          interfaces <- clrCallStatic(reflectionHelperTypeName, 'GetInterfacesFullnames', type)

          for (ifname in interfaces)
           setClrRefClass(ifname, env)

          ## sort the interfaces lexicographically to avoid inconsistencies
          contains <- c(baseTypeName,
                       sort(as.character(unlist(interfaces))))

          ## if an interface or an abstract type, need to contain VIRTUAL
          if (isInterface(type) || isAbstract(type))
           contains <- c(contains, "VIRTUAL")

          declaredMethods <- clrCallStatic(reflectionHelperTypeName, 'GetDeclaredMethodNames', type)
           # Map(function(method) method$getName(),
               # Filter(notProtected, as.list(class$getDeclaredMethods())))
          declaredMethods <- unique(declaredMethods)

          methods <- sapply(as.character(declaredMethods), function(method) {
            eval(substitute(function(...) {
              arguments <- Map(function(argument) {
                if (is(argument, 'System.Object')) {
                  argument$ref
                } else
                argument
              }, list(...))
              'TODO Here there should be the method description'
              do.call(clrCall, c(.self$ref, method, arguments))
            }, list(method=method)))
          })

          if (typeName == "System.Object")
          setRefClass("System.Object",
                      fields = list(ref = 'cobjRef'),
                      methods = c(methods,
                        initialize = function(...) {
                          argu <- list(...)
                          x <- argu[['ref']]
                          if(!is.null(x)) {
                            ref <<- x
                          } else {
                            ref <<- clrNew(class(.self), ...)
                          }
                          .self
                        # },
                        # copy = function(shallow = FALSE) {
                          # ## unlike clone(), this preserves any
                          # ## fields that may be present in
                          # ## an R-specific subclass
                          # x <- callSuper(shallow)
                          # x$ref <- ref$clone()
                          # x
                        }),
                      contains = contains,
                      where = env)
          else setRefClass(typeName,
                          methods = methods,
                          contains = contains,
                          where = env)
        })
}
