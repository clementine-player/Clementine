include_directories(${PROTOBUF_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set(MESSAGES
  remotecontrolmessages.proto
)

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${MESSAGES})

add_library(libclementine-remote STATIC
  ${PROTO_SOURCES}
)

target_link_libraries(libclementine-remote
  ${PROTOBUF_LIBRARY}
  libclementine-common
)

