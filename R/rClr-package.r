
## Note to self: see the roxygen vignette for examples

#' Accessing the Common Language Runtime (.NET/Mono) from R
#' 
#' \tabular{ll}{
#' Package: \tab rClr\cr
#' Type: \tab Package\cr
#' Version: \tab 0.6-4\cr
#' Purpose: \tab Major fix to memory management issue across the R/C++/.NET interop chain. Issue https://rclr.codeplex.com/workitem/33 appears resolved. \cr
#' Date: \tab 2014-11-13\cr
#' License: \tab LGPL 2.1\cr
#' }
#'
#' A low-level, in-process interoperability package between R 
#' and a Common Language Runtime (.NET or Mono). 
#' The supported CLR implementations are Microsoft '.NET' framework on Windows and Mono 
#' on several platforms, currently Windows and Debian Linux. rClr has been used for 
#' production work as of 2013-06-16. Running it on Mono is possible, though less mature than on MS.NET.
#' The approach is similar to that of rJava.
#'
#' Kosei Abe (https://www.codeplex.com/site/users/view/kos59125) is gratefully 
#' acknowledged as the author of the R.NET C# library, reused by this package.
#'
#' @name rClr-package
#' @aliases rClr
#' @docType package
#' @title R accessing .NET/Mono 
#' @author Jean-Michel Perraud \email{jean-michel.perraud_at_csiro.au}
#' @keywords package CLR Mono .NET
NULL



