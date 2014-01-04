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

test_that("Basic types of length one are marshalled correctly", {
  tNameLOne <- 'Rclr.Tests.RefClasses.LevelOneClass'
  tNameLTwo <- 'Rclr.Tests.RefClasses.LevelTwoClass'
  obj <- clrNew(tNameLTwo)
  obj <- clrCobj(obj)
  expect_equal(class(obj), tNameLTwo)
  expect_that(inherits(obj,tNameLOne))
  
  expect_equal(obj$AbstractMethod(), "LevelOneClass:AbstractMethod()")
  expect_equal(obj$AbstractMethod('some_string'), "LevelOneClass:AbstractMethod(string)")
  
})
