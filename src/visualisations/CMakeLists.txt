cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECTM_INCLUDE_DIRS})

set(SOURCES
  projectmvisualisation.cpp
  visualisationcontainer.cpp
)

set(HEADERS
  projectmvisualisation.h
  visualisationcontainer.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(clementine_visualisations
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine_visualisations
  clementine_core
  clementine_engines
  ${PROJECTM_LIBRARIES}
)

add_translation_source(visualisations ${SOURCES} ${MOC})
