include_directories(${SPOTIFY_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

link_directories(${SPOTIFY_LIBRARY_DIRS})

set(EXECUTABLE_OUTPUT_PATH ..)


set(SOURCES
  main.cpp
  spotifyblob.cpp
  spotifyclient.cpp

  ../src/core/logging.cpp
)

set(HEADERS
  spotifyblob.h
  spotifyclient.h
)

set(MESSAGES
  messages.proto
)

qt4_wrap_cpp(MOC ${HEADERS})
protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${MESSAGES})


add_library(clementine-spotifyblob-messages STATIC
  ${PROTO_SOURCES}
)

target_link_libraries(clementine-spotifyblob-messages
  ${PROTOBUF_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)


add_executable(clementine-spotifyblob
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine-spotifyblob
  ${SPOTIFY_LIBRARIES}
  ${QT_LIBRARIES}
  clementine-spotifyblob-messages
)
