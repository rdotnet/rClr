########################
# Define a few test helper functions
########################

clrDateEquals <- function(d, isoDateTimeStr, tzIdClr) { clrCallStatic(cTypename, "UtcDateEquals", d, isoDateTimeStr, tzIdClr) } 
createDotNetDate <- function(...) { clrCallStatic(cTypename, "CreateDate", ...) }
createUtcDate <- function(isoDateTimeStr, tzIdClr) { clrCallStatic(cTypename, "UtcDateForTimeZone", isoDateTimeStr, tzIdClr) }
# convertClrTime <- function(isoDateTimeStr, tzIdClr_from, tzIdClr_to ) { clrCallStatic(cTypename, "ConvertTime", isoDateTimeStr, tzIdClr_from, tzIdClr_to) }


# ?Sys.timezone, See the examples 
tzIdR_AUest = "Australia/Sydney"

# IronPython: tz = [x for x in TimeZoneInfo.GetSystemTimeZones()]
tzId_AUest <- 
ifelse( tolower(Sys.info()['sysname'])== 'windows',
    ifelse(clrGetNativeLibName()=='rClrMono', 
      # As of Mono 3.8.0, and probably earlier releases including 3.4.0, the time zone names have changed. Not Olson DB anymore. Not MS.NET either. *Sigh*
      'E. Australia Standard Time', 
      # I think even on Linux Mono does not use the Olson DB names. If still, use something like the following line
      #  'E. Australia Standard Time', tzIdR_AUest), 
      "AUS Eastern Standard Time") # TODO: is 'Australia/Sydney' also OK for MS.NET?
    ,
tzIdR_AUest # on Linux, use the Olson DB.
)
# Help with unit test labels
pctToString <- function(d) { paste(paste(as.character(d), attr(d, 'tzone')), collapse=';') }

# Creates a POSIXct in the time zone of Canberra
AUest <- function(dateStr) {
  as.POSIXct(dateStr, tz=tzIdR_AUest)
}

pctToUtc <- function(dtPosixct) {
  stopifnot("POSIXct" %in% class(dtPosixct))
  result <- dtPosixct
  attr(result, 'tzone') <- 'UTC'
  result
}

# Given a date ISO 8601 string formatted, check that the marshalling is as expected. 
# Equality tests are done in the CLR, not in R, for this function, so this function tests the R POSIXt to CLR conversion
testRtoClr <- function(dateStr, pfun=as.POSIXct, tzIdR=tzIdR_AUest, tzId=tzId_AUest) {
    
  #### First, a whole day. Test that Date object, then POSIXt objects are converted to the right .NET value
  # Date objects: from R to .NET:
  rdate <- as.Date(dateStr)
  dayComponent <- format(rdate, '%Y-%m-%d')
  # when converting an R Date to a POSIXct it becomes encoded as the date plus 00:00:00 UTC. 
  # Let's check this is the equivalent seen from the CLR
  expect_that( clrDateEquals(rdate, dayComponent, tzIdClr='UTC'), is_true(), label=paste('R Date',rdate,'becomes UTC DateTime',dayComponent));
  
  # if an R POSIXct date is created for a timezone, it is equal to a DateTime time zone
  dr <- pfun(dateStr, tz=tzIdR)
  expect_that( clrDateEquals( dr, dateStr, tzIdClr=tzId ), is_true(), label=paste('R POSIXct',pctToString(dr),'becomes',tzId,'DateTime',dateStr));
}

expect_posixct_equal <- function(actual, expected, mAct='Actual', mExp='Expected') {
  expect_equal(actual, expected, label=paste(mAct,':', pctToString(actual)), expected.label=paste(mExp,':', pctToString(expected)))
}

testDotNetToR <- function(testDateStr) {
  d <- createUtcDate(testDateStr, tzId_AUest)
  dr <- pctToUtc(as.POSIXct(testDateStr, tz=tzIdR_AUest))
  expect_posixct_equal(d, dr, mAct='From CLR', mExp='Expected')

  # The following was the initial intent; however this will take some time to get there, if feasible.
  # clrDate <- createDotNetDate( testDateStr)
  # expect_that( clrDate, equals(as.POSIXct(testDateStr)) );
  # clrDate <- createDotNetDate( testDateStr, 'Local')
  # expect_that( clrDate, equals(as.POSIXct(testDateStr)) );
  # clrDate <- createDotNetDate( testDateStr, 'Utc')
  # expect_that( clrDate, equals(as.POSIXct(testDateStr, tz='UTC')) );
}

testDotNetToRUTC <- function(testDateStr) {
  d <- createUtcDate(testDateStr, 'UTC')
  dr <- pctToUtc(as.POSIXct(testDateStr, tz='UTC'))
  expect_posixct_equal(d, dr, mAct='From CLR', mExp='Expected')
}

