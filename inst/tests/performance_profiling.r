# library(utils)
library(ggplot2)
# Sys.setenv(RCLR='Mono')

library(rClr)

profClassName <- 'Rclr.PerformanceProfiling'
prof <- clrNew(profClassName)
nRepsColname <- 'numReps'
arrayLenColname <- 'arrLen'

clrToRDataTransferFUNGEN <- function(numArray, arrLen) {
  stopifnot(length(numArray) == 1)
  clrCall(prof, 'SetDoubleArrays', as.integer(0), as.integer(arrLen), as.integer(numArray))
  clrToRDataTransfer <- function() { clrCall(prof, 'GetNextArrayDouble') }
  clrToRDataTransfer
}

rToClrDataTransferFUNGEN <- function(numArray, arrLen) {
  set.seed(0)
  if(arrLen <= 1) {stop("Not designed to cope with array length less than two")}
  num_vec = rnorm(arrLen)
  rToClrDataTransfer <- function() { clrCall(prof, 'CallMethodWithArrayDouble', num_vec) }
  rToClrDataTransfer
}

sw <- clrNew('System.Diagnostics.Stopwatch')
startSw <- function () { clrCall(sw,'Stop'); clrCall(sw,'Reset'); clrCall(sw, 'Start') }
stopSw <- function () { clrCall(sw,'Stop'); clrCallStatic('Rclr.PerformanceProfiling', 'GetElapsedSeconds', sw) }

measure <- function(numReps, FUN, normalize=TRUE) {
  blah = numeric(0)
  if(numReps>1) {
    startSw()
    for (i in 1:numReps) {
      blah <- FUN()
    }
    e = stopSw()
    delta = e
    # Fiendish: the for() construct is surprisingly expensive compared to rClr...
    startSw()
    for (i in 1:numReps) {
      blah <- 0
    }
    e = stopSw()
    delta = delta - e
  } else {
    startSw()
    blah <- FUN()
    e = stopSw()
    delta = e
    # Remove the measurement.
    startSw()
    e = stopSw()
    delta = delta - e
  }
  delta <- as.numeric(delta)
  if(normalize) {delta <- delta / normalize}
  delta
}

doMeasure <- function(trials, trow, FUNGEN, dataClass,direction,tag=NA) {
  nReps <- trials[trow,nRepsColname]
  innerReps <- 1
  arrLen <- trials[trow,arrayLenColname]
  # if(arrLen < 1000) {
    # innerReps <- 100
  # } else if (arrLen < 10000) {
    # innerReps <- 50
  # # } else if (arrLen < 100000) {
    # # innerReps <- 10
  # }  
  FUN = FUNGEN(nReps, arrLen)
  deltas <- numeric(0)
  for (i in 1:nReps) {
    deltas <- c(deltas, measure(numReps=innerReps, FUN=FUN))
  }
  data.frame(dataClass=dataClass, arrayLen=as.integer(arrLen),direction,tag=as.character(tag), delta=deltas)
}

doMeasureFun <- function(trials, FUNGEN, dataClass,direction,tag=NA) {
  clrCallStatic('Rclr.TestCases', 'CallGC') # minimize the risk of cases such that we end up with negative runtimes...
  res <- doMeasure(trials, 1, FUNGEN=FUNGEN, dataClass, direction, tag)
  for (trow in 2:nrow(trials)) {
    res <- rbind(res, doMeasure(trials, trow, FUNGEN=FUNGEN, dataClass, direction, tag))
  }
    # dataClass arrayLen direction      tag       delta
# 1    numeric         1    CLR->R no.r.net 0.005999804
# 2    numeric         1    CLR->R no.r.net 0.004001141
  res$rate <- res$arrayLen / res$delta # items/second
  res
}

# We want to obtain plots of effective transfer rates: X axis is the length of the arrays, Y axis is the transfer rate in MB/s
# we may want to measure with/without R.NET loaded, so there is at least one category. 
# We may as well have a single data frame for 
# So the high-level result from the measurement should be a data frame with columns:
# dataClass,array_len,direction,tag
# 'integer',5000,'CLR->R',no.r.net

# repetitions,array_len

