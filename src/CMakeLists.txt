cmake_minimum_required(VERSION 2.6)

# Source files
set(CLEMENTINE-SOURCES
    mainwindow.cpp
    player.cpp
    library.cpp
    librarybackend.cpp
    playlist.cpp
    playlistitem.cpp
    enginebase.cpp
    analyzers/baranalyzer.cpp
    analyzers/analyzerbase.cpp
    fht.cpp
    analyzers/blockanalyzer.cpp
    sliderwidget.cpp
    playlistview.cpp
    backgroundthread.cpp
    librarywatcher.cpp
    song.cpp
    songplaylistitem.cpp
    libraryview.cpp
    libraryconfig.cpp
    systemtrayicon.cpp
    libraryquery.cpp
    fileview.cpp
    fileviewlist.cpp
    playlistheader.cpp
    radioitem.cpp
    radioservice.cpp
    lastfmservice.cpp
    radiomodel.cpp
    lastfmconfig.cpp
    busyindicator.cpp
    radioplaylistitem.cpp
    radioview.cpp
    lastfmstationdialog.cpp
    osd.cpp
    trackslider.cpp
    edittagdialog.cpp
    lineedit.cpp
    multiloadingindicator.cpp
    somafmservice.cpp
    settingsdialog.cpp
    librarydirectorymodel.cpp
    libraryconfigdialog.cpp
    lastfmconfigdialog.cpp
    about.cpp
    albumcoverfetcher.cpp
    addstreamdialog.cpp
    savedradio.cpp
    stylesheetloader.cpp
    shortcutsdialog.cpp
    albumcovermanager.cpp
    albumcoverloader.cpp
)

# Header files that have Q_OBJECT in
set(CLEMENTINE-MOC-HEADERS
    mainwindow.h
    player.h
    library.h
    librarybackend.h
    playlist.h
    enginebase.h
    sliderwidget.h
    playlistview.h
    backgroundthread.h
    librarywatcher.h
    songmimedata.h
    libraryview.h
    libraryconfig.h
    systemtrayicon.h
    fileview.h
    fileviewlist.h
    playlistheader.h
    radioservice.h
    lastfmservice.h
    radiomodel.h
    lastfmconfig.h
    busyindicator.h
    radiomimedata.h
    radioview.h
    lastfmstationdialog.h
    osd.h
    trackslider.h
    edittagdialog.h
    lineedit.h
    multiloadingindicator.h
    somafmservice.h
    settingsdialog.h
    librarydirectorymodel.h
    libraryconfigdialog.h
    lastfmconfigdialog.h
    about.h
    albumcoverfetcher.h
    addstreamdialog.h
    savedradio.h
    shortcutsdialog.h
    albumcovermanager.h
    albumcoverloader.h
)

# UI files
set(CLEMENTINE-UI
    mainwindow.ui
    libraryconfig.ui
    fileview.ui
    lastfmconfig.ui
    lastfmstationdialog.ui
    trackslider.ui
    edittagdialog.ui
    multiloadingindicator.ui
    settingsdialog.ui
    libraryconfigdialog.ui
    lastfmconfigdialog.ui
    about.ui
    addstreamdialog.ui
    shortcutsdialog.ui
    albumcovermanager.ui
)

# Resource files
set(CLEMENTINE-RESOURCES
  ../data/data.qrc
  translations.qrc
)

if(WIN32)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} phononengine.cpp osd_win.cpp)
  set(CLEMENTINE-MOC-HEADERS ${CLEMENTINE-MOC-HEADERS} phononengine.h)
else(WIN32)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} xine-engine.cpp xine-scope.c)
  set(CLEMENTINE-MOC-HEADERS ${CLEMENTINE-MOC-HEADERS} xine-engine.h)
endif(WIN32)

if(APPLE)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_mac.mm)
else(APPLE)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_x11.cpp)
endif(APPLE)

qt4_wrap_cpp(CLEMENTINE-SOURCES-MOC ${CLEMENTINE-MOC-HEADERS})
qt4_wrap_ui(CLEMENTINE-SOURCES-UI ${CLEMENTINE-UI})
qt4_add_resources(CLEMENTINE-SOURCES-RESOURCE ${CLEMENTINE-RESOURCES})

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qtsingleapplication")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qxt")

set(EXECUTABLE_OUTPUT_PATH ..)
add_library(clementine_lib
   ${CLEMENTINE-SOURCES}
   ${CLEMENTINE-SOURCES-MOC}
   ${CLEMENTINE-SOURCES-UI}
)
target_link_libraries(clementine_lib
    ${QT_LIBRARIES}
    qtsingleapplication
    qxt
    ${XINE_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${CLEMENTINE-SOURCES-RESOURCE}
    lastfm
)
if (APPLE)
  target_link_libraries(clementine_lib
      ${GROWL}
  )
  include_directories(${GROWL}/Headers)
endif (APPLE)
add_dependencies(clementine_lib qtsingleapplication qxt)

add_executable(clementine
   MACOSX_BUNDLE
   main.cpp
)
target_link_libraries(clementine clementine_lib)

set_property(TARGET clementine
    PROPERTY COMPILE_FLAGS
    "-Wnon-virtual-dtor -Woverloaded-virtual -Wall"
)

set_target_properties(clementine PROPERTIES
    MACOSX_BUNDLE_INFO_STRING "Clementine 0.1"
    MACOSX_BUNDLE_ICON_FILE "clementine"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
)

install(TARGETS clementine
  BUNDLE DESTINATION bin)
