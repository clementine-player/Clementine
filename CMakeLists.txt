cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

find_package(Qt4 REQUIRED)
set(QT_USE_QTOPENGL 1)
set(QT_USE_QTSQL 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTXML 1)
if(WIN32)
  set(QT_USE_PHONON 1)
endif(WIN32)

find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)

pkg_check_modules(TAGLIB taglib)
pkg_check_modules(XINE libxine)
pkg_check_modules(LIBNOTIFY libnotify)

if (NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif (NOT Boost_FOUND)

if (LIBNOTIFY_FOUND)
  add_definitions(-DHAVE_LIBNOTIFY)
  link_directories(${LIBNOTIFY_LIBRARY_DIRS})
  include_directories(${LIBNOTIFY_INCLUDE_DIRS})
endif (LIBNOTIFY_FOUND)

find_library(LASTFM_LIBRARY_DIRS lastfm)
find_path(LASTFM_INCLUDE_DIRS lastfm/ws.h)

if (APPLE)
  find_library(GROWL Growl)
endif (APPLE)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions(-DNDEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

# Set up definitions and paths
add_definitions(${QT_DEFINITIONS})
link_directories(${TAGLIB_LIBRARY_DIRS})
link_directories(${XINE_LIBRARY_DIRS})
link_directories(${LASTFM_LIBRARY_DIRS})

include(${QT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})
include_directories(${XINE_INCLUDE_DIRS})
include_directories(${LASTFM_INCLUDE_DIRS})

# Subdirectories
add_subdirectory(3rdparty/qtsingleapplication)
add_subdirectory(3rdparty/qxt)
add_subdirectory(src)

add_subdirectory(tests)
