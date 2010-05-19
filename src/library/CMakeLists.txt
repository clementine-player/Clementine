cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  groupbydialog.cpp
  library.cpp
  librarybackend.cpp
  libraryconfig.cpp
  libraryconfigdialog.cpp
  librarydirectorymodel.cpp
  libraryfilterwidget.cpp
  librarymodel.cpp
  libraryplaylistitem.cpp
  libraryquery.cpp
  libraryview.cpp
  librarywatcher.cpp
)

set(HEADERS
  groupbydialog.h
  library.h
  librarybackend.h
  libraryconfig.h
  libraryconfigdialog.h
  librarydirectorymodel.h
  libraryfilterwidget.h
  librarymodel.h
  libraryview.h
  librarywatcher.h
)

set(UI
  groupbydialog.ui
  libraryconfig.ui
  libraryconfigdialog.ui
  libraryfilterwidget.ui
)


qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})

add_library(clementine_library
  ${SOURCES}
  ${MOC}
  ${UIC}
)

target_link_libraries(clementine_library
  clementine_core
  clementine_playlist
  clementine_ui
)

add_translation_source(library ${SOURCES} ${MOC} ${UIC})
