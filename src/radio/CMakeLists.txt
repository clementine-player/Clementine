cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  fixlastfm.cpp
  lastfmconfig.cpp
  lastfmconfigdialog.cpp
  lastfmservice.cpp
  lastfmstationdialog.cpp
  magnatuneplaylistitem.cpp
  magnatuneservice.cpp
  radioitem.cpp
  radiomodel.cpp
  radioplaylistitem.cpp
  radioservice.cpp
  radioview.cpp
  radioviewcontainer.cpp
  savedradio.cpp
  somafmservice.cpp
)

set(HEADERS
  lastfmconfig.h
  lastfmconfigdialog.h
  lastfmservice.h
  lastfmstationdialog.h
  magnatuneservice.h
  radiomimedata.h
  radiomodel.h
  radioservice.h
  radioview.h
  radioviewcontainer.h
  savedradio.h
  somafmservice.h
)

set(UI
  lastfmconfig.ui
  lastfmconfigdialog.ui
  lastfmstationdialog.ui
  radioviewcontainer.ui
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})

add_library(clementine_radio
  ${SOURCES}
  ${MOC}
  ${UIC}
)

target_link_libraries(clementine_radio
  clementine_core
  clementine_library
)

add_translation_source(radio ${SOURCES} ${MOC} ${UIC})
