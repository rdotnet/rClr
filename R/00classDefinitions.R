# Declares the S4 class that is used to hold references to CLR objects.
setClass("cobjRef", representation(clrobj="externalptr", clrtype="character"), prototype=list(clrobj=NULL, clrtype="System.Object"))

