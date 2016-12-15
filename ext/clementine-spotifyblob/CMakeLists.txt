include_directories(${SPOTIFY_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-spotifyblob)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-spotifyblob)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-unused-private-field -Wno-unknown-warning-option --std=c++0x -U__STRICT_ANSI__")

link_directories(${SPOTIFY_LIBRARY_DIRS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES
  main.cpp
  mediapipeline.cpp
  spotifyclient.cpp
  spotify_utilities.cpp
)

set(HEADERS
  spotifyclient.h
)

if(APPLE)
  list(APPEND SOURCES spotify_utilities.mm)
endif(APPLE)

qt4_wrap_cpp(MOC ${HEADERS})

if(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_WIN32_CONSOLE)
  set(win32_build_flag WIN32)
endif(WIN32 AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_WIN32_CONSOLE)

add_executable(clementine-spotifyblob
  ${win32_build_flag}
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine-spotifyblob
  ${SPOTIFY_LIBRARIES} ${SPOTIFY_LDFLAGS}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_APP_LIBRARIES}
  ${PROTOBUF_STATIC_LIBRARY}
  clementine-spotifyblob-messages
  libclementine-common
)

if(APPLE)
  target_link_libraries(clementine-spotifyblob
    "-framework Foundation"
  )
endif(APPLE)

if(NOT APPLE)
  # macdeploy.py takes care of this on mac
  install(TARGETS clementine-spotifyblob
          RUNTIME DESTINATION bin
  )
endif(NOT APPLE)

if(LINUX)
  # Versioned name of the blob
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SPOTIFY_BLOB_ARCH 32)
  else(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SPOTIFY_BLOB_ARCH 64)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

  install(
    FILES ${CMAKE_BINARY_DIR}/clementine-spotifyblob
    DESTINATION ${CMAKE_BINARY_DIR}/spotify/version${SPOTIFY_BLOB_VERSION}-${SPOTIFY_BLOB_ARCH}bit/
    RENAME blob
  )
endif(LINUX)
