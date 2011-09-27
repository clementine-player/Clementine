cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Woverloaded-virtual -Wall")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})

if(HAVE_LIBGPOD)
  include_directories(${LIBGPOD_INCLUDE_DIRS})
endif(HAVE_LIBGPOD)

set(SOURCES
  gstspotifytcpsrc.cpp
)

add_library(gstspotifytcpsrc STATIC
  ${SOURCES}
)

target_link_libraries(gstspotifytcpsrc
  clementine_lib
  ${GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${IMOBILEDEVICE_LIBRARIES}
)
