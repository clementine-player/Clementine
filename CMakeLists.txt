cmake_minimum_required(VERSION 2.6)

include(CheckCXXCompilerFlag)
include(FindPkgConfig)
include(cmake/Summary.cmake)
include(cmake/Version.cmake)
include(cmake/Deb.cmake)
include(cmake/Rpm.cmake)

find_package(Qt4 4.5.0 REQUIRED QtCore QtGui QtOpenGL QtSql QtNetwork QtXml)
if(UNIX AND NOT APPLE)
  find_package(Qt4 REQUIRED QtDbus)
endif(UNIX AND NOT APPLE)

if(NOT APPLE)
  find_package(Qt4 COMPONENTS Phonon QtWebKit)
endif(NOT APPLE)

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
find_package(PkgConfig REQUIRED)

pkg_check_modules(TAGLIB REQUIRED taglib>=1.6)
pkg_check_modules(GSTREAMER gstreamer-0.10)
pkg_check_modules(GSTREAMER_BASE gstreamer-base-0.10)
pkg_check_modules(LIBVLC libvlc)
pkg_check_modules(LIBXINE libxine)
pkg_check_modules(GLIB glib-2.0)
pkg_check_modules(LIBXML libxml-2.0)
pkg_check_modules(GOBJECT gobject-2.0)
pkg_check_modules(LIBGPOD libgpod-1.0>=0.7.92)
pkg_check_modules(GIO gio-2.0)
pkg_check_modules(IMOBILEDEVICE libimobiledevice-1.0)
pkg_check_modules(PLIST libplist)
pkg_check_modules(USBMUXD libusbmuxd)
pkg_check_modules(LIBMTP libmtp>=1.0)
pkg_check_modules(INDICATEQT indicate-qt)

if (WIN32)
  find_package(ZLIB REQUIRED)
  find_library(MSWMDM_LIBRARIES mswmdm)
  find_library(SAC_SHIM_LIBRARIES sac_shim)
endif (WIN32)

find_library(LASTFM_LIBRARIES lastfm)
find_path(LASTFM_INCLUDE_DIRS lastfm/ws.h)

if (LASTFM_LIBRARIES AND LASTFM_INCLUDE_DIRS)
  set(HAVE_LIBLASTFM ON)
endif (LASTFM_LIBRARIES AND LASTFM_INCLUDE_DIRS)

if (APPLE)
  find_library(GROWL Growl)
  option(ENABLE_SPARKLE "Sparkle updating" ON)
  find_library(SPARKLE Sparkle)
  if (ENABLE_SPARKLE AND SPARKLE)
    set(HAVE_SPARKLE ON)
  endif (ENABLE_SPARKLE AND SPARKLE)
  # Uses Darwin kernel version.
  # 9.8.0  -> 10.5/Leopard
  # 10.4.0 -> 10.6/Snow Leopard
  string(REGEX MATCH "[0-9]+" DARWIN_VERSION ${CMAKE_HOST_SYSTEM_VERSION})
  if (DARWIN_VERSION GREATER 9)
    SET(SNOW_LEOPARD 1)
  elseif (DARWIN_VERSION GREATER 8)
    SET(LEOPARD 1)
  endif (DARWIN_VERSION GREATER 9)
endif (APPLE)

