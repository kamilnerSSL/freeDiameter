# The dict_sh extension
PROJECT("Diameter Sh (3GPP) dictionary definitions" C)

# Compile as a module
FD_ADD_EXTENSION(dict_sh dict_sh.c add_sh_avps.c)

####
## INSTALL section ##
INSTALL(TARGETS dict_sh
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-dictionary-sh)