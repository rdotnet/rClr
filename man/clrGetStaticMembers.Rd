\name{clrGetStaticMembers}
\alias{clrGetStaticMembers}
\title{Gets the static members for a type}
\usage{
  clrGetStaticMembers(obj_or_typename)
}
\arguments{
  \item{obj_or_typename}{a CLR object, or type name,
  possibly namespace and assembly qualified type name, e.g.
  'My.Namespace.MyClass,MyAssemblyName'.}
}
\description{
  Gets the static members for a type
}
\examples{
\dontrun{
library(rClr)
cTypename <- "Rclr.TestCases"
clrGetStaticMembers(cTypename)
testClassName <- "Rclr.TestObject";
testObj <- clrNew(testClassName)
clrGetStaticMembers(testObj)
}
}

