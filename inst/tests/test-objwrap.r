testDir <- system.file('tests', package='rClr') 
stopifnot(file.exists(testDir))
source(file.path(testDir, 'load_libs.r'))

context("rClr wrappers using R references classes")

test_that("Basic operations on reference classes", {
  tName <- testClassName
  i1 <- 23L ; i2 <- 42L 
  d1 <- 1.234; d2 <- 2.345;
  obj <- clrNew(tName, i1)
  obj <- clrCobj(obj)
  expect_equal(obj$FieldIntegerOne, i1 );
  expect_equal(as.character(class(obj)), tName)
  rm(obj)
  
  obj <- clrNew(tName, i1, i2, d1, d2)
  expect_equal(obj$FieldIntegerOne, f1)
  expect_equal(obj$FieldIntegerTwo, f2)
  expect_equal(obj$FieldDoubleOne, d1)
  expect_equal(obj$FieldDoubleTwo, d2)

  # Active bindings work
  obj$FieldDoubleTwo <- pi
  expect_equal(obj$FieldDoubleTwo, pi)

  expect_equal(attributes(class(obj)), list(package='rClr'))
})

test_that("rClr functions work indifferently whether R6 classes or clrobj S4 classes ? TBD", {
})

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
