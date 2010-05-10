cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  playlist.cpp
  playlistbackend.cpp
  playlistdelegates.cpp
  playlistheader.cpp
  playlistitem.cpp
  playlistsequence.cpp
  playlistundocommands.cpp
  playlistview.cpp
  songplaylistitem.cpp
)

set(HEADERS
  playlist.h
  playlistbackend.h
  playlistdelegates.h
  playlistheader.h
  playlistsequence.h
  playlistview.h
  songmimedata.h
)

set(UI
  playlistsequence.ui
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})

add_library(clementine_playlist
  ${SOURCES}
  ${MOC}
  ${UIC}
)

target_link_libraries(clementine_playlist
  clementine_core
  clementine_library
  clementine_radio
)

add_translation_source(playlist ${SOURCES} ${MOC} ${UIC})
