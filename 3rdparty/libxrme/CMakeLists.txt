include_directories(${CMAKE_BINARY_DIR})

set(SOURCES
  common.cpp
  connection.cpp
  extensions.cpp
  handler.cpp
  mediaplayerhandler.cpp
  mediaplayerinterface.cpp
  mediastoragehandler.cpp
  mediastorageinterface.cpp
  remotecontrolhandler.cpp
  remotecontrolinterface.cpp
)

set(HEADERS
  connection.h
)

SET(PUBLIC_HEADERS
  connection.h
  common.h
  mediaplayerinterface.h
  mediastorageinterface.h
  remotecontrolinterface.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(xrme ${SOURCES} ${MOC})
target_link_libraries(xrme
  gloox
  ${QT_LIBRARIES}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Also install public headers into the cmake binary dir so packages can use
# libxrme in their source tree by doing include_directories(${CMAKE_BINARY_DIR})
foreach(header ${PUBLIC_HEADERS})
  configure_file(${header} ${CMAKE_BINARY_DIR}/xrme/${header} COPYONLY)
endforeach(header)
