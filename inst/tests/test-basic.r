testDir <- system.file('tests', package='rClr') 
stopifnot(file.exists(testDir))
source(file.path(testDir, 'load_libs.r'))

context("rClr essentials")

areClrRefEquals <- function(x, y) {clrCallStatic('System.Object', 'ReferenceEquals', x, y)}

expectArrayTypeConv <- function(clrType, arrayLength, expectedRObj) {
  tn <- "Rclr.TestArrayMemoryHandling"
  arrayLength <- as.integer(arrayLength)
  expect_equal( clrCallStatic(tn, paste0("CreateArray_", clrType), arrayLength ), expectedRObj )
}

createArray <- function(clrType, arrayLength, elementObject) {
  tn <- "Rclr.TestArrayMemoryHandling"
  arrayLength <- as.integer(arrayLength)
  if(missing(elementObject)) { return(clrCallStatic(tn, paste0("CreateArray_", clrType), arrayLength )) }
  clrCallStatic(tn, paste0("CreateArray_", clrType), arrayLength, elementObject )
}

expectClrArrayElementType <- function(rObj, expectedClrTypeName) {
  tn <- "Rclr.TestArrayMemoryHandling"
  expect_true( clrCallStatic(tn, 'CheckElementType', rObj , clrGetType(expectedClrTypeName) ))
}

callTestCase <- function(...) {
  clrCallStatic(cTypename, ...)
}

test_that("Booleans are marshalled correctly", {
  expect_that( callTestCase( "GetFalse"), is_false() )
  expect_that( callTestCase( "GetTrue"), is_true() )
  expect_that( callTestCase( "IsTrue", TRUE), is_true() )
  expect_that( callTestCase( "IsTrue", FALSE), is_false() )
})

test_that("Object constructors calls work", {
  tName <- 'Rclr.TestObject'
  i1 <- as.integer(23) ; i2 <- as.integer(42) 
  d1 <- 1.234; d2 <- 2.345;
  obj <- clrNew(tName)
  obj <- clrNew(tName, i1)
  expect_that( clrGet(obj, "FieldIntegerOne"), equals(i1) )
  obj <- clrNew(tName, i1, i2)
  expect_that( clrGet(obj, "FieldIntegerOne"), equals(i1) )
  expect_that( clrGet(obj, "FieldIntegerTwo"), equals(i2) )
  obj <- clrNew(tName, d1, d2)
  expect_that( clrGet(obj, "FieldDoubleOne"), equals(d1) )
  expect_that( clrGet(obj, "FieldDoubleTwo"), equals(d2) )
})

test_that("Basic types of length one are marshalled correctly", {
  expect_that( callTestCase( "DoubleEquals", 123.0 ), is_true() )
  expect_that( callTestCase( "CreateDouble"), equals(123.0) )
  expect_that( callTestCase( "IntEquals", as.integer(123) ), is_true() )
  expect_that( callTestCase( "CreateInt"), equals(as.integer(123)) )
  expect_that( callTestCase( "StringEquals", 'ab' ), is_true() )
  expect_that( callTestCase( "CreateString"), equals('ab') )
# TODO: test unicode characters: what is happening then
})

