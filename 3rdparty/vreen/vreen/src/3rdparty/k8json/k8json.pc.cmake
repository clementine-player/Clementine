prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}/bin
libdir=${LIB_DESTINATION}
includedir=${CMAKE_INSTALL_PREFIX}/include

Name: k8json
Description: Small and fast Qt JSON parser
Requires: QtCore
Version: ${CMAKE_LIBK8JSON_VERSION_MAJOR}.${CMAKE_LIBK8JSON_VERSION_MINOR}.${CMAKE_LIBK8JSON_VERSION_PATCH}
Libs: -L${LIB_DESTINATION} -lk8json
Cflags: -I${CMAKE_INSTALL_PREFIX}/include

