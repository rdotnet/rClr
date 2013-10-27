testDir <- system.file('tests', package='rClr') 
stopifnot(file.exists(testDir))
source(file.path(testDir, 'load_libs.r'))

context("rClr wrappers using R references classes")

test_that("Object constructors calls work", {
  tName <- 'Rclr.TestObject'
  i1 <- as.integer(23) ; i2 <- as.integer(42) 
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
  obj <- clrNew(tName, i1, i2)
  expect_that( clrGet(obj, "FieldIntegerOne"), equals(i1) );
  expect_that( clrGet(obj, "FieldIntegerTwo"), equals(i2) );
  obj <- clrNew(tName, d1, d2)
  expect_that( clrGet(obj, "FieldDoubleOne"), equals(d1) );
  expect_that( clrGet(obj, "FieldDoubleTwo"), equals(d2) );
})

test_that("Basic types of length one are marshalled correctly", {
  expect_that( clrCallStatic(cTypename, "DoubleEquals", 123.0 ), is_true() );
  expect_that( clrCallStatic(cTypename, "CreateDouble"), equals(123.0) );
  expect_that( clrCallStatic(cTypename, "IntEquals", as.integer(123) ), is_true() );
  expect_that( clrCallStatic(cTypename, "CreateInt"), equals(as.integer(123)) );
  expect_that( clrCallStatic(cTypename, "StringEquals", 'ab' ), is_true() );
  expect_that( clrCallStatic(cTypename, "CreateString"), equals('ab') );
# TODO: test unicode characters: what is happening then
})
