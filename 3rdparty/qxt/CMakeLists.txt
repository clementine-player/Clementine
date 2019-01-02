cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 11)

set(QXT-SOURCES
  qxtglobal.cpp
  qxtglobalshortcut.cpp
)

set(QXT-MOC-HEADERS
  qxtglobalshortcut.h
)

include_directories(${X11_INCLUDE_DIR})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

if(WIN32)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_win.cpp)
elseif(APPLE)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_mac.cpp)
else(WIN32)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_x11.cpp)
endif(WIN32)

QT5_WRAP_CPP(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})

ADD_LIBRARY(qxt STATIC
  ${QXT-SOURCES}
  ${QXT-SOURCES-MOC}
)

if(WIN32)
  target_link_libraries(qxt Qt5::Core Qt5::Widgets)
else(WIN32)
  target_link_libraries(qxt Qt5::Core Qt5::Widgets Qt5::X11Extras)
endif(WIN32)
