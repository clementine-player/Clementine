cmake_minimum_required(VERSION 2.6)

include(CheckCXXCompilerFlag)
include(FindPkgConfig)
include(cmake/Summary.cmake)
include(cmake/Version.cmake)
include(cmake/Deb.cmake)
include(cmake/Rpm.cmake)
include(cmake/SpotifyVersion.cmake)

if (UNIX AND NOT APPLE)
  set(LINUX 1)
endif (UNIX AND NOT APPLE)

find_package(Qt4 4.5.0 REQUIRED QtCore QtGui QtOpenGL QtSql QtNetwork QtXml)
if (LINUX)
  option(ENABLE_DBUS "Enable D-Bus, MPRIS and native notifications. Required for DeviceKit and Wii remote support" ON)
  if(ENABLE_DBUS)
    find_package(Qt4 REQUIRED QtDbus)
    set(HAVE_DBUS ON)

    option(ENABLE_DEVICEKIT "DeviceKit backend" ON)
    if(ENABLE_DEVICEKIT)
      set(HAVE_DEVICEKIT ON)
    endif(ENABLE_DEVICEKIT)

    option(ENABLE_WIIMOTEDEV "Enable Wii remote support in Clementine" ON)
  endif(ENABLE_DBUS)
endif (LINUX)

if(NOT APPLE)
  find_package(Qt4 COMPONENTS QtWebKit)
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
find_package(Protobuf)

pkg_check_modules(TAGLIB REQUIRED taglib>=1.6)
pkg_check_modules(GSTREAMER gstreamer-0.10)
pkg_check_modules(GSTREAMER_BASE gstreamer-base-0.10)
pkg_check_modules(GSTREAMER_APP gstreamer-app-0.10)
pkg_check_modules(GSTREAMER_CDDA gstreamer-cdda-0.10)
pkg_check_modules(GSTREAMER_TAG gstreamer-tag-0.10)
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
pkg_check_modules(SPOTIFY libspotify>=0.0.8)
pkg_check_modules(CDIO libcdio)

if (WIN32)
  find_package(ZLIB REQUIRED)
  find_library(MSWMDM_LIBRARIES mswmdm)
  find_library(SAC_SHIM_LIBRARIES sac_shim)
  find_library(QTSPARKLE_LIBRARIES qtsparkle)
endif (WIN32)

find_library(LASTFM_LIBRARIES lastfm)
find_path(LASTFM_INCLUDE_DIRS lastfm/ws.h)

if (APPLE)
  find_library(GROWL Growl)

  option(ENABLE_SPARKLE "Sparkle updating" ON)
  find_library(SPARKLE Sparkle)
  if (ENABLE_SPARKLE AND SPARKLE)
    set(HAVE_SPARKLE ON)
  endif (ENABLE_SPARKLE AND SPARKLE)

  find_library(SPOTIFY libspotify)
  if (SPOTIFY)
    set (SPOTIFY_FOUND ON)
    set (SPOTIFY_INCLUDE_DIRS ${SPOTIFY})
    set (SPOTIFY_LIBRARIES ${SPOTIFY})
  endif (SPOTIFY)

  add_subdirectory(3rdparty/SPMediaKeyTap)
  set(SPMEDIAKEYTAP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SPMediaKeyTap)
  set(SPMEDIAKEYTAP_LIBRARIES SPMediaKeyTap)  

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

# Use protobuf-lite if it's available
find_library(PROTOBUF_LITE_LIBRARY protobuf-lite)
if(NOT PROTOBUF_LITE_LIBRARY)
  # Lucid doesn't have a .so symlink
  find_file(PROTOBUF_LITE_LIBRARY
    NAMES libprotobuf-lite.so.5
    PATH_SUFFIXES lib
  )
endif(NOT PROTOBUF_LITE_LIBRARY)

# Set up definitions and paths
add_definitions(${QT_DEFINITIONS})
link_directories(${TAGLIB_LIBRARY_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

# Don't try to use webkit if their include directories couldn't be found.
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

add_definitions(-DQT_NO_CAST_TO_ASCII -DQT_STRICT_ITERATORS)

# Translations stuff
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext PATHS /target/bin)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE)
  message(FATAL_ERROR "Could not find xgettext executable")
