include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(MESSAGES
  tagreadermessages.proto
)

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${MESSAGES})

add_library(libclementine-tagreader STATIC
  ${PROTO_SOURCES}
)

target_link_libraries(libclementine-tagreader
  libclementine-common
)

