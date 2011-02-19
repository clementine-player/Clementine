include_directories(${CMAKE_BINARY_DIR})
include_directories(${GLOOX_INCLUDE_DIRS})

link_directories(${GLOOX_LIBRARY_DIRS})

set(SOURCES
  common.cpp
  connection.cpp
  handler.cpp
  mediaplayerhandler.cpp
  mediaplayerinterface.cpp
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
  remotecontrolinterface.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(xrme ${SOURCES} ${MOC})
target_link_libraries(xrme
  ${QT_LIBRARIES}
  ${GLOOX_LIBRARIES}
)

# Install library
install(TARGETS xrme
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Install public headers
install(FILES ${PUBLIC_HEADERS}
  DESTINATION include/xrme/
)

# Also install public headers into the cmake binary dir so packages can use
# libxrme in their source tree by doing include_directories(${CMAKE_BINARY_DIR})
foreach(header ${PUBLIC_HEADERS})
  configure_file(${header} ${CMAKE_BINARY_DIR}/xrme/${header} COPYONLY)
endforeach(header)
