cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0011 OLD)

include(CheckCXXCompilerFlag)
include(FindPkgConfig)
include(cmake/C++11Compat.cmake)
include(cmake/Summary.cmake)
include(cmake/Version.cmake)
include(cmake/Deb.cmake)
include(cmake/SpotifyVersion.cmake)
include(cmake/OptionalSource.cmake)
include(cmake/Format.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (CMAKE_CXX_COMPILER MATCHES ".*clang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-uninitialized")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --stdlib=libc++")
endif ()

find_program(CCACHE_EXECUTABLE NAMES ccache)
if (CCACHE_EXECUTABLE)
  message(STATUS "ccache found: will be used for compilation and linkage")
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_EXECUTABLE})
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_EXECUTABLE})
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(LINUX ON)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
  set(FREEBSD ON)
endif()

if(APPLE)
  if(NOT QT_MAC_USE_COCOA)
    message(FATAL_ERROR "Cocoa support is required")
  endif(NOT QT_MAC_USE_COCOA)
endif(APPLE)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL)
if(OPENGL_FOUND)
  set(HAVE_OPENGL ON)
endif(OPENGL_FOUND)
find_package(Boost REQUIRED)
find_package(Gettext REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)
find_package(FFTW3)
find_package(ALSA)
pkg_check_modules(DBUS dbus-1)
if (NOT APPLE)
  find_package(X11)
endif()
if(X11_FOUND)
  set(HAVE_X11 ON)
endif()

find_library(PROTOBUF_STATIC_LIBRARY libprotobuf.a libprotobuf)

pkg_check_modules(CDIO libcdio)
pkg_check_modules(CHROMAPRINT REQUIRED libchromaprint)
pkg_search_module(CRYPTOPP cryptopp libcrypto++)
pkg_check_modules(GIO gio-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_AUDIO REQUIRED gstreamer-audio-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED gstreamer-base-1.0)
pkg_check_modules(GSTREAMER_TAG REQUIRED gstreamer-tag-1.0)
pkg_check_modules(GSTREAMER_PBUTILS REQUIRED gstreamer-pbutils-1.0)
pkg_check_modules(LIBGPOD libgpod-1.0>=0.7.92)
pkg_check_modules(LIBMTP libmtp>=1.0)
pkg_check_modules(LIBMYGPO_QT libmygpo-qt>=1.0.9)
pkg_check_modules(LIBPULSE libpulse)
pkg_check_modules(LIBXML libxml-2.0)
pkg_check_modules(QJSON REQUIRED QJson)
pkg_check_modules(LIBSPOTIFY libspotify>=12.1.45)
pkg_check_modules(TAGLIB taglib)

if (WIN32)
  find_package(ZLIB REQUIRED)
  find_library(QTSPARKLE_LIBRARIES qtsparkle)
endif (WIN32)

find_library(LASTFM_LIBRARIES lastfm)
find_path(LASTFM_INCLUDE_DIRS lastfm/ws.h)
find_path(LASTFM1_INCLUDE_DIRS lastfm/Track.h)

find_path(SPARSEHASH_INCLUDE_DIRS google/sparsetable)

# QT
find_package(Qt4 4.8.1 REQUIRED QtCore QtGui QtSql QtNetwork QtXml)
if (DBUS_FOUND)
  find_package(Qt4 REQUIRED QtDbus)
endif ()
if (OPENGL_FOUND)
  find_package(Qt4 REQUIRED QtOpenGL)
endif(OPENGL_FOUND)

# We can include the Qt definitions now
include(${QT_USE_FILE})

# Find Qt's lconvert binary.  Try qt's binary dir first, fall back to looking in PATH
find_program(QT_LCONVERT_EXECUTABLE NAMES lconvert lconvert-qt4 PATHS ${QT_BINARY_DIR} NO_DEFAULT_PATH)
find_program(QT_LCONVERT_EXECUTABLE NAMES lconvert lconvert-qt4)

