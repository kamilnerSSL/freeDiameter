# The dict_rfc5777 extension
PROJECT("S6a protocol based on 3GPP 29.272-990" C)

# Compile as a module
FD_ADD_EXTENSION(dict_s6a dict_s6a.c)


####
## INSTALL section ##

INSTALL(TARGETS dict_s6a
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-dictionary-s6a)
