\name{createReturnedObject}
\alias{createReturnedObject}
\title{Create if possible an S4 CLR object.}
\usage{
  createReturnedObject(ext_ptr)
}
\arguments{
  \item{ext_ptr}{the external pointer.}
}
\value{
  a cobjRef S4 object if the argument is indeed an external
  pointer, otherwise returned unchanged if null of not an
  external pointer.
}
\description{
  Create if possible and adequate the S4 object that wraps
  the external pointer to a CLR object. Currently not
  exported, as this is unlikely to be recommended for use
  outside of unit tests and internal to rClr.
}