find_package(Threads)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions(-DNDEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

# Set up definitions and paths
add_definitions(${QT_DEFINITIONS})
link_directories(${TAGLIB_LIBRARY_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

# Don't try to use Phonon or webkit if their include directories couldn't be found.
if (NOT QT_PHONON_INCLUDE_DIR)
  set (QT_USE_PHONON 0)
endif (NOT QT_PHONON_INCLUDE_DIR)

if (NOT QT_QTWEBKIT_INCLUDE_DIR)
  set (QT_USE_QTWEBKIT 0)
endif (NOT QT_QTWEBKIT_INCLUDE_DIR)

include(${QT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GLIBCONFIG_INCLUDE_DIRS})
include_directories(${LIBXML_INCLUDE_DIRS})
if (HAVE_LIBLASTFM)
  include_directories(${LASTFM_INCLUDE_DIRS})
endif (HAVE_LIBLASTFM)
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

add_definitions(-DQT_NO_CAST_TO_ASCII)

# Translations stuff
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE)
  message(FATAL_ERROR "Could not find xgettext executable")
endif(NOT GETTEXT_XGETTEXT_EXECUTABLE)

# Optional bits
option(ENABLE_LIBGPOD "iPod classic support" ON)
option(ENABLE_IMOBILEDEVICE "iPod Touch, iPhone, iPad support" ON)
option(ENABLE_LIBMTP "MTP support" ON)
option(ENABLE_GIO "GIO backend" ON)
option(ENABLE_VISUALISATIONS "Use libprojectm visualisations" ON)
option(BUNDLE_PROJECTM_PRESETS "Install Clementine's own copies of libprojectm presets - disable this if you want to use a system package instead" ON)
option(ENABLE_SOUNDMENU "Add Clementine to the Gnome sound menu" ON)

if(NOT APPLE AND NOT WIN32)
  option(ENABLE_WIIMOTEDEV "Enable Wii remote support in Clementine" ON)
endif(NOT APPLE AND NOT WIN32)

if(ENABLE_LIBGPOD AND LIBGPOD_FOUND)
  set(HAVE_LIBGPOD ON)
endif(ENABLE_LIBGPOD AND LIBGPOD_FOUND)

if(ENABLE_GIO AND GIO_FOUND AND NOT APPLE)
  set(HAVE_GIO ON)
endif(ENABLE_GIO AND GIO_FOUND AND NOT APPLE)

if(ENABLE_IMOBILEDEVICE AND IMOBILEDEVICE_FOUND AND PLIST_FOUND AND HAVE_LIBGPOD AND USBMUXD_FOUND)
  set(HAVE_IMOBILEDEVICE ON)
endif(ENABLE_IMOBILEDEVICE AND IMOBILEDEVICE_FOUND AND PLIST_FOUND AND HAVE_LIBGPOD AND USBMUXD_FOUND)

if(ENABLE_LIBMTP AND LIBMTP_FOUND)
  set(HAVE_LIBMTP ON)
endif(ENABLE_LIBMTP AND LIBMTP_FOUND)

if(ENABLE_SOUNDMENU AND INDICATEQT_FOUND)
  set(HAVE_LIBINDICATE ON)
endif(ENABLE_SOUNDMENU AND INDICATEQT_FOUND)

if(ENABLE_VISUALISATIONS)
  # When/if upstream accepts our patches then these options can be used to link
  # to system installed projectM instead.
  option(USE_SYSTEM_PROJECTM "Don't set this option unless your system projectM library has been compiled with the Clementine patches in 3rdparty" OFF)
  if(USE_SYSTEM_PROJECTM)
    pkg_check_modules(LIBPROJECTM libprojectM)
  else(USE_SYSTEM_PROJECTM)
    add_subdirectory(3rdparty/libprojectm)
    set(LIBPROJECTM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libprojectm)
    set(LIBPROJECTM_LIBRARIES projectM)
  endif(USE_SYSTEM_PROJECTM)
endif(ENABLE_VISUALISATIONS)

# We compile our own qsqlite3 by default now, because in Qt 4.7 the symbols we
# need from it are private.
option(STATIC_SQLITE "Compile and use a static sqlite3 library" ON)

set(HAVE_STATIC_SQLITE ${STATIC_SQLITE})
if(STATIC_SQLITE)
  message(STATUS "Building static qsqlite plugin")
  add_definitions(-DQT_STATICPLUGIN)
  add_subdirectory(3rdparty/qsqlite)
  include_directories("3rdparty/qsqlite")
endif(STATIC_SQLITE)

# When/if upstream accepts our patches then these options can be used to link
# to system installed qtsingleapplication instead.
option(USE_SYSTEM_QTSINGLEAPPLICATION "Don't set this option unless your system QtSingleApplication library has been compiled with the Clementine patches in 3rdparty" OFF)
if(USE_SYSTEM_QTSINGLEAPPLICATION)
  find_path(QTSINGLEAPPLICATION_INCLUDE_DIRS qtsingleapplication.h PATH_SUFFIXES QtSolutions)
  find_library(QTSINGLEAPPLICATION_LIBRARIES QtSolutions_SingleApplication-2.6)
  find_library(QTSINGLECOREAPPLICATION_LIBRARIES QtSolutions_SingleCoreApplication-2.6)
else(USE_SYSTEM_QTSINGLEAPPLICATION)
  add_subdirectory(3rdparty/qtsingleapplication)
  set(QTSINGLEAPPLICATION_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qtsingleapplication)
  set(QTSINGLEAPPLICATION_LIBRARIES qtsingleapplication)
endif(USE_SYSTEM_QTSINGLEAPPLICATION)

# QtIoCompressor isn't patched, so we can use a system version if it's
# available
find_path(QTIOCOMPRESSOR_INCLUDE_DIRS qtiocompressor.h PATH_SUFFIXES QtSolutions)
find_library(QTIOCOMPRESSOR_LIBRARIES QtSolutions_IOCompressor-2.3)
if(NOT QTIOCOMPRESSOR_INCLUDE_DIRS OR NOT QTIOCOMPRESSOR_LIBRARIES)
  add_subdirectory(3rdparty/qtiocompressor)
  set(QTIOCOMPRESSOR_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qtiocompressor)
  set(QTIOCOMPRESSOR_LIBRARIES qtiocompressor)
endif(NOT QTIOCOMPRESSOR_INCLUDE_DIRS OR NOT QTIOCOMPRESSOR_LIBRARIES)

# When/if upstream accepts our or reimplement our patches then these options can be
# used to link to system installed qxt instead.
option(USE_SYSTEM_QXT "Don't set this option unless your system Qxt library has been compiled with the Clementine patches in 3rdparty" OFF)
if (USE_SYSTEM_QXT)
  find_path(QXTCORE_INCLUDE_DIRS qxtglobal.h PATH_SUFFIXES QxtCore)
  find_path(QXTGUI_INCLUDE_DIRS qxtglobalshortcut.h PATH_SUFFIXES QxtGui)
  set(QXT_INCLUDE_DIRS ${QXTCORE_INCLUDE_DIRS} ${QXTGUI_INCLUDE_DIRS})
  # We only need its header. We don't need to link to QxtCore.
  find_library(QXT_LIBRARIES QxtGui)
else (USE_SYSTEM_QXT)
  add_definitions(-DQXT_STATIC -DBUILD_QXT_GUI -DBUILD_QXT_CORE)
  set(QXT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qxt)
  set(QXT_LIBRARIES qxt)
  if (NOT APPLE)
    add_subdirectory(3rdparty/qxt)
  endif (NOT APPLE)
endif (USE_SYSTEM_QXT)

# Use system libechonest if it's available
find_path(ECHONEST_INCLUDE_DIRS echonest/echonest_export.h)
find_library(ECHONEST_LIBRARIES echonest)
if(NOT ECHONEST_INCLUDE_DIRS OR NOT ECHONEST_LIBRARIES)
  add_subdirectory(3rdparty/libechonest)
  set(ECHONEST_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/3rdparty/libechonest)
  set(ECHONEST_LIBRARIES echonest)
endif(NOT ECHONEST_INCLUDE_DIRS OR NOT ECHONEST_LIBRARIES)


# Subdirectories
add_subdirectory(src)
if (WIN32)
  add_subdirectory(3rdparty/qtwin)
endif (WIN32)
add_subdirectory(3rdparty/universalchardet)
add_subdirectory(tests)
add_subdirectory(dist)
add_subdirectory(tools/ultimate_lyrics_parser)

option(WITH_DEBIAN OFF)
if(WITH_DEBIAN)
  add_subdirectory(debian)
endif(WITH_DEBIAN)

if(IMOBILEDEVICE_FOUND AND PLIST_FOUND)
  add_subdirectory(gstafcsrc)
endif(IMOBILEDEVICE_FOUND AND PLIST_FOUND)

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Show a summary of what we have enabled
summary_add("devices: iPod classic support" HAVE_LIBGPOD)
summary_add("devices: iPod Touch, iPhone, iPad support" HAVE_IMOBILEDEVICE)
summary_add("devices: MTP support" HAVE_LIBMTP)
summary_add("devices: GIO backend" HAVE_GIO)
summary_add("Gnome sound menu integration" HAVE_LIBINDICATE)
summary_add("Wiimote support" ENABLE_WIIMOTEDEV)
summary_add("Visualisations" ENABLE_VISUALISATIONS)
summary_add("Sparkle integration" HAVE_SPARKLE)
summary_add("Last.fm support" HAVE_LIBLASTFM)
summary_show()
