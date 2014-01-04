# Declares the S4 class that is used to hold references to CLR objects.
setClass("cobjRef", representation(clrobj="externalptr", clrtype="character"), prototype=list(clrobj=NULL, clrtype="System.Object"))


setClrRefClass <- function(className,
                            where=topenv(parent.frame()))
{
  tryCatch(getRefClass(className),
        error=function(e) {
          class <- clrGetType(className)

          superclass <- clrGet(class, 'BaseType')
          superclassName <- NULL
          if (!is.null(superclass)) {
            superclassName <- clrGet(class, 'FullName')
            setJavaRefClass(superclassName, where)
          }

          # interfaces <- Map(function(interface) interface$getName(),
                           # as.list(class$getInterfaces()))
          interfaces <- clrCallStatic('Rclr.ReflectionHelper', 'GetInterfacesFullnames', class)

          for (ifname in interfaces)
           setJavaRefClass(ifname, where)

          ## sort the interfaces lexicographically to avoid inconsistencies
          contains <- c(superclassName,
                       sort(as.character(unlist(interfaces))))

          isAbstract <- function(class) { return clrGet(class, 'IsAbstract' ) }
          isInterface <- function(class) { return clrGet(class, 'IsInterface' ) }

          ## if an interface or an abstract class, need to contain VIRTUAL
          if (isInterface(class) || isAbstract(class))
           contains <- c(contains, "VIRTUAL")

          declaredMethods <- clrCallStatic('Rclr.ReflectionHelper', 'GetDeclaredMethodNames', class)
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

          if (className == "System.Object")
          setRefClass("System.Object",
                      fields = list(ref = 'cobjRef'),
                      methods = c(methods,
                        initialize = function(...) {
                          ref <<- clrNew(class(.self), ...)
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
                      where = where)
          else setRefClass(className,
                          methods = methods,
                          contains = contains,
                          where = where)
        })
}