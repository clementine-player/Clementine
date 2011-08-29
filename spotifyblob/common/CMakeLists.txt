include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/blobversion.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/blobversion.h)

set(COMMON_SOURCES
  spotifymessagehandler.cpp
)

set(COMMON_HEADERS
  spotifymessagehandler.h
)

set(COMMON_MESSAGES
  spotifymessages.proto
)

qt4_wrap_cpp(COMMON_MOC ${COMMON_HEADERS})
protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${COMMON_MESSAGES})

add_library(clementine-spotifyblob-messages STATIC
  ${COMMON_SOURCES}
  ${COMMON_MOC}
  ${PROTO_SOURCES}
)

# Use protobuf-lite if it's available
if(PROTOBUF_LITE_LIBRARY AND USE_PROTOBUF_LITE)
  set(protobuf ${PROTOBUF_LITE_LIBRARY})
else(PROTOBUF_LITE_LIBRARY AND USE_PROTOBUF_LITE)
  set(protobuf ${PROTOBUF_LIBRARY})
endif(PROTOBUF_LITE_LIBRARY AND USE_PROTOBUF_LITE)

target_link_libraries(clementine-spotifyblob-messages
  ${protobuf}
  ${CMAKE_THREAD_LIBS_INIT}
)

