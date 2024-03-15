include_directories(${PROTOBUF_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set(MESSAGES
  remotecontrolmessages.proto
)

add_library(libclementine-remote STATIC)

protobuf_generate(
  LANGUAGE cpp
  TARGET libclementine-remote
  PROTOS ${MESSAGES}
)

set_property(
  TARGET libclementine-remote
  PROPERTY CXX_STANDARD 17
)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

target_link_libraries(libclementine-remote
  protobuf::libprotobuf
  libclementine-common
)

