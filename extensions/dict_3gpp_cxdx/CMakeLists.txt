# The dict_3gpp_cxdx extension
PROJECT("Diameter CxDx (3GPP) dictionary definitions" C)

# Compile as a module
FD_ADD_EXTENSION(dict_3gpp_cxdx dict_3gpp_cxdx.c)

####
## INSTALL section ##

INSTALL(TARGETS dict_3gpp_cxdx
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-dictionary-cxdx)
