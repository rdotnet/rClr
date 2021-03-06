% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/rClr-internal.R
\name{mkClrObjRef}
\alias{mkClrObjRef}
\title{Create if possible an S4 CLR object.}
\usage{
mkClrObjRef(obj, clrtype = NULL)
}
\arguments{
\item{obj}{the presumed external pointer.}

\item{clrtype}{character; the name of the CLR type for the object. If NULL, rClr retrieves the type name.}
}
\value{
a cobjRef S4 object if the argument is indeed an external pointer, 
otherwise returned unchanged if null or not an external pointer.
}
\description{
Create if possible and adequate the S4 object that wraps the external pointer to a CLR object. 
Currently not exported, as this is unlikely to be recommended for use outside of unit tests and internal to rClr.
}