endif(NOT GETTEXT_XGETTEXT_EXECUTABLE)
find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge PATHS /target/bin)
if(NOT GETTEXT_MSGMERGE_EXECUTABLE)
  message(FATAL_ERROR "Could not find msgmerge executable")
endif(NOT GETTEXT_MSGMERGE_EXECUTABLE)
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt PATHS /target/bin)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
  message(FATAL_ERROR "Could not find msgfmt executable")
endif(NOT GETTEXT_MSGFMT_EXECUTABLE)

# Optional bits
option(ENABLE_LIBGPOD "iPod classic support" ON)
option(ENABLE_IMOBILEDEVICE "iPod Touch, iPhone, iPad support" ON)
option(ENABLE_LIBMTP "MTP support" ON)
option(ENABLE_GIO "GIO backend" ON)
option(ENABLE_AUDIOCD "Audio CD support" ON)
option(ENABLE_VISUALISATIONS "Use libprojectm visualisations" ON)
option(BUNDLE_PROJECTM_PRESETS "Install Clementine's own copies of libprojectm presets - disable this if you want to use a system package instead" ON)
option(ENABLE_SOUNDMENU "Add Clementine to the Gnome sound menu" ON)
option(ENABLE_LIBLASTFM "Use liblastfm for fetching song info, scrobbling and radio streams" ON)
option(ENABLE_REMOTE "Enable support for using remote controls with Clementine" OFF)
option(ENABLE_BREAKPAD "Enable crash reporting" OFF)
option(ENABLE_SPOTIFY_BLOB "Build the spotify non-GPL binary" ON)
option(ENABLE_SPOTIFY "Enable spotify support" ON)

if(WIN32)
  option(ENABLE_WIN32_CONSOLE "Show the windows console even outside Debug mode" OFF)
endif(WIN32)

if(ENABLE_LIBGPOD AND LIBGPOD_FOUND)
  set(HAVE_LIBGPOD ON)
endif(ENABLE_LIBGPOD AND LIBGPOD_FOUND)

if(ENABLE_GIO AND GIO_FOUND AND NOT APPLE)
  set(HAVE_GIO ON)
endif(ENABLE_GIO AND GIO_FOUND AND NOT APPLE)

if(ENABLE_AUDIOCD AND CDIO_FOUND)
  set(HAVE_AUDIOCD ON)
endif(ENABLE_AUDIOCD AND CDIO_FOUND)

if(ENABLE_IMOBILEDEVICE AND IMOBILEDEVICE_FOUND AND PLIST_FOUND AND HAVE_LIBGPOD AND USBMUXD_FOUND)
  set(HAVE_IMOBILEDEVICE ON)
endif(ENABLE_IMOBILEDEVICE AND IMOBILEDEVICE_FOUND AND PLIST_FOUND AND HAVE_LIBGPOD AND USBMUXD_FOUND)

if(ENABLE_LIBMTP AND LIBMTP_FOUND)
  set(HAVE_LIBMTP ON)
endif(ENABLE_LIBMTP AND LIBMTP_FOUND)

if(ENABLE_SOUNDMENU AND INDICATEQT_FOUND)
  set(HAVE_LIBINDICATE ON)
endif(ENABLE_SOUNDMENU AND INDICATEQT_FOUND)

if(ENABLE_LIBLASTFM AND LASTFM_LIBRARIES AND LASTFM_INCLUDE_DIRS)
  set(HAVE_LIBLASTFM ON)
endif(ENABLE_LIBLASTFM AND LASTFM_LIBRARIES AND LASTFM_INCLUDE_DIRS)

if(ENABLE_WIIMOTEDEV AND HAVE_DBUS)
  set(HAVE_WIIMOTEDEV ON)
endif(ENABLE_WIIMOTEDEV AND HAVE_DBUS)

if(ENABLE_BREAKPAD)
  set(HAVE_BREAKPAD ON)
endif(ENABLE_BREAKPAD)

if(ENABLE_SPOTIFY AND PROTOBUF_FOUND AND PROTOBUF_PROTOC_EXECUTABLE)
  set(HAVE_SPOTIFY ON)
