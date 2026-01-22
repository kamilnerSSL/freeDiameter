cmake_minimum_required(VERSION 2.6)
PROJECT("dbg_metrics" C)

# Find libmicrohttpd headers
FIND_PATH(MHD_INCLUDE_DIR microhttpd.h)
FIND_LIBRARY(MHD_LIBRARY NAMES microhttpd)

IF(NOT MHD_INCLUDE_DIR OR NOT MHD_LIBRARY)
    MESSAGE(FATAL_ERROR "libmicrohttpd not found. Please install libmicrohttpd-dev")
ENDIF()

INCLUDE_DIRECTORIES(${MHD_INCLUDE_DIR})

FD_ADD_EXTENSION(dbg_metrics dbg_metrics.c)

TARGET_LINK_LIBRARIES(metrics ${MHD_LIBRARY})
INSTALL(TARGETS dbg_metrics LIBRARY DESTINATION ${INSTALL_EXTENSIONS_DIR})