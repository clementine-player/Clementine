include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set(SOURCES
  core/closure.cpp
  core/latch.cpp
  core/logging.cpp
  core/messagehandler.cpp
  core/messagereply.cpp
  core/waitforsignal.cpp
  core/workerpool.cpp
)

set(HEADERS
  core/closure.h
  core/latch.h
  core/messagehandler.h
  core/messagereply.h
  core/workerpool.h
)

if(APPLE)
  list(APPEND SOURCES core/scoped_nsautorelease_pool.mm)
endif(APPLE)

qt5_wrap_cpp(MOC ${HEADERS})

add_library(libclementine-common STATIC
  ${SOURCES}
  ${MOC}
)

set_property(
  TARGET libclementine-common
  PROPERTY CXX_STANDARD 17
)

target_link_libraries(libclementine-common
  Qt5::Core
  Qt5::Network
  ${TAGLIB_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

find_package(Backtrace)
configure_file(core/conf_backtrace.h.in conf_backtrace.h)
