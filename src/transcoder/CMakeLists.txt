cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  transcodedialog.cpp
  transcoder.cpp
  transcoderformats.cpp
)

set(HEADERS
  transcodedialog.h
  transcoder.h
)

set(UI
  transcodedialog.ui
  transcodelogdialog.ui
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})

add_library(clementine_transcoder
  ${SOURCES}
  ${MOC}
  ${UIC}
)

add_translation_source(transcoder ${SOURCES} ${MOC} ${UIC})
