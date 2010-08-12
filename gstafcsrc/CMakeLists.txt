cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Woverloaded-virtual -Wall")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${IMOBILEDEVICE_INCLUDE_DIRS})

set(SOURCES
  gstafcsrc.c
)

add_library(gstafcsrc STATIC
  ${SOURCES}
)

target_link_libraries(gstafcsrc
  ${GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${IMOBILEDEVICE_LIBRARIES}
)