# Only use system taglib if it's greater than 1.11.1
# There is a bug in version 1.11.1 corrupting Ogg files, see: https://github.com/taglib/taglib/issues/864
# If you decide to use the systems taglib, make sure it has been patched with the following commit:
# https://github.com/taglib/taglib/commit/9336c82da3a04552168f208cd7a5fa4646701ea4
# The current taglib in 3rdparty also has the following features used by Clementine:
# - Audio file detection by content.
#
if (TAGLIB_VERSION VERSION_GREATER 1.11.1 OR WIN32)
  option(USE_SYSTEM_TAGLIB "Use system taglib" ON)
else()
  option(USE_SYSTEM_TAGLIB "Use system taglib" OFF)
endif()

if (TAGLIB_FOUND AND USE_SYSTEM_TAGLIB)
  if (TAGLIB_VERSION VERSION_GREATER 1.11.1 OR WIN32)
    message(STATUS "Using system taglib library")
  else()
    message(WARNING "Using system taglib library. Version 1.11.1 or less has a bug corrupting Ogg files, make sure your systems version has been patched!")
  endif()
  set(CMAKE_REQUIRED_INCLUDES "${TAGLIB_INCLUDE_DIRS}")
  set(CMAKE_REQUIRED_LIBRARIES "${TAGLIB_LIBRARIES}")
  check_cxx_source_compiles("#include <opusfile.h>
      int main() { char *s; TagLib::Ogg::Opus::File opusfile(s); return 0;}" TAGLIB_HAS_OPUS)
  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_LIBRARIES)
else()
  message(STATUS "Using builtin taglib library")
  set(TAGLIB_VERSION 1.11.1)
  set(TAGLIB_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/3rdparty/taglib/headers/taglib/;${CMAKE_BINARY_DIR}/3rdparty/taglib/headers/")
  set(TAGLIB_LIBRARY_DIRS "")
  set(TAGLIB_LIBRARIES tag)
  set(TAGLIB_HAS_OPUS ON)
  add_subdirectory(3rdparty/utf8-cpp)
  add_subdirectory(3rdparty/taglib)
endif()

if(LASTFM_INCLUDE_DIRS AND LASTFM1_INCLUDE_DIRS)
  set(HAVE_LIBLASTFM1 ON)
endif()

if (APPLE)
  find_library(SPARKLE Sparkle)
  find_library(LIBSPOTIFY libspotify)
  if(LIBSPOTIFY_FOUND)
    set(LIBSPOTIFY_INCLUDE_DIRS ${LIBSPOTIFY})
    set(LIBSPOTIFY_LIBRARIES ${LIBSPOTIFY})
  endif(LIBSPOTIFY_FOUND)

  add_subdirectory(3rdparty/SPMediaKeyTap)
  set(SPMEDIAKEYTAP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SPMediaKeyTap)
  set(SPMEDIAKEYTAP_LIBRARIES SPMediaKeyTap)
endif (APPLE)

find_package(Threads)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions(-DNDEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

# Set up definitions and paths
add_definitions(${QT_DEFINITIONS})
link_directories(${TAGLIB_LIBRARY_DIRS})
link_directories(${QJSON_LIBRARY_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})
include_directories(${QJSON_INCLUDE_DIRS})
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_APP_INCLUDE_DIRS})
include_directories(${GSTREAMER_AUDIO_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GSTREAMER_TAG_INCLUDE_DIRS})
include_directories(${GSTREAMER_PBUTILS_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GLIBCONFIG_INCLUDE_DIRS})
include_directories(${LIBXML_INCLUDE_DIRS})

if (WIN32)
  # RC compiler
  string(REPLACE "gcc" "windres" CMAKE_RC_COMPILER_INIT ${CMAKE_C_COMPILER})
  enable_language(RC)
  SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> <SOURCE> -I ${CMAKE_SOURCE_DIR}/dist/windows")
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
if(WIN32)
  option(ENABLE_WIN32_CONSOLE "Show the windows console even outside Debug mode" OFF)
endif(WIN32)

optional_component(BREAKPAD OFF "Crash reporting")

optional_component(GOOGLE_DRIVE ON "Google Drive support"
  DEPENDS "Google sparsehash" SPARSEHASH_INCLUDE_DIRS
  DEPENDS "Taglib 1.8" "TAGLIB_VERSION VERSION_GREATER 1.7.999"
)