test_that("Basic types of length zero are marshalled correctly", {
  tn <- "Rclr.TestArrayMemoryHandling"
  
  expectEmptyArrayConv <- function(clrType, expectedRObj) {
    expectArrayTypeConv( clrType, 0L , expectedRObj )
  }
  expectEmptyArrayConv( 'float' , numeric(0) )
  expectEmptyArrayConv( 'double', numeric(0) )
  expectEmptyArrayConv( 'int' , integer(0) )
  expectEmptyArrayConv( 'byte' , raw(0) )
  expectEmptyArrayConv( 'char' , character(0) )
  expectEmptyArrayConv( 'bool' , logical(0) )
  expectEmptyArrayConv( 'string' , character(0) )
  
  expect_error( clrCallStatic(tn, 'CreateArray_long', numeric(0) ) )

  expectEmptyArrayConv( 'object' , list() )
  expectEmptyArrayConv( 'Type' , list() )
  
  # a <- now()
  # str(a)
  # str(unclass(a))
  # mode(unclass(a))
  # a <- numeric(0)
  # attributes(a) <- list(tzone='')
  # str(a)
  # class(a) <- c('POSIXct', 'POSIXt')
  # a # <== Curious
  # str(a)

  # check that we fixed https://rclr.codeplex.com/workitem/2
  expectClrArrayElementType( numeric(0)   ,'System.Double')
  expectClrArrayElementType( integer(0)   ,'System.Int32') 
  expectClrArrayElementType( raw(0)       ,'System.Byte')  
  expectClrArrayElementType( logical(0)   ,'System.Boolean')
  expectClrArrayElementType( character(0) ,'System.String')


  aPosixCt <- numeric(0)
  attributes(aPosixCt) <- list(tzone='UTC')
  class(aPosixCt) <- c('POSIXct', 'POSIXt')
  
  expect_equal( clrCallStatic(tn, 'CreateArray_DateTime', 0L ), aPosixCt )

  tdiff <- numeric(0)
  class(tdiff) <- 'difftime'
  attr(tdiff, 'units') <- 'secs'
  expect_equal( clrCallStatic(tn, 'CreateArray_TimeSpan', 0L), tdiff )

  expectClrArrayElementType( aPosixCt     ,'System.DateTime')
  expectClrArrayElementType( tdiff        ,'System.TimeSpan')
  
})



test_that("non-empty arrays of non-basic .NET objects are handled", {
  tn <- "Rclr.TestArrayMemoryHandling"
  tName <- 'Rclr.TestObject'
  
  testListEqual <- function(expectObj, expectedLength, actual) {
    expect_equal(expectedLength, length(actual))
    expect_true(is.list(actual))
    expect_true(all(sapply(actual, FUN = function(x) { areClrRefEquals(expectObj, x)})))
  }  
  
  obj <- clrNew(tName)
  actual <- clrCallStatic(tn, "CreateArray_object", 3L, obj)
  testListEqual(obj, 3L, actual) 
  
  aType <- clrGetType('System.Double')
  actual <- clrCallStatic(tn, "CreateArray_Type", 3L, aType)
  testListEqual(aType, 3L, actual) 
})  


if(clrGetInnerPkgName()=="rClrMs")
{
  test_that("MS CLR: check that the variant types are reported correctly", {
    #         public static bool IsTrue(bool arg)clrCallStatic(tn, "CreateArray_double", 0L )
    expect_equal(clrVT(cTypename, 'IsTrue', TRUE), "VT_BOOL")
    expect_equal(clrVT('System.Convert', 'ToInt64', 123L), "VT_I8")
    expect_equal(clrVT('System.Convert', 'ToUInt64', 123L), "VT_UI8")
    tn <- "Rclr.TestArrayMemoryHandling"
    expect_equal( clrVT(tn, "CreateArray_DateTime", 0L ), "VT_ARRAY | VT_DATE" )
    # expect_equal( clrVT(tn, "CreateArray_Type", 3L, clrGetType('System.Double')), "VT_ARRAY | VT_DATE" )
  })
}

test_that("String arrays are marshalled correctly", {
  ltrs = paste(letters[1:5], letters[2:6], sep='')
  expect_that( callTestCase( "StringArrayEquals", ltrs), is_true() )
  expect_that( callTestCase( "CreateStringArray"), equals(ltrs) )
  
  ltrs[3] = NA
  # expect_that( callTestCase( "CreateStringArrayMissingVal"), equals(ltrs) )
  # expect_that(callTestCase( "StringArrayMissingValsEquals", ltrs), is_true() )
  
})

test_that("clrGetType function", {
  testObj <- clrNew(testClassName)
  expect_equal( testClassName,clrGet(clrGetType(testClassName), 'FullName'))
  expect_equal( testClassName,clrGet(clrGetType(testObj), 'FullName'))
   
})


