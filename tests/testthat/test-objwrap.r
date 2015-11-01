# testDir <- system.file('tests', package='rClr') 
# stopifnot(file.exists(testDir))
# source(file.path(testDir, 'load_libs.r'))

# context("rClr wrappers using R references classes")

# test_that("Object constructors calls work", {
  # tName <- testClassName
  # i1 <- 23L ; i2 <- 42L 
  # d1 <- 1.234; d2 <- 2.345;
  # obj <- clrNew(tName, i1)
  # obj <- clrCobj(obj)
# # > class(w)
# # [1] "Rcpp_World"
# # attr(,"package")
# # [1] "rcppf"
# # > 
  # expect_equal(as.character(class(obj)), tName)
  # expect_equal(attributes(class(obj)), list(package='rClr'))
  # expect_equal(obj$FieldIntegerOne, i1 );
# })

# test_that("Basic types of length one are marshalled correctly", {
  # tNameLOne <- 'Rclr.Tests.RefClasses.LevelOneClass'
  # tNameLTwo <- 'Rclr.Tests.RefClasses.LevelTwoClass'
  # tNameLThree <- 'Rclr.Tests.RefClasses.LevelThreeClass'

  # obj <- clrNew(tNameLTwo)
  # obj <- clrCobj(obj)
  # # obj <- refgen$new()
  # expect_equal(as.character(class(obj)), tNameLTwo)
  # expect_true(inherits(obj,tNameLOne))
  
  # expect_equal(obj$AbstractMethod(), "LevelOneClass::AbstractMethod()")
  # expect_equal(obj$AbstractMethod('some_string'), "LevelOneClass::AbstractMethod(string)")
  # expect_equal(obj$VirtualMethod(), "BaseAbstractClassOne::VirtualMethod()")
  # expect_equal(obj$IfBaseTwoMethod(), "LevelTwoClass::IfBaseTwoMethod()")

  # s <- "test string"
  # obj$IfBaseOneString <- s
  # expect_equal(obj$IfBaseOneString, s)

  # # Check that explicit interface implementations are working
  # expect_equal(obj$IfOneStringGetter, "Explicit LevelOneClass::InterfaceOne.IfOneStringGetter")

  
  # obj <- clrNew(tNameLThree)
  # obj <- clrCobj(obj)
  # expect_equal(as.character(class(obj)), tNameLTwo)
  # expect_true(inherits(obj,tNameLOne))
  # expect_true(inherits(obj,tNameLTwo))
  
  # expect_equal(obj$AbstractMethod(), "LevelThreeClass::AbstractMethod()")
  # expect_equal(obj$AbstractMethod('some_string'), "LevelOneClass::AbstractMethod(string)")
  # expect_equal(obj$VirtualMethod(), "LevelThreeClass::VirtualMethod()")

  # s <- "test string"
  # obj$IfBaseOneString <- s
  # expect_equal(obj$IfBaseOneString, paste( "Overriden LevelThreeClass::IfBaseOneString", s))
  
# })
