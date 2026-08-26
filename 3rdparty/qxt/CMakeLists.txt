cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 11)

set(QXT-SOURCES
  qxtglobal.cpp
  qxtglobalshortcut.cpp
)

set(QXT-MOC-HEADERS
  qxtglobalshortcut.h
)

include_directories(${X11_INCLUDE_DIR})
include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})

if(WIN32)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_win.cpp)
elseif(APPLE)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_mac.cpp)
else(WIN32)
  set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_x11.cpp)
endif(WIN32)

QT6_WRAP_CPP(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})

ADD_LIBRARY(qxt STATIC
  ${QXT-SOURCES}
  ${QXT-SOURCES-MOC}
)

# Qt6 has no X11Extras module. qxtglobalshortcut_x11.cpp's QX11Info usage and
# nativeEventFilter() long*->qintptr* signature change still need reworking
# for Qt6 (tracked separately) - this only fixes the CMake side.
target_link_libraries(qxt Qt6::Core Qt6::Widgets)
