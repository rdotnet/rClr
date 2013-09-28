library(utils)

library(testthat)

# Sys.setenv(RCLR='Mono')

library(rClr)
# Load the library with the C# part of things.

# context("rClrTestCases")

# cTypename <- "Rclr.TestCases"
# testClassName <- "Rclr.TestObject";
# setRDotNet(TRUE)
# rproffn <- 'f:/tmp/rproftests.txt'
# Rprof(NULL)

# Rprof(filename = rproffn, append=TRUE)

# for (i in 1:1000) {
  # expect_that( clrCallStatic(cTypename, "DoubleEquals", 123.0 ), is_true() );
  # expect_that( clrCallStatic(cTypename, "CreateDouble"), equals(123.0) );
# }
  
# Rprof(NULL)

# summaryRprof(rproffn)

profClassName <- 'Rclr.PerformanceProfiling'

prof <- clrNew(profClassName)

numArrays <- 100
# arrLen <- 10000
nIter <- 100

trials <- expand.grid ( numArrays = numArrays, arrLen = c(1:10, 20, 50, 100, 1000, 5000, as.integer(1:5*1e4)) )

s = Sys.time()
for (i in 1:(numArrays*nIter)) {
  blah <- clrCall(prof, 'DoNothing')
}
e = Sys.time()
methCallDelta = (e-s)

clrToRDataTransferFUNGEN <- function(numArray, arrLen) {
  clrCall(prof, 'SetDoubleArrays', as.integer(0), as.integer(arrLen), as.integer(numArray))
  clrToRDataTransfer <- function() { clrCall(prof, 'GetNextArrayDouble') }
  clrToRDataTransfer
}

rToClrDataTransferFUNGEN <- function(numArray, arrLen) {
  set.seed(0)
  num_vec = rnorm(arrLen)
  rToClrDataTransfer <- function() { clrCall(prof, 'CallMethodWithArrayDouble', num_vec) }
  rToClrDataTransfer
}

measure <- function(numArray, arrLen, FUNGEN) {
  blah = numeric(0)
  FUN = FUNGEN(numArray, arrLen)
  s = Sys.time()
  for (i in 1:(numArray*nIter)) {
    blah <- FUN()
  }
  e = Sys.time()
  delta = (e-s)
  as.numeric(delta)
}

doMeasure <- function(FUNGEN) {
  deltas <- numeric(0)
  for (trow in 1:nrow(trials)) {
    deltas <- c(deltas, measure(trials[trow,1], trials[trow,2], FUNGEN=FUNGEN))
  }
  deltas
}

nRep = 3

doMeasureFun <- function(FUNGEN) {
  measures <- data.frame(rep_1 = doMeasure(FUNGEN=FUNGEN))

  for (j in 2:nRep) {
    measures <- cbind(measures, doMeasure(FUNGEN=FUNGEN))
  }
  names(measures) <- paste( 'rep_', 1:nRep, sep='')
  bench = trials
  bench$measure = apply(measures, MARGIN=1,FUN=mean)
  bench
}

getConversionRate <- function(bench) {
  callsPerTrial = nIter * bench$numArrays
  numNumbersPassed = callsPerTrial * bench$arrLen
  unitCost = (bench$measure-methCallDelta) / numNumbersPassed # time/number ~ time
  rate = 1/as.numeric(unitCost) # number or doubles/sec without the overhead of the method call.
  rate
}

plotRate <- function(bench, rate, case = 'R->CLR') {
  qplot(x=bench$arrLen, y=rate) + scale_x_log10() + ylab('MBytes/sec') + xlab('Array size') + ggtitle(paste('Numeric vector conversion rate', case))
}

bench <- doMeasureFun(FUNGEN=clrToRDataTransferFUNGEN)
rate <- getConversionRate(bench)
plotRate(bench, rate, case = 'CLR->R')

bench <- doMeasureFun(FUNGEN=rToClrDataTransferFUNGEN)
rate <- getConversionRate(bench)
plotRate(bench, rate, case = 'R->CLR')



# With the MS implementation (x64):
# > rate
 # [1]   103785.2   204040.8   370295.1   463226.1   595119.1   702986.7   769077.6   784153.1   964098.6  1034270.3  2157841.3  4490122.2
# [13]  8062899.1 25856885.5 34571239.5 36067822.5 36660757.6

# With the MS implementation (R i386):

#and the winner  Mono (on i386):
