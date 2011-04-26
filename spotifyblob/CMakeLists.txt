include_directories(${SPOTIFY_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

link_directories(${SPOTIFY_LIBRARY_DIRS})

set(EXECUTABLE_OUTPUT_PATH ..)


set(COMMON_SOURCES
  spotifymessageutils.cpp
)

set(COMMON_MESSAGES
  spotifymessages.proto
)

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${COMMON_MESSAGES})

add_library(clementine-spotifyblob-messages STATIC
  ${COMMON_SOURCES}
  ${PROTO_SOURCES}
)

target_link_libraries(clementine-spotifyblob-messages
  ${PROTOBUF_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)


set(SOURCES
  main.cpp
  spotifyclient.cpp

  ../src/core/logging.cpp
)

set(HEADERS
  spotifyclient.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_executable(clementine-spotifyblob
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine-spotifyblob
  ${SPOTIFY_LIBRARIES}
  ${QT_LIBRARIES}
  clementine-spotifyblob-messages
)
