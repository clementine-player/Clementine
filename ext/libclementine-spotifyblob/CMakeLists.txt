include_directories(${PROTOBUF_INCLUDE_DIRS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/blobversion.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/blobversion.h)

set(MESSAGES
  spotifymessages.proto
)

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${MESSAGES})

add_library(clementine-spotifyblob-messages STATIC
  ${PROTO_SOURCES}
)

target_link_libraries(clementine-spotifyblob-messages
  libclementine-common
)
