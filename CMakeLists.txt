project(clementine)

cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0053 OLD)

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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

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
if (NOT APPLE)
  find_package(X11)
endif()
if(X11_FOUND)
  set(HAVE_X11 ON)
endif()

find_library(PROTOBUF_STATIC_LIBRARY libprotobuf.a libprotobuf)

pkg_check_modules(CDIO libcdio)
pkg_check_modules(CHROMAPRINT REQUIRED libchromaprint)
pkg_search_module(CRYPTOPP libcryptopp libcrypto++)
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
pkg_check_modules(LIBMYGPO_QT5 libmygpo-qt5>=1.0.9)
pkg_check_modules(LIBPULSE libpulse)
pkg_check_modules(LIBXML libxml-2.0)
pkg_check_modules(LIBSPOTIFY libspotify>=12.1.45)
pkg_check_modules(TAGLIB taglib)

if (WIN32)
  find_package(ZLIB REQUIRED)
endif (WIN32)

find_library(LASTFM5_LIBRARIES lastfm5)
find_path(LASTFM5_INCLUDE_DIRS lastfm5/ws.h)
find_path(LASTFM51_INCLUDE_DIRS lastfm5/Track.h)

find_path(SPARSEHASH_INCLUDE_DIRS google/sparsetable)

# QT
set(QT_MIN_VERSION 5.6.0)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Concurrent Widgets Network Sql Xml OpenGL Test)
if(HAVE_X11)
  find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS X11Extras)
  find_package(Qt5DBus ${QT_MIN_VERSION})
endif()
if(APPLE)
  find_package(Qt5 REQUIRED COMPONENTS MacExtras)
endif()
if(WIN32)
  find_package(Qt5 REQUIRED COMPONENTS WinExtras)
endif()

set(QT_LIBRARIES Qt5::Core Qt5::Concurrent Qt5::Widgets Qt5::Network Qt5::Sql Qt5::Xml Qt5::OpenGL)

if(Qt5DBus_FOUND)
  set(QT_LIBRARIES ${QT_LIBRARIES} Qt5::DBus)
  get_target_property(QT_DBUSXML2CPP_EXECUTABLE Qt5::qdbusxml2cpp LOCATION)
endif()
if(HAVE_X11)
  set(QT_LIBRARIES ${QT_LIBRARIES} Qt5::X11Extras)
endif()
if(APPLE)
  set(QT_LIBRARIES ${QT_LIBRARIES} Qt5::MacExtras)
endif()
if(WIN32)
  set(QT_LIBRARIES ${QT_LIBRARIES} Qt5::WinExtras)
endif()

find_package(Qt5LinguistTools CONFIG)
if (Qt5LinguistTools_FOUND)
  set(QT_LCONVERT_EXECUTABLE Qt5::lconvert)
endif()


# Only use system taglib if it's greater than 1.11.1
# There is a bug in version 1.11.1 corrupting Ogg files, see: https://github.com/taglib/taglib/issues/864
# If you decide to use the systems taglib, make sure it has been patched with the following commit:
# https://github.com/taglib/taglib/commit/9336c82da3a04552168f208cd7a5fa4646701ea4
# The current taglib in 3rdparty also has the following features used by Clementine:
# - Audio file detection by content.
#
if (TAGLIB_VERSION VERSION_GREATER 1.11.1)
  option(USE_SYSTEM_TAGLIB "Use system taglib" ON)
else()
  option(USE_SYSTEM_TAGLIB "Use system taglib" OFF)
endif()

if (TAGLIB_FOUND AND USE_SYSTEM_TAGLIB)
  if (TAGLIB_VERSION VERSION_GREATER 1.11.1)
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
  add_definitions(-DTAGLIB_STATIC)
endif()

if(LASTFM5_INCLUDE_DIRS AND LASTFM51_INCLUDE_DIRS)
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
  add_definitions(-DQT_NO_WARNING_OUTPUT)
endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

