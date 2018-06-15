
# Scope

These notes gather information and assessment regarding the possibility to target .NET Core (v2.0) for embedding in rClr

# "Literature review"

## Generic information

[MS doc - .NET Core hosting](https://docs.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)
[MS doc - CLR Hosting](https://docs.microsoft.com/en-us/dotnet/framework/unmanaged-api/hosting/index)
[Blog post hoswing coreclr](http://yizhang82.me/hosting-coreclr)
[Hichhikers guide to coreclr](http://mattwarren.org/2017/03/23/Hitchhikers-Guide-to-the-CoreCLR-Source-Code)
[Mono embedding](http://www.mono-project.com/docs/advanced/embedding)

[Python for .NET thread on CoreCLR](https://github.com/pythonnet/pythonnet/issues/96)

## Relevant source code:

[Marqin/simpleCoreCLRHost](https://github.com/Marqin/simpleCoreCLRHost)
[CoreCLR embedding in pythonnet](https://github.com/pythonnet/pythonnet/issues/96)
[Core CLR hosting part of the codebase](https://github.com/dotnet/coreclr/tree/master/src/coreclr/hosts)
[Fancy.CoreClrHost codebase](https://github.com/fancyDevelopment/Fancy.CoreClrHost) and the related [post](http://www.fancy-development.net/hosting-net-core-clr-in-your-own-process)

## Related but less relevant:

[CppSharp](https://github.com/mono/CppSharp)

# Notes 

## Main research questions

Bootstrapping the .NET execution engine is the start, and this is what most of the information out there details. Fine. But in the end it is a small part of rclr, and documentation is very patchy or unclear as to: 

How much of the prior, .NET Framework MSCorEE API objects and methods remains valid or conversely superseded by .NET core
How much of the prior, .NET Framework MSCorEE API objects and methods remains windows only
Would the Mono part of the native codebase be completely obsolete and if so, has as much to be rewritten for .NET core 


More "drastic" or left field approaches:
* Feasibility to completely reengineer with more code in C# than C++ - but has this already been pushed as much as possible?
* Is there an opportunity to engineer some subsystems and share with projects such as Python for .NET

* Desirable things "on the edge"
    * Using Rcpp for glue code - definitely wanted technically, but what does that mean for licensing; are there subtleties different from R itself in terms of rClr users


C:\src\tmp\coreclr-master\src\coreclr\hosts\unixcoreruncommon\coreruncommon.h is very thin.
C:\src\tmp\coreclr-master\src\coreclr\hosts\inc\coreclrhost.h 

