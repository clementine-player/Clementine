cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  asxparser.cpp
  m3uparser.cpp
  parserbase.cpp
  playlistparser.cpp
  plsparser.cpp
  xmlparser.cpp
  xspfparser.cpp
)

set(HEADERS
  asxparser.h
  m3uparser.h
  parserbase.h
  playlistparser.h
  plsparser.h
  xspfparser.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(clementine_playlistparsers
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine_playlistparsers
)

add_translation_source(playlistparsers ${SOURCES} ${MOC})