# Set up definitions and paths
add_definitions(${QT_DEFINITIONS})
link_directories(${TAGLIB_LIBRARY_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

# Top level headers and generated headers
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})
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
  DEPENDS "liblastfm" LASTFM5_LIBRARIES LASTFM5_INCLUDE_DIRS
)

optional_component(DBUS ON "D-Bus support"
  DEPENDS "Qt5DBus" Qt5DBus_FOUND
)

optional_component(WIIMOTEDEV ON "Wiimote support"
  DEPENDS "D-Bus support" Qt5DBus_FOUND
)

optional_component(UDISKS2 ON "Devices: UDisks2 backend"
  DEPENDS "D-Bus support" Qt5DBus_FOUND
)

optional_component(SPOTIFY_BLOB ON "Spotify support: non-GPL binary helper"
  DEPENDS "protobuf" PROTOBUF_FOUND PROTOBUF_PROTOC_EXECUTABLE
  DEPENDS "libspotify" LIBSPOTIFY_FOUND
)

if (CRYPTOPP_FOUND OR HAVE_SPOTIFY_BLOB)
  set(CRYPTOPP_OR_HAVE_SPOTIFY_BLOB ON)
endif()

optional_component(SPOTIFY ON "Spotify support"
  DEPENDS "cryptopp or spotify blob" CRYPTOPP_OR_HAVE_SPOTIFY_BLOB
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

optional_component(BACKGROUND_STREAMS ON "Background streams")

optional_component(TRANSLATIONS ON "Translations"
  DEPENDS "gettext" GETTEXT_XGETTEXT_EXECUTABLE
  DEPENDS "Qt5LinguistTools" Qt5LinguistTools_FOUND
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
  find_path(QTSINGLEAPPLICATION_INCLUDE_DIRS qtsingleapplication.h PATH_SUFFIXES qt5/QtSolutions)
  find_library(QTSINGLEAPPLICATION_LIBRARIES Qt5Solutions_SingleApplication-2.6)
  find_library(QTSINGLECOREAPPLICATION_LIBRARIES Qt5Solutions_SingleCoreApplication-2.6)
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
  find_path(QXTCORE_INCLUDE_DIRS qxtglobal.h PATH_SUFFIXES qt5/QxtCore)
  find_path(QXTGUI_INCLUDE_DIRS qxtglobalshortcut.h PATH_SUFFIXES qt5/QxtWidgets)
  set(QXT_INCLUDE_DIRS ${QXTCORE_INCLUDE_DIRS} ${QXTGUI_INCLUDE_DIRS})
  # We only need its header. We don't need to link to QxtCore.
  find_library(QXT_LIBRARIES QxtWidgets-qt5)
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

# Use the system libmygpo-qt5 if a recent enough version was found
if(LIBMYGPO_QT5_FOUND)
  set(MYGPOQT5_LIBRARIES ${LIBMYGPO_QT5_LIBRARIES})
  set(MYGPOQT5_INCLUDE_DIRS ${LIBMYGPO_QT5_INCLUDE_DIRS})
else()
  add_definitions(-DMYGPO_STATIC)
  add_subdirectory(3rdparty/libmygpo-qt5)
  set(MYGPOQT5_LIBRARIES mygpo-qt5)
  set(MYGPOQT5_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libmygpo-qt5/src)
endif()

# Qocoa
add_subdirectory(3rdparty/qocoa)

# Subdirectories
add_subdirectory(src)
if (WIN32)
  add_subdirectory(3rdparty/tinysvcmdns)
endif (WIN32)
add_subdirectory(tests)
add_subdirectory(dist)
add_subdirectory(ext/libclementine-common)
add_subdirectory(ext/libclementine-tagreader)
add_subdirectory(ext/clementine-tagreader)
add_subdirectory(ext/libclementine-remote)
if(HAVE_SPOTIFY)
  add_subdirectory(ext/libclementine-spotifyblob)
endif(HAVE_SPOTIFY)

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

# Global configuration
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/clementine-config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/clementine-config.h)

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Show a summary of what we have enabled
summary_show()
