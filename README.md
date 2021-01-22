# rClr

## R package for accessing .NET

Accessing the Common Language Runtime (.NET or Mono) from the R statistical software, in-process.

## Prerequisites

As of May 2019, on Windows Clr requires the .NET Framework (4.6.1+, 4.7.2+ recommended). Groundwork towards running on .NET Core started but it is unclear how to embed the .NET Core runtime into R.

## Installing

Releases can be found via the [release tab of the rClr GitHub repository](https://github.com/Open-Systems-Pharmacology/rClr/releases).

### Pre-compiled binaries

You can install pre-compiled rClr for Windows 10 x64, Ubuntu 18, and CentOS 7 via the [release tab of the rClr GitHub repository](https://github.com/Open-Systems-Pharmacology/rClr/releases). Binary packages are available for R 3.6 and 4.x.

### From source

Compiling of the **master** branch from source has been tested under Windows 10 and Linux Ubuntu 18.

#### Windows

On Windows you will need a C# and C and/or Visual C++ compiler. The current releases have been made using the Visual Studio 2019 toolchain. Check the [visual studio download page](https://visualstudio.microsoft.com/downloads/) for options.

It is recommended to build the packages in RStudio.

Following path variables must be present in your environment (see [How to set an environment variable in Windows 10](https://www.onmsft.com/how-to/how-to-set-an-environment-variable-in-windows-10))

- `MSBUILD_EXE_PATH` pointing to MSBuild.exe, e.g. `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe`
- `VSCOMMON` pointing to the folder where Visual Studio common folder is located, e.g. `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7`. Alternatively, you can set the path in the file `configure.win'.

#### Linux

Install on Ubuntu 18.04. Mono is supported up to versiono 5.18, so make sure that mono 6 is not installed.

##### GIT
```
sudo apt install git
```

##### NUGET
```
sudo apt install nuget
sudo nuget update -self
```

##### LIBS
```
sudo apt update
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install libssl-dev
sudo apt install libxml2-dev
```

##### MONO

```
sudo apt install gnupg ca-certificates
sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
echo "deb https://download.mono-project.com/repo/ubuntu stable-bionic/snapshots/5.18 main" | sudo tee /etc/apt/sources.list.d/mono-official-stable.list
sudo apt update
sudo apt install mono-complete
```

Check version with `mono -V`. It should read `5.18.xxx`


##### .NET CORE SDK (For ubuntu 18.04..). 

Install vary based on system. see https://aka.ms/dotnet-download
(e.g. https://docs.microsoft.com/en-us/dotnet/core/install/linux-package-manager-ubuntu-1910)


```
wget -q https://packages.microsoft.com/config/ubuntu/18.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb

sudo add-apt-repository universe
sudo apt-get update
sudo apt-get install apt-transport-https
sudo apt-get update
sudo apt-get install dotnet-sdk-3.1
```

Check version with `dotnet --version`. It should read `3.1.xxx`


##### Install R Packages

Within a R sesssion in a terminal

```
install.packages('devtools')
```

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
