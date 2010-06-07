cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories("../../3rdparty/libprojectm")

set(SOURCES
  projectmpresetmodel.cpp
  projectmvisualisation.cpp
  visualisationcontainer.cpp
  visualisationoverlay.cpp
  visualisationselector.cpp
)

set(HEADERS
  projectmpresetmodel.h
  projectmvisualisation.h
  visualisationcontainer.h
  visualisationoverlay.h
  visualisationselector.h
)

set(UI
  visualisationoverlay.ui
  visualisationselector.ui
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
  projectM
)

add_translation_source(visualisations ${SOURCES} ${MOC} ${UIC})
