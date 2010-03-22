cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)

if (CMAKE_FIND_ROOT_PATH)
  # Help find the Qt headers if we're cross compiling (since we can't run qmake.exe)
  # CMAKE_FIND_ROOT_PATH should get set by your cmake toolchain file
  set(QT_HEADERS_DIR ${CMAKE_FIND_ROOT_PATH}/include)
  set(QT_LIBRARY_DIR ${CMAKE_FIND_ROOT_PATH}/lib)
  set(QT_PHONON_INCLUDE_DIR ${CMAKE_FIND_ROOT_PATH}/include/phonon)
endif (CMAKE_FIND_ROOT_PATH)

find_package(Qt4 REQUIRED)
set(QT_USE_QTOPENGL 1)
set(QT_USE_QTSQL 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTXML 1)
set(QT_USE_QTTEST 1)
if(UNIX AND NOT APPLE)
  set(QT_USE_QTDBUS 1)
endif(UNIX AND NOT APPLE)
if(WIN32)
  set(QT_USE_PHONON 1)
endif(WIN32)

find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)

find_path(SQLITE3_INCLUDE_DIRS "sqlite3.h")

if(WIN32)
  find_library(TAGLIB_LIBRARIES tag)
else(WIN32)
  pkg_check_modules(TAGLIB taglib)
  pkg_check_modules(XINE libxine)
  pkg_check_modules(LIBNOTIFY libnotify)
endif(WIN32)

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
include_directories(${SQLITE3_INCLUDE_DIRS})

# Remove GLU and GL from the link line - they're not really required
# and don't exist on my mingw toolchain
list(REMOVE_ITEM QT_LIBRARIES "-lGLU -lGL")

if (WIN32)
  # RC compiler
  string(REPLACE "gcc" "windres" CMAKE_RC_COMPILER_INIT ${CMAKE_C_COMPILER})
  enable_language(RC)
  SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> <SOURCE>")
endif(WIN32)

# Subdirectories
add_subdirectory(3rdparty/qtsingleapplication)
add_subdirectory(3rdparty/qxt)
add_subdirectory(src)

add_subdirectory(tests)

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