test_that("Numeric arrays are marshalled correctly", {
  expectedNumArray <- 1:5 * 1.1  
  expect_that( callTestCase( "CreateNumArray"), equals(expectedNumArray) )
  ## Internally somewhere, some noise is added probably in a float to double conversion. 
  ## Expected, but 5e-8 is more difference than I'd have guessed. Some watch point.
  # expect_that( callTestCase( "CreateFloatArray"), equals(expectedNumArray) )
  expect_equal( callTestCase( "CreateFloatArray"), expected = expectedNumArray, tolerance = 5e-8, scale = 2)
  expect_that( callTestCase( "NumArrayEquals", expectedNumArray ), is_true() )

  numDays = 5
  expect_equal( callTestCase( "CreateIntArray", as.integer(numDays)), expected = 0:(numDays-1))

  expectedNumArray[3] = NA
  expect_that( callTestCase( "CreateNumArrayMissingVal"), equals(expectedNumArray) )
  expect_that( callTestCase( "NumArrayMissingValsEquals", expectedNumArray ), is_true() )
    
})

test_that("Complex numbers are converted", {
  z = 1+2i
  expect_equal(callTestCase( "CreateComplex", 1, 2), z)
  expect_true(callTestCase( "ComplexEquals", z, 1, 2 ))
  z = c(1+2i, 3+4i, 3.3+4.4i)
  expect_equal(callTestCase( "CreateComplex", c(1,3,3.3), c(2,4,4.4)), z)
  expect_true(callTestCase( "ComplexEquals", z, c(1,3,3.3), c(2,4,4.4)))
  

})

# TODO: test that passing an S4 object that is not a clr object converts to a null reference in the CLR

test_that("Methods with variable number of parameters with c# 'params' keyword", {
  testObj <- clrNew(testClassName)
  actual <- clrCall(testObj, "TestParams", "Hello, ", "World!", 1L, 2L, 3L, 6L, 5L, 4L)
  expected <- "Hello, World!123654"
  expect_equal(actual, expected=expected)
  actual <- clrCall(testObj, "TestParams", "Hello, ", "World!", as.integer(1:6))
  expected <- "Hello, World!123456"
  expect_equal(actual, expected=expected)
})

test_that("Correct method binding based on parameter types", {
  mkArrayTypeName <- function(typeName) { paste(typeName, '[]', sep='') }
  f <- function(...){ clrCallStatic('Rclr.TestMethodBinding', 'SomeStaticMethod', ...) }
  printIfDifferent <- function( got, expected ) { if(any(got != expected)) {print( paste( "got", got, ", expected", expected))} }
  g <- function(values, typeName) {
    if(is.list(values)) { # this is what one gets with a concatenation of S4 objects, when we use c(testObj,testObj) with CLR objects
      printIfDifferent( f(values[[1]]), typeName)
      printIfDifferent( f(values), mkArrayTypeName(typeName)) # This is not yet supported?
      printIfDifferent( f(values[[1]], values[[2]]), rep(typeName, 2))
      expect_equal( f(values[[1]]), typeName)
      expect_equal( f(values), mkArrayTypeName(typeName))
      expect_equal( f(values[[1]], values[[2]]), rep(typeName, 2))
    } else {
      printIfDifferent( f(values[1]), typeName)
      printIfDifferent( f(values), mkArrayTypeName(typeName))
      printIfDifferent( f(values[1], values[2]), rep(typeName, 2))
      expect_equal( f(values[1]), typeName)
      expect_equal( f(values), mkArrayTypeName(typeName))
      expect_equal( f(values[1], values[2]), rep(typeName, 2))
    }
  }
  intName <- 'System.Int32'
  doubleName <- 'System.Double'
  stringName <- 'System.String'
  boolName <- 'System.Boolean'
  dateTimeName <- 'System.DateTime'
  objectName <- 'System.Object'
  testObj <- clrNew(testClassName)
  
  testMethodBinding <- function() {
    g(1:3, intName)
    g(1.2*1:3, doubleName)
    g(letters[1:3], stringName)
    g(rep(TRUE,3), boolName)
    g(as.Date('2001-01-01') + 1:3, dateTimeName)
    g(c(testObj,testObj,testObj), objectName )

    expect_equal( f(1.0, 'a'), c(doubleName, stringName))
    expect_equal( f(1.0, 'a', 'b'), c(doubleName, stringName, stringName))
    expect_equal( f(1.0, letters[1:2]), c(doubleName, mkArrayTypeName(stringName)))
    expect_equal( f(1.0, letters[1:10]), c(doubleName, mkArrayTypeName(stringName)))
    
    expect_equal( f('a', letters[1:3]), c(stringName, mkArrayTypeName(stringName)) )
    expect_equal( f(letters[1:3], 'a'), c(mkArrayTypeName(stringName), stringName) )
    expect_equal( f(letters[1:3], letters[4:6]), c(mkArrayTypeName(stringName), mkArrayTypeName(stringName)) )
  }  
  testMethodBinding()
  obj <- clrNew('Rclr.TestMethodBinding')
  f <- function(...){ clrCall(obj, 'SomeInstanceMethod', ...) }
  testMethodBinding()
  # Test that methods implemented to comply with an interface are found, even if the method is explicitely implemented.
  # We do not want the users to have to figure out which interface type they deal with, at least not for R users.
  f <- function(...){ clrCall(obj, 'SomeExplicitlyImplementedMethod', ...) }
  testMethodBinding()
})


