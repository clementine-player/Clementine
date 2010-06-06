cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)
include(Version.cmake)

if (CMAKE_FIND_ROOT_PATH)
  # Help find the Qt headers if we're cross compiling (since we can't run qmake.exe)
  # CMAKE_FIND_ROOT_PATH should get set by your cmake toolchain file
  set(QT_HEADERS_DIR ${CMAKE_FIND_ROOT_PATH}/include)
  set(QT_LIBRARY_DIR ${CMAKE_FIND_ROOT_PATH}/lib)
endif (CMAKE_FIND_ROOT_PATH)

find_package(Qt4 REQUIRED QtCore QtGui QtOpenGL QtSql QtNetwork QtXml)
if(UNIX AND NOT APPLE)
  find_package(Qt4 REQUIRED QtDbus)
endif(UNIX AND NOT APPLE)
find_package(Qt4 COMPONENTS Phonon)

# Find Qt's lconvert binary.  Try qt's binary dir first, fall back to looking in PATH
find_program(QT_LCONVERT_EXECUTABLE NAMES lconvert lconvert-qt4 PATHS ${QT_BINARY_DIR} NO_DEFAULT_PATH)
find_program(QT_LCONVERT_EXECUTABLE NAMES lconvert lconvert-qt4)

if(APPLE)
  if(NOT QT_MAC_USE_COCOA)
    message(FATAL_ERROR "Cocoa support is required")
  endif(NOT QT_MAC_USE_COCOA)
endif(APPLE)

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
  find_path(GSTREAMER_INCLUDE_DIRS gst/gst.h
    PATH_SUFFIXES gstreamer-0.10)
  find_library(GSTREAMER_LIBRARIES gstreamer-0.10)
  find_library(GSTREAMER_BASE_LIBRARIES gstbase-0.10)
  find_library(GLIB_LIBRARIES glib-2.0)
  find_library(GOBJECT_LIBRARIES gobject-2.0)
else(WIN32)
  pkg_check_modules(TAGLIB REQUIRED taglib>=1.6)
  pkg_check_modules(GSTREAMER gstreamer-0.10)
  pkg_check_modules(GSTREAMER_BASE gstreamer-base-0.10)
  pkg_check_modules(LIBVLC libvlc)
  pkg_check_modules(LIBXINE libxine)
endif(WIN32)

find_library(LASTFM_LIBRARIES lastfm)
find_path(LASTFM_INCLUDE_DIRS lastfm/ws.h)

find_library(PROJECTM_LIBRARIES projectM)
find_path(PROJECTM_INCLUDE_DIRS libprojectM/projectM.hpp)

if (APPLE)
  find_library(GROWL Growl)
  find_library(SPARKLE Sparkle)
endif (APPLE)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions(-DNDEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

# Set up definitions and paths
add_definitions(${QT_DEFINITIONS})
link_directories(${TAGLIB_LIBRARY_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

# Don't try to use Phonon if its include directory couldn't be found.
# For some reason this works differently to the rest of Qt...
if (NOT ${QT_PHONON_INCLUDE_DIR})
  set (QT_USE_PHONON 0)
endif (NOT ${QT_PHONON_INCLUDE_DIR})

include(${QT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GLIBCONFIG_INCLUDE_DIRS})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${LASTFM_INCLUDE_DIRS})
include_directories("3rdparty/qsqlite")
include_directories("3rdparty/universalchardet")

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
add_definitions(-DQT_NO_CAST_TO_ASCII)

# Translations stuff
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE)
  message(FATAL_ERROR "Could not find xgettext executable")
endif(NOT GETTEXT_XGETTEXT_EXECUTABLE)

# Subdirectories
add_subdirectory(3rdparty/qtsingleapplication)
add_subdirectory(3rdparty/qtiocompressor)
add_subdirectory(src)
if (NOT APPLE)
  add_subdirectory(3rdparty/qxt)
endif (NOT APPLE)
add_subdirectory(3rdparty/universalchardet)
add_subdirectory(tests)
add_subdirectory(dist)

if (WIN32 OR APPLE)
  message(STATUS "Building static qsqlite plugin")
  add_definitions(-DQT_STATICPLUGIN)
  add_subdirectory(3rdparty/qsqlite)
endif(WIN32 OR APPLE)

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
