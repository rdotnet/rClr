# rClr

## R package for accessing .NET

Accessing the Common Language Runtime (.NET or Mono) from the R statistical software, in-process.

## Prerequisites

As of May 2019, on Windows Clr requires the .NET Framework (4.6.1+, 4.7.2+ recommended). Groundwork towards running on .NET Core started but it is unclear how to embed the .NET Core runtime into R.

As of September 2015 using Mono on Windows is not maintained.

## Installing

As of 2019-04, releases can be found via the [release tab of the rClr GitHub repository](https://github.com/rdotnet/rClr/releases).

### Pre-compiled binaries

You can install pre-compiled rClr for Windows via the [release tab of the rClr GitHub repository](https://github.com/rdotnet/rClr/releases). May 2019 has binary packages for R 3.4 and 3.5. August 2020 has a package for version 4.0.x. 

`7z a rClr_windows_pkgs.7z` then:

* `cd R_pkgs\bin\windows\contrib\3.5\` for R 3.5.x
* `cd R_pkgs\bin\windows\contrib\4.0\` for R 4.0.x

You can use from the command line `R CMD INSTALL rclr_0.8.zip` where `R` points to one of the R.exe installed on your machine, or from R itself `install.packages('c:/path/to/rclr_0.8.zip')`

### From source

If you want to compile from source, you may be interested in using the [testing branch](https://github.com/rdotnet/rClr/tree/testing).

rClr is not your average R package and requires a few more tools than is typical for most R packages.

#### Windows

On Windows you will need a C# and C and/or Visual C++ compiler. Using the Visual Studio 2019 toolchain is recommended as of April 2019. Check the [visual studio download page](https://visualstudio.microsoft.com/downloads/) for options.

Under construction as of 2019-04:

You should work from the windows command prompt. I normally use and love ConEmu but for odd reasons the package install fails with `/Rtools/bin/sh: ./configure.win: No such file or directory`.

```bat
REM IMPORTANT to not have nuget.exe or other commands under c:\bin; RTools mingw cannot find these commands
set PATH=C:\cmd_bin;%PATH%
set SRC_ROOT=c:\src\github_jm

cd %SRC_ROOT%
rm rClr*.zip rClr*.tar.gz

set PKG_VERSION=0.9.0
```

Make sure we use a recent msbuild, otherwise there may be issues with targetting .NET netstandard2.0. `.\rClr\src\setup_vcpp.cmd` may help detect the most recent `msbuild.exe` you have if you have installed visual studio (unsure it works for Build Tools for Visual Studio). You can start a development prompt as a fallback if setup_vcpp fails to work on your machine.

```bat
REM https://github.com/rdotnet/rClr/issues/42 may need to use VS2019
.\rClr\src\setup_vcpp.cmd
```

After that `where msbuild` returns e.g. `C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe`

Optionally to `roxygenize` the package, launch R but from the same command prompt e.g.:

```bat
REM set R_EXE="c:\Program Files\R\R-3.5.2\bin\x64\R.exe"
set R_EXE="c:\Program Files\R\R-4.0.2\bin\x64\R.exe"
set R_VANILLA=%R_EXE% --no-save --no-restore-data

%R_VANILLA%
```

```R
library(devtools)
install_github("jmp75/rclr-devtools/packages/rClrDevtools")
library(rClrDevtools) # https://github.com/rdotnet/rClr-devtools
rClrDevtools::roxyRclr('c:/src/github_jm/rClr')
# possibly, to build vignettes
install.packages(c('knitr', 'xtable', 'formatR'))
```

back to windows cmd, to build the tarball. 

```bat
%R_VANILLA% CMD build rClr
```

```bat
set R_REPO_DIR=c:\build\software\R_pkgs\
```

```bat
set R_WINBIN_REPO_DIR=%R_REPO_DIR%bin\windows\contrib\4.0\
if not exist %R_WINBIN_REPO_DIR% mkdir %R_WINBIN_REPO_DIR%
set R_EXE="c:\Program Files\R\R-4.0.2\bin\x64\R.exe"
set R_VANILLA=%R_EXE% --no-save --no-restore-data
cd %R_WINBIN_REPO_DIR%
rm *
%R_VANILLA% CMD INSTALL --build %SRC_ROOT%\rClr_%PKG_VERSION%.tar.gz
```

and to build for R 3.x:

```bat
set R_WINBIN_REPO_DIR=%R_REPO_DIR%bin\windows\contrib\3.6\
if not exist %R_WINBIN_REPO_DIR% mkdir %R_WINBIN_REPO_DIR%
set R_EXE="c:\Program Files\R\R-3.6.1\bin\x64\R.exe"
set R_VANILLA=%R_EXE% --no-save --no-restore-data
cd %R_WINBIN_REPO_DIR%
rm *
%R_VANILLA% CMD INSTALL --build %SRC_ROOT%\rClr_%PKG_VERSION%.tar.gz

set R_WINBIN_REPO_DIR=%R_REPO_DIR%bin\windows\contrib\3.5\
if not exist %R_WINBIN_REPO_DIR% mkdir %R_WINBIN_REPO_DIR%
set R_EXE="c:\Program Files\R\R-3.5.3\bin\x64\R.exe"
set R_VANILLA=%R_EXE% --no-save --no-restore-data
cd %R_WINBIN_REPO_DIR%
rm *
%R_VANILLA% CMD INSTALL --build %SRC_ROOT%\rClr_%PKG_VERSION%.tar.gz

set R_WINBIN_REPO_DIR=%R_REPO_DIR%bin\windows\contrib\3.4\
if not exist %R_WINBIN_REPO_DIR% mkdir %R_WINBIN_REPO_DIR%
set R_EXE="c:\Program Files\R\R-3.4.4\bin\x64\R.exe"
set R_VANILLA=%R_EXE% --no-save --no-restore-data
cd %R_WINBIN_REPO_DIR%
rm *
%R_VANILLA% CMD INSTALL --build %SRC_ROOT%\rClr_%PKG_VERSION%.tar.gz
```

```R
library(testthat)
library(rClr)
test_dir('c:/src/github_jm/rClr/tests/testthat')
```

```bat
cd %R_REPO_DIR%..
7z a rClr_windows_pkgs.7z R_pkgs
```

#### Linux

Installing on Linux is always installing from source anyway, be it from a tarball, cloning the repo, or using `devtools`.

A Linux distribution with R, g++ and the Mono toolchain (including xbuild) should work. Note that while a range of Mono versions in the 3.X series may work, I recommend you use versions 3.8 or above. This may require you to look for adequate versions (for instance Debian is lagging behind currently). You may want to have a look at the instructions at the [mono download page for Linux](http://www.mono-project.com/download/#download-lin) and use the Xamarin packages.

```sh
sudo apt install mono-xbuild
```

#### Using devtools

You should be able to install the package using the `install_github` function of the package `devtools`. The following commands have been tested successfully on Windows with VS2013 and Linux with Mono 3.10, on 2014-12-19.

```R
## Optionally you may remove a prior package
remove.packages('rClr')
library(devtools)
install_github("rdotnet/rClr", build_vignettes=TRUE)
```

NOTE: you must have a fully working devtools package. If devtools, on loading, reports a warning about not finding a suitable version of RTools (on Windows), this may prevent it from installing rClr. The issue has been seen for instance using devtools 1.7.0, installed from CRAN, via R 3.2.2. Package devtools 1.7.0 seems to require RTools 3.1, even when run from R 3.2.2. One way to overcome this is to install devtools from a more recent download, from its github repository.

## Getting started

The package contains documentation, code sample and a vignette to get started.

```S
library(rClr)
?rClr
## There is an HTML vignette:
browseVignettes('rClr')
```

OBSOLETE: You will otherwise find some documentation at [https://r2clr.codeplex.com/documentation](https://r2clr.codeplex.com/documentation)

## Feedback and contributions

While this package is sometimes used for the author's paid day job, this is largely a personal endeavour. Support is appreciated in many forms.

* Citations: As of December 2014, [A presentation given at the R user conference 2013](https://publications.csiro.au/rpr/pub?list=ASE&pid=csiro:EP132284&expert=false&sb=RECENT&n=6&rpp=50&page=17&tr=3274&dr=all&csiro.affiliation=B3800). A journal paper will, hmm, "soon" follow.
* Documentation: reporting issues, feature requests or discussion threads as such can be very valuable material if done well.  
* Consulting or contract work is an option that may be arranged.

## Related work

A few packages using rClr are publicly accessible, and may be of interest if you want to build your own package with dependencies on rClr.

* If you are interested in environmental modelling: [RtoTIME](https://github.com/jmp75/RtoTIME) is a package that depends on rClr
* [rsqlserver](https://github.com/agstudy/rsqlserver) is an Sql Server driver database interface (DBI) driver for R
* [A package to access optimization tools on .NET](https://github.com/jmp75/metaheuristics/tree/master/R/pkgs/mh)
