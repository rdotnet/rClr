\name{clrGetInnerPkgName}
\alias{clrGetInnerPkgName}
\title{Gets the inner name used for the package}
\usage{
  clrGetInnerPkgName()
}
\value{
  the short name of the library currently loaded, depending
  on the runtime used (Mono or Microsoft .NET)
}
\description{
  Gets the inner name used for the package (rClrMono or
  rClrMs). This is not intented for use by most users.
}

