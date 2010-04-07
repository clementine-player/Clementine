cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

if (CMAKE_FIND_ROOT_PATH)
  # Help find the Qt headers if we're cross compiling (since we can't run qmake.exe)
  # CMAKE_FIND_ROOT_PATH should get set by your cmake toolchain file
  set(QT_HEADERS_DIR ${CMAKE_FIND_ROOT_PATH}/include)
  set(QT_LIBRARY_DIR ${CMAKE_FIND_ROOT_PATH}/lib)
endif (CMAKE_FIND_ROOT_PATH)

find_package(Qt4 REQUIRED)
set(QT_USE_QTOPENGL 1)
set(QT_USE_QTSQL 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTXML 1)
if(UNIX AND NOT APPLE)
  set(QT_USE_QTDBUS 1)
endif(UNIX AND NOT APPLE)

find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)
find_package(Gettext REQUIRED)

if(WIN32)
  find_library(TAGLIB_LIBRARIES tag)
  find_path(GLIB_INCLUDE_DIRS glib.h
    PATH_SUFFIXES glib glib-2.0)
  find_path(GLIBCONFIG_INCLUDE_DIRS glibconfig.h
    PATH_SUFFIXES ../lib/glib/include ../lib/glib-2.0/include)
  find_path(LIBXML_INCLUDE_DIRS libxml/parser.h
    PATH_SUFFIXES libxml libxml2)
  find_library(GSTREAMER_LIBRARIES gstreamer-0.10)
  find_library(GSTREAMER_BASE_LIBRARIES gstbase-0.10)
  find_library(GLIB_LIBRARIES glib-2.0)
  find_library(GOBJECT_LIBRARIES gobject-2.0)
else(WIN32)
  pkg_check_modules(TAGLIB taglib)
  pkg_check_modules(GSTREAMER gstreamer-0.10)
  pkg_check_modules(GSTREAMER_BASE gstreamer-base-0.10)

  if (TAGLIB_VERSION VERSION_LESS 1.6)
    message(FATAL_ERROR "Taglib version 1.6 or greater is required")
  endif (TAGLIB_VERSION VERSION_LESS 1.6)
endif(WIN32)

if (NOT Boost_FOUND)
  message(FATAL_ERROR "Boost not found")
endif (NOT Boost_FOUND)

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
link_directories(${GSTREAMER_LIBRARY_DIRS})
link_directories(${LASTFM_LIBRARY_DIRS})

include(${QT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GLIBCONFIG_INCLUDE_DIRS})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${LASTFM_INCLUDE_DIRS})
include_directories("3rdparty/qsqlite")

# Remove GLU and GL from the link line - they're not really required
# and don't exist on my mingw toolchain
list(REMOVE_ITEM QT_LIBRARIES "-lGLU -lGL")

if (WIN32)
  # RC compiler
  string(REPLACE "gcc" "windres" CMAKE_RC_COMPILER_INIT ${CMAKE_C_COMPILER})
  enable_language(RC)
  SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> <SOURCE> -I../../dist/windows")
endif(WIN32)

add_definitions(-DQXT_STATIC -DBUILD_QXT_GUI -DBUILD_QXT_CORE)

# Translations stuff
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

# Subdirectories
add_subdirectory(3rdparty/qtsingleapplication)
add_subdirectory(3rdparty/qxt)
add_subdirectory(src)

add_subdirectory(tests)

if (WIN32)
  add_definitions(-DQT_STATICPLUGIN)
  add_subdirectory(3rdparty/qsqlite)
endif(WIN32)

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
