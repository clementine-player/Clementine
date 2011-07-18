include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/covers
  ${CMAKE_SOURCE_DIR}/src/engines
  ${CMAKE_SOURCE_DIR}/src/internet
  ${CMAKE_SOURCE_DIR}/src/library
  ${CMAKE_SOURCE_DIR}/src/playlist
  ${CMAKE_SOURCE_DIR}/src/playlistparsers
  ${CMAKE_SOURCE_DIR}/src/smartplaylists
  ${CMAKE_SOURCE_DIR}/src/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/clementine
  ${PYTHON_INCLUDE_DIRS}
)

set(SOURCES
  clementine/clementine0.cpp
  clementine/clementine1.cpp
  clementine/clementine_init.cpp
)

set(HEADERS
  clementine/clementine0.h
  clementine/clementine1.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(clementine-pythonqt-bindings
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine-pythonqt-bindings
  clementine_lib
)
