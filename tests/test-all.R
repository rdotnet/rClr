# source('C:/src/codeplex/r2clr/packages/rClr/tests/test-all.R')
library(testthat)
library(rClr)
# if( Sys.getenv('RCLR') != 'Mono') { setRDotNet(TRUE) }
setRDotNet(TRUE)

testDir <- system.file('tests', package='rClr') 
stopifnot(file.exists(testDir))
source(file.path(testDir, 'load_libs.r'))

debug_test = FALSE
# debug_test = TRUE

test_package("rClr", 'basic') 
test_package("rClr", 'datetime') 
