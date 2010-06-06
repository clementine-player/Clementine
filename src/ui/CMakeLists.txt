cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  about.cpp
  addstreamdialog.cpp
  albumcovermanager.cpp
  edittagdialog.cpp
  equalizer.cpp
  globalshortcutgrabber.cpp
  globalshortcutsdialog.cpp
  iconloader.cpp
  mainwindow.cpp
  settingsdialog.cpp
  systemtrayicon.cpp
)

set(HEADERS
  about.h
  addstreamdialog.h
  albumcovermanager.h
  edittagdialog.h
  equalizer.h
  globalshortcutgrabber.h
  globalshortcutsdialog.h
  mainwindow.h
  settingsdialog.h
  systemtrayicon.h
)

set(UI
  about.ui
  addstreamdialog.ui
  albumcovermanager.ui
  edittagdialog.ui
  equalizer.ui
  globalshortcutgrabber.ui
  globalshortcutsdialog.ui
  mainwindow.ui
  settingsdialog.ui
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})

add_library(clementine_ui
  ${SOURCES}
  ${MOC}
  ${UIC}
)

target_link_libraries(clementine_ui
  clementine_analyzers
  clementine_core
  clementine_engines
  clementine_library
  clementine_playlist
  clementine_playlistparsers
  clementine_radio
  clementine_transcoder
  clementine_visualisations
)

add_translation_source(ui ${SOURCES} ${MOC} ${UIC})
