# Declares the S4 class that is used to hold references to CLR objects.
setClass("cobjRef", representation(clrobj="externalptr", clrtype="character"), prototype=list(clrobj=NULL, clrtype="System.Object"))

### syntactic sugar to allow object$field and object$methods(...)
### first attempts to find a field of that name and then a method
#cobjRefDollar <- function(clrobj, name) {
#  memberNames <- clrReflect(clrobj)
#	if (name %in% memberNames$Fields){
#		clrGetField(clrobj, name)
#	} else 	if (name %in% memberNames$Properties){
#		clrGetProperty(clrobj, name) # not sure...
#	} else 	if (name %in% memberNames$Methods){
#    function(...) clrCallMethod(clrobj, name, ...)
#	# else if inner class?
#	} else {
#		stop( sprintf( "no field, property or method called '%s' ", name ) ) 
#	}
#}
# setMethod("$", c(x="cobjRef"), cobjRefDollar )

### support for object$field<-...
# ._jobjRef_dollargets <- function(x, name, value) {
	# if( hasField( x, name ) ){
		# .jfield(x, name) <- value
	# }
	# x
# }
# setMethod("$<-", c(x="jobjRef"), ._jobjRef_dollargets )

