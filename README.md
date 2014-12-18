rClr
====

R package for accessing .NET

Accessing the Common Language Runtime (.NET or Mono) from the R statistical software, in-process.

# Installing

## Windows packages

You can find windows binaries and/or source packages for Linux of the latest version at [https://rclr.codeplex.com](https://rclr.codeplex.com).

## From source

rClr is not your average R package and requires a C# and C and/or Visual C++ compiler. Read [the current build instructions](https://r2clr.codeplex.com/wikipage?title=Build%20instructions&referringTitle=Documentation)

To install from source, as of December 2014 you very probably need to clone or download a tarball of this site. 

You might be able use the `install_github` function of the package `devtools`, however this may not work yet due to the NuGet package dependencies of rClr (downloading them automatically may be a machine specific feature anyway)

```S
library(devtools)
install_github("rClr", username='jmp75')
```

# Getting started

```S
library(rClr)
?rClr
# There is an HTML vignette:
browseVignettes('rClr')
```

You will find some documentation at [https://r2clr.codeplex.com/documentation](https://r2clr.codeplex.com/documentation)

# Related work

A few packages using rClr are publicly accessible, and may be of interest if you want to build your own package with dependencies on rClr :

* If you are interested in environmental modelling: [RtoTIME](https://github.com/jmp75/RtoTIME) is a package that depends on rClr
* [rsqlserver](https://github.com/agstudy/rsqlserver) is an Sql Server driver database interface (DBI) driver for R
* [A package to access optimization tools on .NET](https://github.com/jmp75/metaheuristics/tree/master/R/pkgs/mh)