testDir <- system.file('tests', package='rClr') 
stopifnot(file.exists(testDir))
source(file.path(testDir, 'load_libs.r'))

context("rClr wrappers using R references classes")

test_that("Object constructors calls work", {
  tName <- testClassName
  i1 <- 23L ; i2 <- 42L 
  d1 <- 1.234; d2 <- 2.345;
  obj <- clrNew(tName, i1)
  obj <- clrCobj(obj)
# > class(w)
# [1] "Rcpp_World"
# attr(,"package")
# [1] "rcppf"
# > 
  expect_equal(class(obj), tName)
  expect_equal( obj$FieldIntegerOne, i1 );
})

#' Create reference classes for an object hierarchy
#'
#' Create reference classes for an object hierarchy
#'
#' @return the object generator function
#' @export
setClrRefClassDev <- function(typeName, # TODO: acknowledge rJavax inspired... http://cran.csiro.au/web/packages/rJavax/index.html

                            where=topenv(parent.frame()))
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
            setClrRefClassDev(baseTypeName, where)
          }

          # interfaces <- Map(function(interface) interface$getName(),
                           # as.list(class$getInterfaces()))
                           
          # If the type is the type for an interface, then GetInterfacesFullnames will not return 'itself', so no need to deal with infinite recursion here.
          interfaces <- clrCallStatic(reflectionHelperTypeName, 'GetInterfacesFullnames', type)

          for (ifname in interfaces)
           setClrRefClassDev(ifname, where)

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
                      where = where)
          else setRefClass(typeName,
                          methods = methods,
                          contains = contains,
                          where = where)
        })
}

clrCobj <- function(obj) {
  refgen <- setClrRefClassDev(obj@clrtype)
  refgen$new(ref=obj)
}

test_that("Basic types of length one are marshalled correctly", {
  tNameLOne <- 'Rclr.Tests.RefClasses.LevelOneClass'
  tNameLTwo <- 'Rclr.Tests.RefClasses.LevelTwoClass'
  tNameLThree <- 'Rclr.Tests.RefClasses.LevelThreeClass'

  obj <- clrNew(tNameLTwo)
  obj <- clrCobj(obj)
  # obj <- refgen$new()
  expect_equal(as.character(class(obj)), tNameLTwo)
  expect_true(inherits(obj,tNameLOne))
  
  expect_equal(obj$AbstractMethod(), "LevelOneClass::AbstractMethod()")
  expect_equal(obj$AbstractMethod('some_string'), "LevelOneClass::AbstractMethod(string)")
  expect_equal(obj$VirtualMethod(), "BaseAbstractClassOne::VirtualMethod()")
  expect_equal(obj$IfBaseTwoMethod(), "LevelTwoClass::IfBaseTwoMethod()")

  s <- "test string"
  obj$IfBaseOneString <- s
  expect_equal(obj$IfBaseOneString, s)

  # Check that explicit interface implementations are working
  expect_equal(obj$IfOneStringGetter, "Explicit LevelOneClass::InterfaceOne.IfOneStringGetter")

  
  obj <- clrNew(tNameLThree)
  obj <- clrCobj(obj)
  expect_equal(as.character(class(obj)), tNameLTwo)
  expect_true(inherits(obj,tNameLOne))
  expect_true(inherits(obj,tNameLTwo))
  
  expect_equal(obj$AbstractMethod(), "LevelThreeClass::AbstractMethod()")
  expect_equal(obj$AbstractMethod('some_string'), "LevelOneClass::AbstractMethod(string)")
  expect_equal(obj$VirtualMethod(), "LevelThreeClass::VirtualMethod()")

  s <- "test string"
  obj$IfBaseOneString <- s
  expect_equal(obj$IfBaseOneString, paste( "Overriden LevelThreeClass::IfBaseOneString", s))
  
})