optional_component(DROPBOX ON "Dropbox support"
  DEPENDS "Google sparsehash" SPARSEHASH_INCLUDE_DIRS
  DEPENDS "Taglib 1.8" "TAGLIB_VERSION VERSION_GREATER 1.7.999"
)

optional_component(SKYDRIVE ON "Skydrive support"
  DEPENDS "Google sparsehash" SPARSEHASH_INCLUDE_DIRS
  DEPENDS "Taglib 1.8" "TAGLIB_VERSION VERSION_GREATER 1.7.999"
)

optional_component(BOX ON "Box support"
  DEPENDS "Google sparsehash" SPARSEHASH_INCLUDE_DIRS
  DEPENDS "Taglib 1.8" "TAGLIB_VERSION VERSION_GREATER 1.7.999"
)

optional_component(SEAFILE ON "Seafile support"
  DEPENDS "Google sparsehash" SPARSEHASH_INCLUDE_DIRS
  DEPENDS "Taglib 1.8" "TAGLIB_VERSION VERSION_GREATER 1.7.999"
)

optional_component(AUDIOCD ON "Devices: Audio CD support"
  DEPENDS "libcdio" CDIO_FOUND
)

optional_component(LIBGPOD ON "Devices: iPod classic support"
  DEPENDS "libgpod" LIBGPOD_FOUND
)

optional_component(GIO ON "Devices: GIO device backend"
  DEPENDS "libgio" GIO_FOUND
  DEPENDS "Linux or Windows" "NOT APPLE"
)

optional_component(LIBMTP ON "Devices: MTP support"
  DEPENDS "libmtp" LIBMTP_FOUND
)

optional_component(LIBLASTFM ON "Last.fm support"
  DEPENDS "liblastfm" LASTFM_LIBRARIES LASTFM_INCLUDE_DIRS
)

optional_component(DBUS ON "D-Bus support"
  DEPENDS "D-Bus" DBUS_FOUND
)

optional_component(WIIMOTEDEV ON "Wiimote support"
  DEPENDS "D-Bus support" DBUS_FOUND
)

optional_component(DEVICEKIT ON "Devices: DeviceKit backend"
  DEPENDS "D-Bus support" DBUS_FOUND
)

optional_component(UDISKS2 ON "Devices: UDisks2 backend"
  DEPENDS "D-Bus support" DBUS_FOUND
)

if (CRYPTOPP_FOUND OR HAVE_SPOTIFY_BLOB)
  set(CRYPTOPP_OR_HAVE_SPOTIFY_BLOB ON)
endif()

optional_component(SPOTIFY ON "Spotify support"
  DEPENDS "cryptopp or spotify blob" CRYPTOPP_OR_HAVE_SPOTIFY_BLOB
)

optional_component(SPOTIFY_BLOB ON "Spotify support: non-GPL binary helper"
  DEPENDS "spotify support" HAVE_SPOTIFY
  DEPENDS "protobuf" PROTOBUF_FOUND PROTOBUF_PROTOC_EXECUTABLE
  DEPENDS "libspotify" LIBSPOTIFY_FOUND
)

optional_component(MOODBAR ON "Moodbar support"
  DEPENDS "fftw3" FFTW3_FOUND
)

optional_component(SPARKLE ON "Sparkle integration"
  DEPENDS "macOS" APPLE
  DEPENDS "Sparkle" SPARKLE
)

optional_component(ALSA ON "ALSA integration"
  DEPENDS "ALSA" ALSA_FOUND
)

optional_component(LIBPULSE ON "Pulse audio integration"
  DEPENDS "libpulse" LIBPULSE_FOUND
)

optional_component(VISUALISATIONS ON "Visualisations"
  DEPENDS "opengl" OPENGL_FOUND
)

optional_component(TRANSLATIONS ON "Translations"
  DEPENDS "gettext" GETTEXT_XGETTEXT_EXECUTABLE
  DEPENDS "lconvert" QT_LCONVERT_EXECUTABLE
)

option(USE_BUNDLE "Bundle macos/windows dependencies" ON)
if (APPLE AND USE_BUNDLE AND NOT USE_BUNDLE_DIR)
  set(USE_BUNDLE_DIR "../PlugIns")
