include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set(MESSAGES
  tagreadermessages.proto
)

set(SOURCES
  fmpsparser.cpp
  tagreader.cpp
  gmereader.cpp
)

set(HEADERS
)

optional_source(HAVE_GOOGLE_DRIVE
  INCLUDE_DIRECTORIES ${SPARSEHASH_INCLUDE_DIRS}
  SOURCES
    cloudstream.cpp
  HEADERS
    cloudstream.h
)

qt5_wrap_cpp(MOC ${HEADERS})

add_library(libclementine-tagreader STATIC
  ${SOURCES}
  ${MOC}
)

protobuf_generate(
  LANGUAGE cpp
  TARGET libclementine-tagreader
  PROTOS ${MESSAGES}
)

set_property(
  TARGET libclementine-tagreader
  PROPERTY CXX_STANDARD 17
)

target_link_libraries(libclementine-tagreader
  protobuf::libprotobuf
  libclementine-common
)

