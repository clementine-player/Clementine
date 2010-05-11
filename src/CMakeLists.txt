cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wnon-virtual-dtor -Woverloaded-virtual -Wall")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qtsingleapplication")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qtiocompressor")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qxt")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest/include")

# These can be seen from child scopes
set(CLEMENTINE-TRANSLATION-SOURCE CACHE INTERNAL foo)
set(CLEMENTINE-TRANSLATION-DEPS CACHE INTERNAL foo)

macro(add_translation_source subdirectory)
  get_filename_component(_translations_dir ${CMAKE_CURRENT_SOURCE_DIR}/../translations ABSOLUTE)
  foreach(_file ${ARGN})
    get_filename_component(_dep ${_file} ABSOLUTE)
    if (IS_ABSOLUTE ${_file})
      file(RELATIVE_PATH _relative ${_translations_dir} ${_file})
    else (IS_ABSOLUTE ${_file})
      set(_relative "../${subdirectory}/${_file}")
    endif (IS_ABSOLUTE ${_file})
    set(CLEMENTINE-TRANSLATION-SOURCE ${CLEMENTINE-TRANSLATION-SOURCE} ${_relative} CACHE INTERNAL foo)
    set(CLEMENTINE-TRANSLATION-DEPS ${CLEMENTINE-TRANSLATION-DEPS} ${_dep} CACHE INTERNAL foo)
  endforeach(_file)
endmacro(add_translation_source)

add_subdirectory(analyzers)
add_subdirectory(core)
add_subdirectory(engines)
add_subdirectory(library)
add_subdirectory(playlist)
add_subdirectory(radio)
add_subdirectory(transcoder)
add_subdirectory(ui)
add_subdirectory(widgets)
add_subdirectory(translations)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CLEMENTINE-RESOURCES
  ../data/data.qrc
)

qt4_wrap_cpp(CLEMENTINE-SOURCES-MOC ${CLEMENTINE-MOC-HEADERS})
qt4_wrap_ui(CLEMENTINE-SOURCES-UI ${CLEMENTINE-UI})
qt4_add_resources(CLEMENTINE-SOURCES-RESOURCE ${CLEMENTINE-RESOURCES})

add_library(clementine_lib
  ${CLEMENTINE-SOURCES-RESOURCE}
)

target_link_libraries(clementine_lib
  clementine_analyzers
  clementine_core
  clementine_engines
  clementine_library
  clementine_playlist
  clementine_radio
  clementine_transcoder
  clementine_ui
  clementine_widgets
  clementine_translations
  qtsingleapplication
  qtiocompressor
  lastfm
  chardet
  z
  ${GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${QT_LIBRARIES}
  ${ENGINE_LIBRARIES}
)

if (APPLE)
  target_link_libraries(clementine_lib
    ${GROWL}
    ${SPARKLE}
    /System/Library/Frameworks/Carbon.framework
    /System/Library/Frameworks/Foundation.framework
    /System/Library/Frameworks/AppKit.framework
  )
  include_directories(${SPARKLE}/Headers)
else (APPLE)
  target_link_libraries(clementine_lib qxt)
endif (APPLE)

# Link against the qsqlite plugin on windows and mac
if(WIN32 OR APPLE)
  set(3RDPARTY_SQLITE_LIBRARY qsqlite)
  target_link_libraries(clementine_lib qsqlite)
endif(WIN32 OR APPLE)

add_dependencies(clementine_lib qtsingleapplication)


###############################################################################

set(EXECUTABLE_OUTPUT_PATH ..)

# Show the console window in debug mode on Windows
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CLEMENTINE-WIN32-FLAG WIN32)
endif (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")

# resource file for windows
if(WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dist/windows/windres.rc.in
                 ${CMAKE_CURRENT_BINARY_DIR}/windres.rc)
  set(CLEMENTINE-WIN32-RESOURCES windres.rc)
endif(WIN32)

add_executable(clementine
  MACOSX_BUNDLE
  ${CLEMENTINE-WIN32-FLAG}
  ${CLEMENTINE-WIN32-RESOURCES}
  main.cpp
)
target_link_libraries(clementine
  clementine_lib
)

set_target_properties(clementine PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "../dist/Info.plist"
)

if (APPLE)
  configure_file(
    ../dist/clementine.icns
    ${PROJECT_BINARY_DIR}/clementine.app/Contents/Resources/clementine.icns
    COPY_ONLY
  )
  configure_file(
    ../dist/qt.conf
    ${PROJECT_BINARY_DIR}/clementine.app/Contents/Resources/qt.conf
    COPY_ONLY
  )
  configure_file(
    ../dist/sparkle_pub.pem
    ${PROJECT_BINARY_DIR}/clementine.app/Contents/Resources/sparkle_pub.pem
    COPY_ONLY
  )
  add_custom_target(bundle
    ${CMAKE_CURRENT_SOURCE_DIR}/../macdeploy.py ${PROJECT_BINARY_DIR}/clementine.app -f
    DEPENDS clementine
  )
  add_custom_target(dmg
    macdeployqt ${PROJECT_BINARY_DIR}/clementine.app -dmg
    DEPENDS clementine
  )
endif (APPLE)

install(TARGETS clementine
  BUNDLE DESTINATION bin
  RUNTIME DESTINATION bin
)
  
install(FILES ../dist/clementine_64.png
  DESTINATION share/icons/hicolor/64x64/apps/
  RENAME application-x-clementine.png
)

install(FILES ../dist/clementine.desktop
  DESTINATION share/applications
)

