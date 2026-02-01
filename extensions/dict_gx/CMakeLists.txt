# The dict_gx extension
PROJECT("Diameter Gx (3GPP) dictionary definitions" C)

# Compile as a module
FD_ADD_EXTENSION(dict_gx dict_gx.c)

####
## INSTALL section ##

INSTALL(TARGETS dict_gx
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-dictionary-gx)