test_that("Numerical bi-dimensional arrays are marshalled correctly", {
  numericMat = matrix(as.numeric(1:15), nrow=3, ncol=5, byrow=TRUE)
  # A natural marshalling of jagged arrays is debatable. For the time being assuming that they are matrices, due to the concrete use case.
  expect_that( callTestCase( "CreateJaggedFloatArray"), equals(numericMat))
  expect_that( callTestCase( "CreateJaggedDoubleArray"), equals(numericMat))
  expect_that( callTestCase( "CreateRectFloatArray"), equals(numericMat))
  expect_that( callTestCase( "CreateRectDoubleArray"), equals(numericMat))

  # expect_that( callTestCase( "NumericMatrixEquals", numericMat), equals(numericMat))

})

testSmartDictConversion <- function(){
  # The definition of 'as expected' for these collections is not all that clear, and there may be some RDotNet limitations.
  expect_that( callTestCase( "CreateStringDictionary"), equals(list(a='A', b='B')))
  expect_that( callTestCase( "CreateStringDoubleArrayDictionary"), 
    equals(
      list(
      a=c(1.0, 2.0, 3.0, 3.5, 4.3, 11),
      b=c(1.0, 2.0, 3.0, 3.5, 4.3 ),
      c=c(2.2, 3.3, 6.5))
      )
    )
  # d <- callTestCase( "CreateObjectDictionary")
  # expect_true
}

test_that("CLI dictionaries are marshalled as expected", {
  testSmartDictConversion()
})

test_that("Conversion of non-bijective types can be turned on/off", {
  setConvertAdvancedTypes(FALSE)
  expect_true( is(callTestCase( "CreateStringDictionary"), 'cobjRef'))
  expect_true( is(callTestCase( "CreateStringDoubleArrayDictionary"), 'cobjRef') )
  setConvertAdvancedTypes(TRUE)
  expect_false( is(callTestCase( "CreateStringDictionary"), 'cobjRef'))
  expect_false( is(callTestCase( "CreateStringDoubleArrayDictionary"), 'cobjRef') )
  testSmartDictConversion()
})

test_that("Basic objects are created correctly", {
  testObj = clrNew(testClassName)
  expect_that( testObj@clrtype, equals(testClassName))
  rm(testObj)
  # Note to self: I originally wrote code to make sure that r_call_static_method kept returning an external pointer, not 
  # an R object of type clrObjRef already created. I am not sure why this would have been a compulsory behavior. 
  # Delete if no harm done...
#	 extptr <-.External("r_call_static_method", cTypename, "CreateTestObject",PACKAGE=clrGetNativeLibName())
#  expect_that(is.null(extptr), is_false())
#  expect_that("externalptr" %in% class(extptr), is_true())
#  expect_that(clrTypeNameExtPtr(extptr), equals(testClassName))
	testObj <-.External("r_call_static_method", cTypename, "CreateTestObject",PACKAGE=clrGetNativeLibName())
  expect_that(is.null(testObj), is_false())
  expect_that( testObj@clrtype, equals(testClassName))
  rm(testObj)
	testObj <- callTestCase( "CreateTestObject")
  expect_that(is.null(testObj), is_false())
  expect_that( testObj@clrtype, equals(testClassName))

  # cover part of the issue https://rclr.codeplex.com/workitem/39
	testObj <- callTestCase( "CreateTestObjectGenericInstance")
  expect_that(is.null(testObj), is_false())
  
  
  testObj <- callTestCase( "CreateTestArrayGenericObjects")
  testObj <- callTestCase( "CreateTestArrayInterface")
  testObj <- callTestCase( "CreateTestArrayGenericInterface")
  
})

