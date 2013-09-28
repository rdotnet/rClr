\name{clrTypename}
\alias{clrTypename}
\title{Gets the type name of an object}
\usage{
  clrTypename(clrobj)
}
\arguments{
  \item{clrobj}{CLR object}
}
\value{
  type name
}
\description{
  Gets the CLR type name of an object, given an S4 clrobj
  object
}
\examples{
\dontrun{
library(rClr)
testClassName <- "Rclr.TestObject";
testObj <- clrNew(testClassName)
clrTypename(testObj)
}
}

