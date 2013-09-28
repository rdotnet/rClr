\name{clrGetStaticMethods}
\alias{clrGetStaticMethods}
\title{Gets the static members for a type}
\usage{
  clrGetStaticMethods(obj_or_typename, contains = "")
}
\arguments{
  \item{obj_or_typename}{a CLR object, or type name,
  possibly namespace and assembly qualified type name, e.g.
  'My.Namespace.MyClass,MyAssemblyName'.}

  \item{contains}{a string that the property names returned
  must contain}
}
\description{
  Gets the static members for a type
}

