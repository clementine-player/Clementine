include_directories(${PROTOBUF_INCLUDE_DIRS})

set(MESSAGES
  remotecontrolmessages.proto
)

add_library(libclementine-remote STATIC
  ${MESSAGES}
)

protobuf_generate(TARGET libclementine-remote LANGUAGE cpp)

target_link_libraries(libclementine-remote
  libclementine-common
  protobuf::libprotobuf
)

