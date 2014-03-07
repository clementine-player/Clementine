prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}/bin
libdir=${LIB_DESTINATION}
includedir=${VREEN_PKG_INCDIR}

Name: vreen
Description: Simple and fast Qt Binding for vk.com API
Requires: QtCore QtNetwork
Version: ${LIBRARY_VERSION}
Libs: ${VREEN_PKG_LIBS}
Cflags: -I${VREEN_PKG_INCDIR}

