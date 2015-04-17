cmake_minimum_required(VERSION 2.6)

set(SOURCES
  fancytabwidget.cpp
  stylehelper.cpp
)

set(HEADERS
  fancytabwidget.h
)

qt5_wrap_cpp(MOC ${HEADERS})

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_library(fancytabwidget STATIC
  ${SOURCES}
  ${MOC}
)
target_link_libraries(fancytabwidget
  ${QT_LIBRARIES}
)
