cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECTM_INCLUDE_DIRS})

set(SOURCES
  projectmvisualisation.cpp
  visualisationcontainer.cpp
  visualisationoverlay.cpp
)

set(HEADERS
  projectmvisualisation.h
  visualisationcontainer.h
  visualisationoverlay.h
)

set(UI
  visualisationoverlay.ui
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})

add_library(clementine_visualisations
  ${SOURCES}
  ${MOC}
  ${UIC}
)

target_link_libraries(clementine_visualisations
  clementine_core
  clementine_engines
  ${PROJECTM_LIBRARIES}
)

add_translation_source(visualisations ${SOURCES} ${MOC} ${UIC})
