cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wnon-virtual-dtor -Woverloaded-virtual -Wall")

# Source files
set(CLEMENTINE-SCOPE-SOURCES
  clementinescope.cpp
)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")

if (WIN32)
  # This gets linked at runtime on unix
  set(MAYBE_CLEMENTINE_LIB_LIBRARIES clementine_lib)
endif (WIN32)

add_library(clementine_scope SHARED
  ${CLEMENTINE-SCOPE-SOURCES}
)
target_link_libraries(clementine_scope
  ${VLC_PLUGIN_LIBRARIES}
  ${QT_LIBRARIES}
  ${MAYBE_CLEMENTINE_LIB_LIBRARIES}
)

if (NOT WIN32)
  install(TARGETS clementine_scope
    LIBRARY DESTINATION ${VLC_PLUGIN_LIBDIR}/vlc/clementine
  )
endif (NOT WIN32)