test_that("Creation of SEXP via R.NET", {
  # cover issue https://rclr.codeplex.com/workitem/42. Just check that the stack imbalance warning does not show up (could not find a way to check this via testthat; warnings() is not affected by the warning given by the stack imbalance checking mechanism in R itself. This is probably because check_stack_balance uses the function REprintf instead of warningcall
  aDataFrame <- callTestCase("CreateTestDataFrame")
})

test_that("CLR type compatibility checking", {
  testObj <- clrNew(testClassName)
  expect_true(clrIs(testObj, testClassName))
  expect_true(clrIs(testObj, 'System.Object'))
  expect_false(clrIs(testObj, 'System.Double'))
  testObj <- clrNew('Rclr.TestMethodBinding')
  expect_true(clrIs(testObj, 'Rclr.ITestMethodBindings'))
  expect_true(clrIs(testObj, clrGetType('Rclr.ITestMethodBindings')))
  expect_true(clrIs(testObj, clrGetType('Rclr.TestMethodBinding')))
  expect_false(clrIs(testObj, clrGetType('System.Reflection.Assembly')))
  expect_error(clrIs(testObj, testObj))
})

test_that("Loaded assemblies discovery", {
  expect_that(all(c('ClrFacade', 'mscorlib') %in% clrGetLoadedAssemblies()), is_true())
  d <- clrGetLoadedAssemblies(fullname=TRUE, filenames=TRUE)
  expect_true(is.data.frame(d))
})

test_that("Object members discovery behaves as expected", {
  expect_that('Rclr.TestObject' %in% clrGetTypesInAssembly('ClrFacade'), is_true())
  testObj = clrNew(testClassName)
  members = clrReflect(testObj)

  f<- function(obj_or_tname, static=FALSE, getF, getP, getM) { # copy-paste may have been more readable... Anyway.
    prefix <- ifelse(static, 'Static', '') 
    collate <- function(...) {paste(..., sep='')} # surely in stringr, but avoid dependency
    p <- function(basefieldname) {collate(prefix, basefieldname)}

    expect_that(getF(obj_or_tname, 'IntegerOne'), equals(p('FieldIntegerOne')))
    expect_that(getP(obj_or_tname, 'IntegerOne'), equals(p('PropertyIntegerOne')))

    expected_mnames <- paste(c('get_','','set_'), p(c('PropertyIntegerOne', "GetFieldIntegerOne", "PropertyIntegerOne")), sep='')
    actual_mnames <- getM(obj_or_tname, 'IntegerOne')

    expect_that( length(actual_mnames), equals(length(expected_mnames)))
    expect_that( all( actual_mnames %in% expected_mnames), is_true())

    sig_prefix = ifelse(static, 'Static, ', '')
    expect_that(clrGetMemberSignature(obj_or_tname, p('GetFieldIntegerOne')), 
      equals(collate(sig_prefix, "Method: Int32 ", p("GetFieldIntegerOne"))))
    expect_that(clrGetMemberSignature(obj_or_tname, p('GetMethodWithParameters')), 
      equals(collate(sig_prefix, "Method: Int32 ", p("GetMethodWithParameters, Int32, String"))))
  }
  f(testObj, static=FALSE, clrGetFields, clrGetProperties, clrGetMethods)
  f(testClassName, static=TRUE, clrGetStaticFields, clrGetStaticProperties, clrGetStaticMethods)
  # TODO test that methods that are explicit implementations of interfaces are found
})

test_that("Object constructor discovery behaves as expected", {
  expect_equal(  
    c("Constructor: .ctor"                              
      , "Constructor: .ctor, Double"                      
      , "Constructor: .ctor, Double, Double"              
      , "Constructor: .ctor, Int32"                       
      , "Constructor: .ctor, Int32, Int32"                
      , "Constructor: .ctor, Int32, Int32, Double, Double"),
    clrGetConstructors(testClassName)
  )
})