endif(ENABLE_SPOTIFY AND PROTOBUF_FOUND AND PROTOBUF_PROTOC_EXECUTABLE)

if(ENABLE_SPOTIFY_BLOB AND PROTOBUF_FOUND AND PROTOBUF_PROTOC_EXECUTABLE AND SPOTIFY_FOUND)
  set(HAVE_SPOTIFY_BLOB ON)
endif(ENABLE_SPOTIFY_BLOB AND PROTOBUF_FOUND AND PROTOBUF_PROTOC_EXECUTABLE AND SPOTIFY_FOUND)


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

pkg_check_modules(GNUTLS gnutls)
if(GNUTLS_LIBRARIES)
  set(HAVE_GNUTLS 1)
endif(GNUTLS_LIBRARIES)
pkg_check_modules(PJSIP libpjproject>=1.8)
if(PJSIP_LIBRARIES)
  set(HAVE_PJSIP 1)
endif(PJSIP_LIBRARIES)

if(ENABLE_REMOTE AND HAVE_GNUTLS AND HAVE_PJSIP)
  set(HAVE_REMOTE ON)
  add_subdirectory(3rdparty/gloox)
  add_subdirectory(3rdparty/libxrme)
  add_subdirectory(3rdparty/libportfwd)
  include_directories(3rdparty)
  include_directories(3rdparty/libxrme)
  include_directories(3rdparty/libportfwd/include)
endif(ENABLE_REMOTE AND HAVE_GNUTLS AND HAVE_PJSIP)

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

# Use system gmock if it's available
# We need to look for both gmock and gtest
find_path(GMOCK_INCLUDE_DIRS gmock/gmock.h)
find_library(GMOCK_LIBRARIES gmock)
if(GMOCK_INCLUDE_DIRS)
  find_path(GTEST_INCLUDE_DIRS gtest/gtest.h)
  find_library(GTEST_LIBRARIES gtest)
  if(GTEST_INCLUDE_DIRS)
    set(USE_SYSTEM_GMOCK 1)
    set(GMOCK_LIBRARIES ${GMOCK_LIBRARIES} ${GTEST_LIBRARIES})
  endif(GTEST_INCLUDE_DIRS)
endif(GMOCK_INCLUDE_DIRS)

# Subdirectories
add_subdirectory(src)
if (WIN32)
  add_subdirectory(3rdparty/qtwin)
endif (WIN32)
add_subdirectory(3rdparty/echoprint-codegen)
add_subdirectory(3rdparty/sha2)
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

if(HAVE_BREAKPAD)
  add_subdirectory(3rdparty/google-breakpad)
endif(HAVE_BREAKPAD)

if(HAVE_SPOTIFY)
  add_subdirectory(spotifyblob/common)
endif(HAVE_SPOTIFY)

if(HAVE_SPOTIFY_BLOB)
  add_subdirectory(spotifyblob/blob)
endif(HAVE_SPOTIFY_BLOB)

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Show a summary of what we have enabled
summary_add("Crash reporting" HAVE_BREAKPAD)
summary_add("D-Bus support" HAVE_DBUS)
summary_add("Devices: Audio CD support" HAVE_AUDIOCD)
summary_add("Devices: DeviceKit backend" HAVE_DEVICEKIT)
summary_add("Devices: iPod classic support" HAVE_LIBGPOD)
summary_add("Devices: iPod Touch, iPhone, iPad support" HAVE_IMOBILEDEVICE)
summary_add("Devices: MTP support" HAVE_LIBMTP)
summary_add("Devices: GIO backend" HAVE_GIO)
summary_add("Gnome sound menu integration" HAVE_LIBINDICATE)
summary_add("Last.fm support" HAVE_LIBLASTFM)
summary_add("Spotify support: core code" HAVE_SPOTIFY)
summary_add("Spotify support: non-GPL binary helper" HAVE_SPOTIFY_BLOB)
summary_add("Visualisations" ENABLE_VISUALISATIONS)
summary_add("Wiimote support" HAVE_WIIMOTEDEV)
summary_add("(Mac OS X) Sparkle integration" HAVE_SPARKLE)
summary_add("(unstable) Remote control support" HAVE_REMOTE)
summary_show()
