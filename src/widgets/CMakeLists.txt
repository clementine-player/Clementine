cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  autoexpandingtreeview.cpp
  busyindicator.cpp
  equalizerslider.cpp
  fileview.cpp
  fileviewlist.cpp
  lineedit.cpp
  multiloadingindicator.cpp
  osd.cpp
  osdpretty.cpp
  sliderwidget.cpp
  spinbox.cpp
  stickyslider.cpp
  trackslider.cpp
  tracksliderslider.cpp
)

set(HEADERS
  autoexpandingtreeview.h
  busyindicator.h
  equalizerslider.h
  fileview.h
  fileviewlist.h
  lineedit.h
  multiloadingindicator.h
  osd.h
  osdpretty.h
  sliderwidget.h
  spinbox.h
  stickyslider.h
  trackslider.h
)

set(UI
  equalizerslider.ui
  fileview.ui
  multiloadingindicator.ui
  osdpretty.ui
  trackslider.ui
)

if(APPLE)
  list(APPEND SOURCES osd_mac.mm)
  include_directories(${GROWL}/Headers)
else(APPLE)
  if(WIN32)
    list(APPEND SOURCES osd_win.cpp)
  else(WIN32)
    list(APPEND SOURCES osd_x11.cpp)
  endif(WIN32)
endif(APPLE)


qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})

add_library(clementine_widgets
  ${SOURCES}
  ${MOC}
  ${UIC}
)

target_link_libraries(clementine_widgets
  clementine_core
)

add_translation_source(widgets ${SOURCES} ${MOC} ${UIC})