test_that("Retrieval of object or class (i.e. static) members values behaves as expected", {
  f <- function(obj_or_type, rootMemberName, staticPrefix='') {
    fieldName <- paste(staticPrefix, 'Field', rootMemberName, sep='')
    propName <- paste(staticPrefix, 'Property', rootMemberName, sep='')
    clrSet(obj_or_type, fieldName, as.integer(0))
    expect_that(clrGet(obj_or_type, fieldName), equals(0))
    clrSet(obj_or_type, fieldName, as.integer(2))
    expect_that(clrGet(obj_or_type, fieldName), equals(2))
    clrSet(obj_or_type, propName, as.integer(0))
    expect_that(clrGet(obj_or_type, propName), equals(0))
    clrSet(obj_or_type, propName, as.integer(2))
    expect_that(clrGet(obj_or_type, propName), equals(2))
  }
  # first object members
  testObj = clrNew(testClassName)
  f(testObj, 'IntegerOne', staticPrefix='')
  # then test static members
  f(testClassName, 'IntegerOne', staticPrefix='Static')
})

test_that("enums get/set", {
  # very basic support for the time being. Behavior to be defined for cases such as enums with binary operators ([FlagsAttribute]) 
  eType <- 'Rclr.TestEnum'
  expect_that(clrGetEnumNames(eType), equals(c('A','B','C')))  
#  TODO, but problematic.
#  e <- clrCall(cTypename, 'GetTestEnum', 'B')
#  expect_false(is.null(e))  
#  expect_that(clrCall(e, 'ToString'), equals('B'))  
})

testGarbageCollection <- function( getObjCountMethodName = 'GetMemTestObjCounter', createTestObjectMethodName = 'CreateMemTestObj')
{
  callGcMethname <- "CallGC"
  forceDotNetGc <- function() { callTestCase( callGcMethname) }
  checkPlusOne <- function () { expect_that( callTestCase( getObjCountMethodName), equals(counter+1) ) }

  counter = callTestCase( getObjCountMethodName)
  expect_that( counter, equals(0) ) # make sure none of these test objects instances are hanging in the CLR
  testObj = callTestCase( createTestObjectMethodName)
  checkPlusOne()
  forceDotNetGc()
  # the object should still be in memory.
  checkPlusOne()
  gc()
  # the object should still be in memory, since testObj is in use and thus the underlying clr handle should be pinned too.
  checkPlusOne()
  rm(testObj)
  gc()
  forceDotNetGc()
  expect_that( callTestCase( getObjCountMethodName), equals(counter) ) 
  
  # Trying to test that issue https://r2clr.codeplex.com/workitem/71 is fixed. 
  # However the underlying COM type for a Form is a VT_DISPATCH but for out MemTestObject a VT_UNKNOWN. 
  # Needs more work to reproduce in a unit test; no desire to introduce dependency on System.Windows.Form
  # counter = callTestCase( getObjCountMethodName)
  # expect_that( counter, equals(0) )
  # testObj = callTestCase( createTestObjectMethodName)
  # clrSet( testObj, 'Text', "et nous alimentons nos aimables remords comme les mendiants nourissent leur vermine" )
  # forceDotNetGc()
  # checkPlusOne()
  # clrSet( testObj, 'Text', "Sur l'oreiller du mal..." )
  # checkPlusOne()
  # rm(testObj) ; gc() ; forceDotNetGc()
  
}

test_that("Garbage collection in R and the CLR behaves as expected", {
  testGarbageCollection( getObjCountMethodName = 'GetMemTestObjCounter', createTestObjectMethodName = 'CreateMemTestObj')
})

test_that("Garbage collection of R.NET objects", {
  # Unfortunately cannot test this yet because of http://r2clr.codeplex.com/workitem/30
  # testGarbageCollection( getObjCountMethodName = 'GetMemTestObjCounterRDotnet', createTestObjectMethodName = 'CreateMemTestObjRDotnet')
})


test_that("Assembly loading", {
  # following not supported on Mono
  # clrLoadAssembly("System.Windows.Presentation, Version=3.5.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089")
  clrLoadAssembly('System.Net.Http, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a')
  
  # The use of partial assembly names is discouraged; nevertheless it is supported
  clrLoadAssembly("System.Web.Services")
})