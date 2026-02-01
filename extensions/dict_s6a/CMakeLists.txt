# The dict_s6a extension
PROJECT("Diameter s6a (3GPP) dictionay definitions" C)

# Compile as a module
FD_ADD_EXTENSION(dict_s6a dict_s6a.c)

####
## INSTALL section ##

INSTALL(TARGETS dict_s6a
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-dictionary-s6a)