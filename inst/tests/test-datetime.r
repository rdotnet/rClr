testDir <- system.file('tests', package='rClr') 
stopifnot(file.exists(testDir))
source(file.path(testDir, 'load_libs.r'))
source(file.path(testDir, 'datetime-functions.r'))

context("Date and times")

test_that("Date and Time objects are marshalled correctly", {
# TODO:  https://r2clr.codeplex.com/workitem/52 

########################################################
# handling date, time and time zone conversions in software is complicated. Doing it naively can quickly lead to nightmarish situation, so let's clarify the essential early, and determine the unambiguous behavior to expect in converting date and time information between R and the CLR.
# Dates, Times, and Time Zones: http://msdn.microsoft.com/en-us/library/bb384268.aspx
# There are also useful guildelines in Choosing Between DateTime, DateTimeOffset, and TimeZoneInfo [http://msdn.microsoft.com/en-us/library/bb384267.aspx]
# http://www.r-project.org/doc/Rnews/Rnews_2001-2.pdf
# To get information on how R handles time zones: [?Sys.timezone]

# DateTime objects in .NET can have a specified Kind: Unspecified, UTC or Local. DateTime.Now.Kind will be Local, but DateTime(2001,1,1).Kind is Unspecified. Methods converting to UTC seem to assume that Unspecified means Local, however. 
# "The DateTimeOffset structure represents a date and time value, together with an offset that indicates how much that value differs from UTC. Thus, the value always unambiguously identifies a single point in time." This seems interesting, but there is no direct, clear equivalent in R

# Time Zone information
# --------------------
# Both POSIXlt and POSIXct objects can have a tzone attribute. No tzone info seems to be treated as 'local time'. Note that 
# Conversion in .NET between time zones should be done using TimeZoneInfo objects.

# ------------
# origUtc <- as.POSIXct('1970-01-01',tz='UTC')
# as.POSIXlt(origUtc)
# as.POSIXlt(origUtc, Sys.timezone())
# # [1] "1969-12-31 19:00:00 EST" I am in Canberra, Australia: AUS EST. What's returned looks more like New York time then
# ------------
# Considering the (1)inherent complication of time zone handling, (2)Unexpected behaviors in R (or my underlying OS) and (3) likely mismatches of time zone handling behaviors between R and .NET, time zones will have only limited support in initial versions of rClr. Basically, UTC (GMT) or Unspecified (empty character). Push back the conversion of R POSIXt objects to the user. I think I can handle the conversion of DateTimes and DateTimeOffset in .NET to UTC DateTime objects.

# Other References:
# http://www.codeproject.com/Articles/144159/Time-Format-Conversion-Made-Easy
########################################################

  # First, a few functions in IronPython and R to test the respective behaviors of date and time
  ########################
  # Start of IronPython test code:
  ########################
# import clr
# from System import *

# def dateKind(dtstr, kind):
    # d = DateTime.Parse(dtstr)
    # return DateTime(d.Ticks, kind)

# def utcDate(dtstr):
    # return dateKind(dtstr, DateTimeKind.Utc)

# def localDate(dtstr):
    # return dateKind(dtstr, DateTimeKind.Local)

# def toUtc(dt):
    # return dt.ToUniversalTime()

# def toLocal(dt):
    # return dt.ToLocalTime()

# def getSeconds(tspan): return tspan.Ticks / 1.0e7

# def f(d) : return d.ToString('yyyy-MM-dd HH:mm:ss')

# datestr='2013-10-06 '

# t2 = localDate(datestr + ' 03:01')
# t1 = localDate(datestr + ' 01:59')
# getSeconds(t2 - t1) # 3720.0
# getSeconds(toUtc(t2) - toUtc(t1)) # 120 seconds

# f(toUtc(t2)) # 5/10/2013 4:01:00 PM
# f(toUtc(t2.AddMinutes(-30))) # during the missing hour. Returns 5/10/2013 4:31:00 PM . Interesting.
# f(toUtc(t2.AddMinutes(-62))) # actualy 2 minutes before in UTC, as expected: 5/10/2013 3:59:00 PM

# d = DateTime(2000,01,01)
# d.Kind
# f(d)
# f(toUtc(d)) # Looks like unspecified is assumed as a Local if ToUtc is called, and vice-versa 
# f(toLocal(d))

# HOWEVER: consider the following with Mono 3.0.9
# Trouble in TimeZoneInfo, and this looks like a Mono issue (several bugzilla entries too around TimeZoneInfo)
# "c:\Program Files\Mono-3.0.9\bin\mono.exe" "c:\Program Files\IronPython 2.7\ipy.exe" -X:TabCompletion -X:ColorfulConsole
# clr.AddReferenceToFileAndPath(r'C:\Rlib\rClr\libs\RDotNetDataConverter.dll')
# clr.AddReferenceToFileAndPath(r'C:\Rlib\rClr\libs\ClrFacade.dll')
# from Rclr import *

# from System import DateTime, TimeZoneInfo

# datestr='2013-10-06'
# tzId_AUest = "AUS Eastern Standard Time"
# isoDateTime = datestr + ' 01:59' # One minute before DST kicks in

# tz = TimeZoneInfo.FindSystemTimeZoneById(tzId_AUest)
# dtun = DateTime.Parse(isoDateTime)
# dtUtc = TimeZoneInfo.ConvertTimeToUtc(dtun, tz)

# dtun = DateTime.Parse(datestr + ' 10:59' )
# dtUtc = TimeZoneInfo.ConvertTimeToUtc(dtun, tz)

# dtun = DateTime.Parse('2007-01-01 02:00')
# dtUtc = TimeZoneInfo.ConvertTimeToUtc(dtun, tz)


  ########################
  # End of IronPython test code:
  ########################

  ########################
  # Start of R test code:
  ########################

  # printDetail <- function(strdt) { unclass(as.POSIXlt(strdt, tz="Australia/Sydney")) }
  # printDetail('2013-10-06 02:00')
  # printDetail('2013-10-06 02:01')
  # printDetail('2013-10-06 02:59')
  # printDetail('2013-10-06 03:01')
  # printDetail('2013-04-07 03:01')
  # # Looking at the times that are passed twice through:
  # printDetail('2013-04-07 02:59') #flagged not DST
  # printDetail('2013-04-07 02:01') #flagged DST
  # printDetail('2013-04-07 02:32') #flagged not DST
  # printDetail('2013-04-07 02:33') #flagged DST

  ########################
  # End of R test code:
  ########################
  
  
  ##########
  # .NET to R
  ##########

  # First, test around the origin of the POSIXct structure, '1970-01-01 00:00', 'UTC' is zero
  testSameInteger(posixct_orig_str)
  testSameInteger('1970-01-01 01:00:00')
  testSameInteger('1969-12-31 23:00:00')
  
  # test around two daylight savings dates for the test time zone eastern Australia
  # DST skip one hour
  testDotNetToR('2013-10-06 01:59')
  expect_error(testDotNetToR('2013-10-06 02:30')) # TimeZoneInfo complains, rightly so.
  # Note for information that in R:
  # > pctToUtc(as.POSIXct('2013-10-06 02:30', tz=tzIdR_AUest))
  # [1] "2013-10-05 14:00:00 UTC"
  testDotNetToR('2013-10-06 03:01')

  # DST go back one hour; 02:00 to 03:00 happens twice for same date
  testDotNetToR('2013-04-07 01:59')
  # from 02:00 to around 02:32, the CLR base class library and R behave differently when converting to UTC. So be it.
  
  # FIXME:
  #  Oddly, the following  expect_error() is behaving as expected if run from R, however inside a 
  # test_that function, this fails twice (test, and then that the expected error is nevertheless not 'detected'
  # expect_error(testDotNetToR('2013-04-07 02:32'))
  
  testDotNetToR('2013-04-07 02:33') #from then on same UTC date is returned.
  testDotNetToR('2013-04-07 03:00')

  # we can only test local date times post sometime in 1971 - DST rules for AU EST differ prior to that.
  # Further illustration: consider the output of the following 5 lines, it looks like there is no DST 
  # for summer 1971 (meaning January, down under); only kicks in in 1972.
  # pctToUtc(AUest('1970-01-01 11:00'))
  # pctToUtc(AUest('1971-01-01 11:00'))
  # pctToUtc(AUest('1971-07-01 11:00'))
  # pctToUtc(AUest('1972-01-01 11:00'))
  # pctToUtc(AUest('1972-07-01 11:00'))

  pDate <- function(dateStr) {
    if(exists('debug_test') && debug_test) { print(paste('testing', dateStr)) }
  }
  
  for ( dateStr in post1971_DateStr ) {
    testDotNetToR(dateStr)
    pDate(dateStr)
  }

  # This however must work for all test dates
  for ( dateStr in testDatesStr ) {
    testDotNetToRUTC(dateStr)
    pDate(dateStr)
  }

  # Test that DateTime[] becomes POSIXt vectors of expected length
  secPerDay <- 86400
  testDateStr='2001-01-01'

  testDateSeq <- function(startDateStr, numSeq) {
    dateSeq <- as.POSIXct(startDateStr)+numSeq
    d <- createUtcDate(as.character(dateSeq), tzId_AUest)
    dr <- pctToUtc(dateSeq)
    expect_posixct_equal(d, dr, mAct='From CLR', mExp='Expected')
  }
  numDays = 5;
  testDateSeq(testDateStr, (0:(numDays-1))*secPerDay);
  numSecs = 42;
  testDateSeq(testDateStr,(0:(numSecs-1)))

  # Time spans: .NET to R
  # TimeSpan were not handled gracefully. Following tests also check that https://r2clr.codeplex.com/workitem/52 is dealt with
  # TODO broader, relating to that: default unknown value types are handled gracefully
  threePfive_sec <- as.difftime(3.5, units='secs')
  expect_equal( clrCallStatic('System.TimeSpan','FromSeconds', 3.5), expected = threePfive_sec)
  threePfive_min <- as.difftime(3.5, units='mins')
  expect_equal( clrCallStatic('System.TimeSpan','FromMinutes', 3.5), expected = as.difftime(180+30, units='secs'))
  # arrays of timespan
  expect_equal( clrCallStatic(cTypename, "CreateTimeSpanArray", 3.5, as.integer(5)), expected = threePfive_sec + 5*(0:4))

  ##########
  # R to .NET conversions
  ##########

  testRtoClrNoTz('1980-01-01')
  testRtoClrNoTz('1972-01-01')
  
  # FIXME: expect-error lines pass if run interactively but not if inside a test_that() function call
  # However, there is this DST discrepancy of one hour creeping in sometime in 1971, and before that as well
  # expect_error(testRtoClrNoTz('1971-01-01'))
  # expect_error(testRtoClrNoTz(posixct_orig_str))

  # we can only test local date times post sometime in 1971 - DST rules for AU EST differ prior to that.
  for ( dateStr in post1971_DateStr ) {
    testRtoClrNoTz(dateStr)
    pDate(dateStr)
  }

  # This however must work for all test dates
  for ( dateStr in testDatesStr ) {
    testRtoClrUtc(dateStr)
    pDate(dateStr)
  }

  # The following lines may look puzzling but really do have a purpose.
  # Check  that http://r2clr.codeplex.com/workitem/37 has been fixed The following will crash if not.
  x <- as.Date(testDateStr)
  clrType = clrCallStatic('Rclr.ClrFacade', 'GetObjectTypeName', x)
  expect_that( x, equals(as.Date(testDateStr)) );
  x <- as.Date(testDateStr) + 0:3
  clrType = clrCallStatic('Rclr.ClrFacade', 'GetObjectTypeName', x)
  expect_that( x[1], equals(as.Date(testDateStr)) );
  expect_that( x[2], equals(as.Date(testDateStr)+1) );
  # End check http://r2clr.codeplex.com/workitem/37

  #### daily sequences
  # Dates
  testDateStr='2001-01-01'; numDays = 5;
  # Note that there no point looking below daily period wuth Date objects. consider:
  # z <- ISOdate(2010, 04, 13, c(0,12)) 
  # str(unclass(as.Date(z)))
  expect_true( clrCallStatic(cTypename, "CheckIsDailySequence", as.Date(testDateStr) + 0:(numDays-1), testDateStr, as.integer(numDays)))
 
   # Time spans: R to .NET
  # This seems trickier, at least with the MS CLR hosting API.
  # TODO
  # Basically, I don't know how I can create a TimeSpan from the C layer, since the hosting API will fail to find a suitable COM type for it. 
  # expect_true( clrCallStatic(cTypename, "TimeSpanEquals", threePfive_min, '00:03:30.00'))
 
 
  # further notes, summary, thoughts to elaborate:
  # in .NET DateTime objects of the same Kind (what if different Kind?) are such that one day is always 86400 seconds.
  # In R, a POSIXt object with no time zone information is such that a day length is not 86400 seconds if in a local time zone with daylight savings
  # .NET has objects DateTimeOffset and TimeZoneInfo to deal more strictly with date-time information worldwide.
  # R uses the [TBC] time zone database with identifiers that are different than the ones in .NET/Windows
  
  # ----
  # | R | .NET | Remarks |
  # | POSIXct with tzone = 'UTC' or GMT | Same DateTime with Kind Utc | Easy in C |
  # | POSIXlt with tzone = 'UTC' or GMT | Same DateTime with Kind Utc | More tedious than POSIXct but same as no time zone, below |
  # | POSIXct with tzone = '' | Same DateTime with Kind Local | Very difficult to get right in C due to non-linear scale in R representation; locale dependent |
  # | POSIXlt with tzone = '' | Same DateTime with Kind Local | Decomposition alleviates the DST issue, but is more code in C |
  # | Date | Same DateTime | time component 00:00:00 ; Kind Local or Unspecified? |
  
  # | POSIXct with specific tzone = e.g. 'Australia/Sydney' | DateTimeOffset? |  |
  # | POSIXlt with specific tzone = e.g. 'Australia/Sydney' |  |  |
  
  # | .NET | R | Remarks |
  # | DateTime with Kind Utc | POSIXct or POSIXlt with tzone = 'UTC' | |
  # | DateTime with Kind Local | POSIXct or POSIXlt with tzone = '' |  |
  # | DateTime with Kind Unspecified | POSIXct or POSIXlt with tzone = '' | Check what happens on an Unspecified DateTime with ToUniversalTime/ToLocalTime |
  
  # | POSIXct with specific tzone = e.g. 'Australia/Sydney' | DateTimeOffset? |  |
  # | POSIXlt with specific tzone = e.g. 'Australia/Sydney' |  |  |

    
  ###########################
  # Notes on leap seconds (.leap.seconds)
  ###########################
  # I was not sure what was happening with leap seconds. Well, it seems every day is 86400 seconds regardless in both systems. Simpler, but I am curious as to when/where leap seconds should be accounted for. Be aware, but dont seek complications. 
  # in R
  # as.numeric(as.POSIXct('2012-07-01 12:00:00', "GMT")) - as.numeric(as.POSIXct('2012-06-30 12:00:00', "GMT"))
  # as.numeric(as.POSIXct('2012-07-02 12:00:00', "GMT")) - as.numeric(as.POSIXct('2012-07-01 12:00:00', "GMT"))
  # .NET (via IronPython)
  # getSeconds(utcDate('2012-07-01 12:00:00') - utcDate('2012-06-30 12:00:00'))
  # getSeconds(utcDate('2012-07-02 12:00:00') - utcDate('2012-07-01 12:00:00'))
  ###########################
  

})
