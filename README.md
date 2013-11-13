rClr
====

R package for accessing .NET

Accessing the Common Language Runtime (.NET or Mono) from the R statistical software, in-process.

# Installing

## Windows packages

You can install windows binaries of the latest version at [https://r2clr.codeplex.com](https://r2clr.codeplex.com). Note that the side is migrating to [https://rclr.codeplex.com](https://rclr.codeplex.com), and you may find a newer build at the new address.

## From source

rClr is not your average R package and requires a C# and C and/or Visual C++ compiler. Read [the current build instructions](https://r2clr.codeplex.com/wikipage?title=Build%20instructions&referringTitle=Documentation)

To install from source you can either clone, or use the package `devtools` to access the mirror on github:

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

You will find some material at [https://r2clr.codeplex.com/documentation](https://r2clr.codeplex.com/documentation)

# Related work

A couple of packages using rClr are publically accessible:

* If you are interested in environmental modelling: [RtoTIME](https://github.com/jmp75/RtoTIME) is a package that depends on rClr
* [rsqlserver](https://github.com/agstudy/rsqlserver) is an Sql Server driver database interface (DBI) driver for R