endif()

if(CRYPTOPP_FOUND)
  set(HAVE_CRYPTOPP ON)
  if(HAVE_SPOTIFY)
    set(HAVE_SPOTIFY_DOWNLOADER ON)
  endif(HAVE_SPOTIFY)
endif(CRYPTOPP_FOUND)

# Remove GLU and GL from the link line - they're not really required
# and don't exist on my mingw toolchain
list(REMOVE_ITEM QT_LIBRARIES "-lGLU -lGL")

option(USE_SYSTEM_PROJECTM "Don't set this option unless your system projectM library has been compiled with the Clementine patches in 3rdparty" OFF)
option(BUNDLE_PROJECTM_PRESETS "Install Clementine's own copies of libprojectm presets - disable this if you want to use a system package instead" ON)

if(HAVE_VISUALISATIONS)
  # When/if upstream accepts our patches then these options can be used to link
  # to system installed projectM instead.
  if(USE_SYSTEM_PROJECTM)
    pkg_check_modules(LIBPROJECTM libprojectM)
  else(USE_SYSTEM_PROJECTM)
    add_subdirectory(3rdparty/libprojectm)
    set(LIBPROJECTM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libprojectm)
    set(LIBPROJECTM_LIBRARIES projectM)
  endif(USE_SYSTEM_PROJECTM)
endif(HAVE_VISUALISATIONS)


# Build our copy of QSqlLiteDriver.
# We do this because we can't guarantee that the driver shipped with Qt exposes the
# raw sqlite3_ functions required for FTS support. This way we know that those symbols
# exist at compile-time and that our code links to the same sqlite library as the
# Qt driver.
add_subdirectory(3rdparty/qsqlite)
include_directories("3rdparty/qsqlite")

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

# Use the system's sha2 if it's available.
find_path(SHA2_INCLUDE_DIRS sha2.h)
find_library(SHA2_LIBRARIES sha2)
if(SHA2_LIBRARIES AND SHA2_INCLUDE_DIRS)
  message(STATUS "Using system sha2 library")
  set(USE_SYSTEM_SHA2 ON)
else()
  message(STATUS "Using builtin sha2 library")
  set(USE_SYSTEM_SHA2 OFF)
  add_subdirectory(3rdparty/sha2)
  set(SHA2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sha2)
  set(SHA2_LIBRARIES sha2)
endif()

# Use the system libmygpo-qt if a recent enough version was found
if(LIBMYGPO_QT_FOUND)
  set(MYGPOQT_LIBRARIES ${LIBMYGPO_QT_LIBRARIES})
  set(MYGPOQT_INCLUDE_DIRS ${LIBMYGPO_QT_INCLUDE_DIRS})
else()
  add_definitions(-DMYGPO_EXPORT=)
  add_subdirectory(3rdparty/libmygpo-qt)
  set(MYGPOQT_LIBRARIES mygpo-qt)
  set(MYGPOQT_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/3rdparty/libmygpo-qt/)
endif()

# Qocoa
add_subdirectory(3rdparty/qocoa)

# Subdirectories
add_subdirectory(src)
if (WIN32)
  add_subdirectory(3rdparty/qtwin)
  add_subdirectory(3rdparty/tinysvcmdns)
endif (WIN32)
add_subdirectory(tests)
add_subdirectory(dist)
add_subdirectory(ext/libclementine-common)
add_subdirectory(ext/libclementine-tagreader)
add_subdirectory(ext/clementine-tagreader)
add_subdirectory(ext/libclementine-remote)
add_subdirectory(ext/libclementine-spotifyblob)

option(WITH_DEBIAN OFF)
if(WITH_DEBIAN)
  add_subdirectory(debian)
endif(WITH_DEBIAN)

if(HAVE_BREAKPAD)
  add_subdirectory(3rdparty/google-breakpad)
endif(HAVE_BREAKPAD)

if(HAVE_SPOTIFY_BLOB)
  add_subdirectory(ext/clementine-spotifyblob)
endif(HAVE_SPOTIFY_BLOB)

if(HAVE_MOODBAR)
  add_subdirectory(gst/moodbar)
endif()

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Show a summary of what we have enabled
summary_show()
