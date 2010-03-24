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
    analyzers/analyzercontainer.cpp
    analyzers/boomanalyzer.cpp
    analyzers/sonogram.cpp
    analyzers/turbine.cpp
    sliderwidget.cpp
    playlistview.cpp
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
    albumcovermanager.cpp
    albumcoverloader.cpp
    m3uparser.cpp
    playlistsequence.cpp
    xspfparser.cpp
    globalshortcuts/globalshortcuts.cpp
    fixlastfm.cpp
    backgroundthread.cpp
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
    albumcovermanager.h
    albumcoverloader.h
    m3uparser.h
    playlistsequence.h
    xspfparser.h
    analyzers/analyzercontainer.h
    analyzers/baranalyzer.h
    analyzers/blockanalyzer.h
    analyzers/analyzerbase.h
    analyzers/boomanalyzer.h
    analyzers/sonogram.h
    analyzers/turbine.h
    globalshortcuts/globalshortcuts.h
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
    albumcovermanager.ui
    playlistsequence.ui
)

# Resource files
set(CLEMENTINE-RESOURCES
  ../data/data.qrc
)

# Translations
file (GLOB TRANSLATIONS_FILES *.ts)
set (FILES_TO_TRANSLATE ${CLEMENTINE-SOURCES} ${CLEMENTINE-UI} ${CLEMENTINE-MOC-HEADERS})

option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts files
    (WARNING: make clean will delete the source .ts files! Danger!)")
if (UPDATE_TRANSLATIONS)
  qt4_create_translation(CLEMENTINE-QM-FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS_FILES})
else (UPDATE_TRANSLATIONS)
  qt4_add_translation(CLEMENTINE-QM-FILES ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS)

# Generate a qrc file for the translations
set(CLEMENTINE-QM-RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)
file(WRITE ${CLEMENTINE-QM-RESOURCE} "<RCC><qresource prefix=\"/translations\">")
foreach(QM-FILE ${CLEMENTINE-QM-FILES})
  file(RELATIVE_PATH QM-RELATIVE-PATH ${CMAKE_CURRENT_BINARY_DIR} ${QM-FILE})
  file(APPEND ${CLEMENTINE-QM-RESOURCE} "<file>" ${QM-RELATIVE-PATH} "</file>")
endforeach(QM-FILE)
file(APPEND ${CLEMENTINE-QM-RESOURCE} "</qresource></RCC>")

# Audio engine - phonon on windows, xine elsewhere
if(WIN32)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} phononengine.cpp)
  set(CLEMENTINE-MOC-HEADERS ${CLEMENTINE-MOC-HEADERS} phononengine.h)
else(WIN32)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} xine-engine.cpp xine-scope.c)
  set(CLEMENTINE-MOC-HEADERS ${CLEMENTINE-MOC-HEADERS} xine-engine.h)
endif(WIN32)

# OSD
if(APPLE)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_mac.mm)
else(APPLE)
  if(WIN32)
    set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_win.cpp)
  else(WIN32)
    set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_x11.cpp)
  endif(WIN32)
endif(APPLE)

# resource file for windows
if(WIN32)
  set(CLEMENTINE-WIN32-RESOURCES ../dist/windows/windres.rc)
endif(WIN32)

qt4_wrap_cpp(CLEMENTINE-SOURCES-MOC ${CLEMENTINE-MOC-HEADERS})
qt4_wrap_ui(CLEMENTINE-SOURCES-UI ${CLEMENTINE-UI})
qt4_add_resources(CLEMENTINE-SOURCES-RESOURCE ${CLEMENTINE-RESOURCES} ${CLEMENTINE-QM-RESOURCE})

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qtsingleapplication")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qxt")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest/include")

set(EXECUTABLE_OUTPUT_PATH ..)
add_library(clementine_lib
   ${CLEMENTINE-SOURCES}
   ${CLEMENTINE-SOURCES-MOC}
   ${CLEMENTINE-SOURCES-UI}
   ${CLEMENTINE-SOURCES-RESOURCE}
   ${CLEMENTINE-QM-FILES}
)
target_link_libraries(clementine_lib
    qtsingleapplication
    qxt
    lastfm
    ${XINE_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${QT_LIBRARIES}
)
if (APPLE)
  target_link_libraries(clementine_lib
      ${GROWL}
  )
  include_directories(${GROWL}/Headers)
endif (APPLE)
add_dependencies(clementine_lib qtsingleapplication qxt)

# Link against the qsqlite plugin on windows
if(WIN32)
  set(3RDPARTY_SQLITE_LIBRARY qsqlite)
  target_link_libraries(clementine_lib qsqlite)
endif(WIN32)

add_executable(clementine
   MACOSX_BUNDLE
   WIN32
   ${CLEMENTINE-WIN32-RESOURCES}
   main.cpp
)
target_link_libraries(clementine clementine_lib)

set_property(TARGET clementine clementine_lib
    PROPERTY COMPILE_FLAGS
    "-Wnon-virtual-dtor -Woverloaded-virtual -Wall"
)

set_target_properties(clementine PROPERTIES
    MACOSX_BUNDLE_INFO_STRING "Clementine 0.2"
    MACOSX_BUNDLE_ICON_FILE "clementine"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.2"
)

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
