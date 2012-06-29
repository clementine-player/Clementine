include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCES
  core/closure.cpp
  core/concurrentrun.cpp
  core/encoding.cpp
  core/logging.cpp
  core/messagehandler.cpp
  core/messagereply.cpp
  core/waitforsignal.cpp
  core/workerpool.cpp
)

set(HEADERS
  core/closure.h
  core/messagehandler.h
  core/messagereply.h
  core/workerpool.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(libclementine-common STATIC
  ${SOURCES}
  ${MOC}
)

# Use protobuf-lite if it's available
if(PROTOBUF_LITE_LIBRARY AND USE_PROTOBUF_LITE)
  set(protobuf ${PROTOBUF_LITE_LIBRARY})
else(PROTOBUF_LITE_LIBRARY AND USE_PROTOBUF_LITE)
  set(protobuf ${PROTOBUF_LIBRARY})
endif(PROTOBUF_LITE_LIBRARY AND USE_PROTOBUF_LITE)

target_link_libraries(libclementine-common
  ${protobuf}
  ${TAGLIB_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  chardet
)

