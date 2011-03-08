PROJECT(libportfwd)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_INSTALL_PREFIX "/usr/local/")

SET(MINIUPNP_DIR    "third-party/miniupnpc-20090605/")
SET(NATPMP_DIR      "third-party/libnatpmp")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
ADD_DEFINITIONS(-Wall -O2 -DNDEBUG)
ADD_DEFINITIONS(-fPIC)
INCLUDE_DIRECTORIES(${MINIUPNP_DIR} include)

ADD_LIBRARY(portfwd STATIC

        # the needed bits of miniupnpc (no python module, no tests, no cli)
        ${MINIUPNP_DIR}/igd_desc_parse.c
        ${MINIUPNP_DIR}/minisoap.c
        ${MINIUPNP_DIR}/minissdpc.c
        ${MINIUPNP_DIR}/miniupnpc.c
        ${MINIUPNP_DIR}/miniwget.c
        ${MINIUPNP_DIR}/minixml.c
        ${MINIUPNP_DIR}/minixmlvalid.c
        ${MINIUPNP_DIR}/upnpc.c
        ${MINIUPNP_DIR}/upnpcommands.c
        ${MINIUPNP_DIR}/upnperrors.c
        ${MINIUPNP_DIR}/upnpreplyparse.c

        # Our wrapper:
        src/portfwd.cpp
)

#ADD_EXECUTABLE(portfwd-demo
#               src/main.cpp
#              )

#TARGET_LINK_LIBRARIES(portfwd-demo portfwd)

#INSTALL(TARGETS portfwd ARCHIVE DESTINATION lib)
#INSTALL(TARGETS portfwd-demo RUNTIME DESTINATION bin)
#INSTALL(DIRECTORY include/portfwd DESTINATION include PATTERN "*~" EXCLUDE)