testPosixToDotNet <- function(dateStr) {
  testRtoClr(dateStr, pfun=as.POSIXct)
  # demand POSIXct UTC dates only
  expect_error(testRtoClr(dateStr, pfun=as.POSIXlt))
}

testBothDirections <- function(dateStr) {
  testPosixToDotNet(dateStr)
  testDotNetToR(dateStr)
}

# All times in pseudo code for Time zone "Australia/Sydney" in R , identifier "AUS Eastern Standard Time" in .NET TimeZoneInfo (name "Canberra, Melbourne, Sydney")
# change from no DST to DST:
# bidirectional '2013-10-06 01:59' proper conversion then true for every minute for over 62 minutes.
# bidirectional '2013-04-07 01:59' proper conversion then true for every minute for over 62 minutes.
# In the CLR and R respectively, what is: '2013-10-06 03:01' - '2013-10-06 01:59'. In .NET, what is the result for a DateTime that is Local

# Define dates of interest, to iterate over and test

post1971_DateStr <- c('2001-01-01',
  '2001-01-01 23:22:21',
  # Testing proper function around a daylight saving time
  # http://www.timeanddate.com/worldclock/clockchange.html?n=57 
  # Daylight savings time changes in Australia for 2013

  # Sunday, 7 April 2013	2:59:57 AM	+1h	UTC+11h	AEDT
  # 2:59:58 AM	+1h	UTC+11h	AEDT
  # 2:59:59 AM	+1h	UTC+11h	AEDT
  # 3:00:00 AM ? 2:00:00 AM	No	UTC+10h	AEST
  # 2:00:01 AM	No	UTC+10h	AEST
  # 2:00:02 AM	No	UTC+10h	AEST

  # Sunday, 6 October 2013	1:59:57 AM	No	UTC+10h	AEST
  # 1:59:58 AM	No	UTC+10h	AEST
  # 1:59:59 AM	No	UTC+10h	AEST
  # 2:00:00 AM ? 3:00:00 AM	+1h	UTC+11h	AEDT
  # 3:00:01 AM	+1h	UTC+11h	AEDT
  # 3:00:02 AM	+1h	UTC+11h	AEDT
  '2013-10-06 01:59', # DST starts in Canberra
  '2013-10-06 03:01',
  '2013-04-07 01:59',  # DST ends in Canberra. See also notes below for peculiarities over the twice run over 02:00 to 03:00 
  '2013-04-07 02:33',
  '2013-04-07 03:00',
  # Around one of the leap seconds (that is, when the UTC/GMT is tested
  '1994-06-30 23:00:00',
  '1994-06-30 23:59:59',
  '1994-07-01 00:00:00',
  '1994-07-01 00:00:01',
  '1994-07-01 01:00:00',
  '3000-01-01' 
)

# The origin of the POSIXct structure, '1970-01-01 00:00', 'UTC' is zero
posixct_orig_str <- '1970-01-01 00:00:00'

pre1971_DateStr <- c(
  # Around the origin for POSIXct
  '1970-01-01 10:00',
  posixct_orig_str,
  '1970-01-01 01:00:00',
  '1969-12-31 23:00:00',
  # Further ago/ahead
  # First we do want to test around the date of the origin of the VT_DATE thing in the COM world; see
  # http://blogs.msdn.com/b/ericlippert/archive/2003/09/16/eric-s-complete-guide-to-vt-date.aspx
  # Scary, huh?
  '1899-12-29',
  '1899-12-29 23:59:59',
  '1899-12-30',
  '1899-12-30 00:00:01',
  '1899-12-31',
  '1789-07-14', 
  '0200-01-01'
)

problem_DateStr <- c(
  '0020-01-01', # MS CLR hosting method invokation fails if such a DateTime is returned. 
  '0001-01-01' # MS CLR hosting in C shows an OLEAUT VT_DATE of numeric value 0.0. Supposed to be for 1899-12-30
 )
 
testDatesStr <- c(post1971_DateStr, pre1971_DateStr)

testSameInteger <- function(datestr) {
  expect_equal( as.integer(createUtcDate(datestr, 'UTC')), as.integer((as.POSIXct(datestr, tz='UTC'))))
}

# See issue #45; I do not think this could be passing but by chance
testRtoClrAestTz <- function(dateStr) {
  testRtoClr(dateStr, pfun=as.POSIXct, tzIdR=tzIdR_AUest, tzId=tzId_AUest)
}

testRtoClrNoTz <- function(dateStr) {
  testRtoClr(dateStr, pfun=as.POSIXct, tzIdR="", tzId="")
}

testRtoClrUtc <- function(dateStr) {
  testRtoClr(dateStr, pfun=as.POSIXct, tzIdR='Utc', tzId='UTC')
}


# testDotNetToR('2013-04-07 02:32')