createCases <- function(numReps=10, maxArrayLen=7.5e6) {
  # May want to tailor to whether this is 32 or 64 bits; use:
  # clrGet('System.Environment', 'Is64BitProcess')
  cases <- c(2,5,7.5)
  trials <- expand.grid ( numReps = numReps, arrLen = as.integer(cases) )
  mult <- c(1,2,5,7.5)
  cases <- expand.grid(mult, 10**(1:7))
  cases <- as.integer(cases[,1]*cases[,2]) # 1 2 5 10 20 50 etc.
  trials <- rbind(trials, expand.grid ( numReps = numReps, arrLen = cases ))
  return(subset(trials, arrLen <= maxArrayLen))
}


rclrNumericPerf <- function(trials, tag) {
  fgen <- clrToRDataTransferFUNGEN
  bench <- doMeasureFun(trials, FUNGEN=fgen, dataClass='numeric', direction="CLR->R",tag=tag)
  bench <- rbind(bench, doMeasureFun(trials, FUNGEN=fgen, dataClass='numeric', direction="R->CLR",tag=tag))
  bench
}

rclrNumericPerfAll <- function(numReps=10, maxArrayLen=7.5e6) {
  trials <- createCases(numReps=numReps, maxArrayLen=maxArrayLen)
  setRDotNet(FALSE)
  bench <- doMeasureFun(trials, FUNGEN=clrToRDataTransferFUNGEN, dataClass='numeric', direction="CLR->R",tag="no.r.net")
  bench <- rbind(bench, doMeasureFun(trials, FUNGEN=rToClrDataTransferFUNGEN, dataClass='numeric', direction="R->CLR",tag="no.r.net"))
  setRDotNet(TRUE)
  bench <- rbind(doMeasureFun(trials, FUNGEN=clrToRDataTransferFUNGEN, dataClass='numeric', direction="CLR->R",tag="r.net"))
  bench <- rbind(bench, doMeasureFun(trials, FUNGEN=rToClrDataTransferFUNGEN, dataClass='numeric', direction="R->CLR",tag="r.net"))
}

#####################################
# Plotting
plotRate <- function(bench, case = 'R->CLR', dataType='numeric', logy=TRUE) {
  bench <- subset(bench, direction==case)
  p <- qplot(x=bench$arrayLen, y=bench$rate) +
    ylab('Items/sec') + xlab('Array size') +
    ggtitle(paste(dataType, 'vector conversion rate', case))
  p <- p + scale_x_log10() 
  if(logy) {
    p <- p + scale_y_log10() + annotation_logticks(sides='bl')
  } else {
    p <- p + annotation_logticks(sides='b')
  }
  p
}
#####################################


# options(error=recover)
# trials <- createCases(numReps=10, maxArrayLen=7.5e6)
# bench <- rclrNumericPerf(trials, tag='no.r.net')

## REPRO: this crashes Rgui. All in R.dll, and likely related to R.NET; unclear where/why
# behaves for a length of 100000; above, trouble start.
# trials <- createCases(maxArrayLen=50000)
# setRDotNet(TRUE)
# bench <- doMeasureFun(trials, FUNGEN=clrToRDataTransferFUNGEN, dataClass='numeric', direction="CLR->R",tag="r.net")
## bench <- rbind(bench, doMeasureFun(trials, FUNGEN=rToClrDataTransferFUNGEN, dataClass='numeric', direction="R->CLR",tag="r.net"))

#bench$measure = apply(measures, MARGIN=1,FUN=mean)
# plotRate(bench, case = 'CLR->R')
#  + scale_y_log10() + annotation_logticks()
# plotRate(bench, case = 'CLR->R') + annotation_logticks(sides='b')
# 
# bench <- doMeasureFun(trials, FUNGEN=rToClrDataTransferFUNGEN, dataClass='numeric', direction="R->CLR",tag="no.r.net")
# plotRate(bench, case = 'R->CLR') + scale_y_log10() + annotation_logticks()


# With the MS implementation (x64):
# > rate
 # [1]   103785.2   204040.8   370295.1   463226.1   595119.1   702986.7   769077.6   784153.1   964098.6  1034270.3  2157841.3  4490122.2
# [13]  8062899.1 25856885.5 34571239.5 36067822.5 36660757.6

# With the MS implementation (R i386):

#and the winner  Mono (on i386):
