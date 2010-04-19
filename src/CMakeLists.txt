cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wnon-virtual-dtor -Woverloaded-virtual -Wall")

# Source files
set(CLEMENTINE-SOURCES
    mainwindow.cpp
    player.cpp
    library.cpp
    librarybackend.cpp
    playlist.cpp
    playlistitem.cpp
    engines/enginebase.cpp
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
    osdpretty.cpp
    playlistdelegates.cpp
    groupbydialog.cpp
    equalizer.cpp
    equalizerslider.cpp
    tracksliderslider.cpp
    stickyslider.cpp
    commandlineoptions.cpp
    settingsprovider.cpp
    libraryplaylistitem.cpp
    scopedtransaction.cpp
    playlistundocommands.cpp
)

# Header files that have Q_OBJECT in
set(CLEMENTINE-MOC-HEADERS
    mainwindow.h
    player.h
    library.h
    librarybackend.h
    playlist.h
    engines/enginebase.h
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
    osdpretty.h
    playlistdelegates.h
    groupbydialog.h
    equalizer.h
    equalizerslider.h
    stickyslider.h
)

# lists of engine source files
set(GST_ENGINE_SRC
  engines/gstengine.cpp
  engines/gstequalizer.cpp
  engines/gstenginepipeline.cpp
)
set(GST_ENGINE_MOC
  engines/gstengine.h
  engines/gstenginepipeline.h
)
set(GST_ENGINE_LIB
  GSTREAMER
  GSTREAMER_BASE
)
set(XINE_ENGINE_SRC
  engines/xine-engine.cpp
  engines/xine-scope.c
)
set(XINE_ENGINE_MOC
  engines/xine-engine.h
)

# try to add engines
include(AddEngine.cmake)
add_engine(gst "${GST_ENGINE_LIB}" "${GST_ENGINE_SRC}" "${GST_ENGINE_MOC}" ON)
add_engine(vlc LIBVLC engines/vlcengine.cpp engines/vlcengine.h OFF)
add_engine(xine LIBXINE "${XINE_ENGINE_SRC}" "${XINE_ENGINE_MOC}" OFF)
add_engine(qt-phonon QT_PHONON engines/phononengine.cpp engines/phononengine.h OFF)
print_engines()

# need at least 1 engine
if(NOT ENGINES_ENABLED)
  message(FATAL_ERROR "no engine enabled!")
endif(NOT ENGINES_ENABLED)

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
    osdpretty.ui
    groupbydialog.ui
    equalizer.ui
    equalizerslider.ui
)

# Resource files
set(CLEMENTINE-RESOURCES
  ../data/data.qrc
)

set(CLEMENTINE-LANGUAGES
  cs
  da_DK
  de
  el
  en_GB
  es
  fr
  nb
  pl
  pt
  pt_BR
  ru
  sk
  sv
  tr
)

# OSD and DBus.
if(APPLE)
  set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_mac.mm mac_startup.mm)
else(APPLE)
  if(WIN32)
    set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_win.cpp)
  else(WIN32)
    set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES} osd_x11.cpp)
    # MPRIS
    qt4_add_dbus_adaptor(MPRIS-PLAYER-SOURCES
        ../data/org.freedesktop.MediaPlayer.player.xml
        player.h Player mpris_player MprisPlayer)
    qt4_add_dbus_adaptor(MPRIS-ROOT-SOURCES
        ../data/org.freedesktop.MediaPlayer.root.xml
        mpris.h MPRIS mpris_root MprisRoot)
    qt4_add_dbus_adaptor(MPRIS-TRACKLIST-SOURCES
        ../data/org.freedesktop.MediaPlayer.tracklist.xml
        player.h Player mpris_tracklist MprisTrackList)

    # org.freedesktop.Notifications
    qt4_add_dbus_interface(NOTIFICATION-SOURCES
        ../data/org.freedesktop.Notifications.xml
        notification)
    set(CLEMENTINE-SOURCES ${CLEMENTINE-SOURCES}
        ${MPRIS-PLAYER-SOURCES}
        ${MPRIS-ROOT-SOURCES}
        ${MPRIS-TRACKLIST-SOURCES}
        ${NOTIFICATION-SOURCES}
        mpris.cpp
        )
    set(CLEMENTINE-MOC-HEADERS ${CLEMENTINE-MOC-HEADERS} mpris.h)
  endif(WIN32)
endif(APPLE)

# resource file for windows
if(WIN32)
  set(CLEMENTINE-WIN32-RESOURCES ../dist/windows/windres.rc)
endif(WIN32)

qt4_wrap_cpp(CLEMENTINE-SOURCES-MOC ${CLEMENTINE-MOC-HEADERS})
qt4_wrap_ui(CLEMENTINE-SOURCES-UI ${CLEMENTINE-UI})
qt4_add_resources(CLEMENTINE-SOURCES-RESOURCE ${CLEMENTINE-RESOURCES})

# Translations
set (FILES_TO_TRANSLATE ${CLEMENTINE-SOURCES} ${CLEMENTINE-SOURCES-UI} ${CLEMENTINE-SOURCES-MOC})

# Make all the filenames relative
foreach (_file ${FILES_TO_TRANSLATE})
  if (IS_ABSOLUTE ${_file})
    file(RELATIVE_PATH _rel ${CMAKE_CURRENT_SOURCE_DIR} ${_file})
  else (IS_ABSOLUTE ${_file})
    set(_rel ${_file})
  endif (IS_ABSOLUTE ${_file})
  set(FILES_TO_TRANSLATE_REL ${FILES_TO_TRANSLATE_REL} ${_rel})
endforeach (_file)

set (XGETTEXT_OPTIONS --qt --keyword=tr --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=trUtf8 --flag=tr:1:pass-c-format --flag=tr:1:pass-qt-format
    --keyword=translate:2 --flag=translate:2:pass-c-format --flag=translate:2:pass-qt-format
    --keyword=QT_TR_NOOP --flag=QT_TR_NOOP:1:pass-c-format --flag=QT_TR_NOOP:1:pass-qt-format
    --keyword=QT_TRANSLATE_NOOP:2 --flag=QT_TRANSLATE_NOOP:2:pass-c-format --flag=QT_TRANSLATE_NOOP:2:pass-qt-format
    --keyword=_ --flag=_:1:pass-c-format --flag=_:1:pass-qt-format
    --keyword=N_ --flag=N_:1:pass-c-format --flag=N_:1:pass-qt-format
    --from-code=utf-8)

# Generate the .pot
set (CLEMENTINE-POT "${CMAKE_CURRENT_SOURCE_DIR}/translations/translations.pot")
add_custom_target(pot ALL
  COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} 
      ${XGETTEXT_OPTIONS} -C --omit-header --no-location
      --directory=${CMAKE_CURRENT_SOURCE_DIR}
      --output=${CMAKE_CURRENT_BINARY_DIR}/translations.pot
      ${FILES_TO_TRANSLATE_REL}
  COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/translations/header ${CMAKE_CURRENT_BINARY_DIR}/translations.pot > ${CLEMENTINE-POT}
  DEPENDS ${FILES_TO_TRANSLATE})

add_custom_target(po_all ALL)

# Merge the .pot into .po files
foreach (_lang ${CLEMENTINE-LANGUAGES})
  set(_po ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_lang}.po)
  add_custom_target("po_${_lang}"
    COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet -U --no-location
        ${_po} ${CLEMENTINE-POT}
    DEPENDS ${_po})
  add_dependencies("po_${_lang}" pot)
  add_dependencies(po_all "po_${_lang}")
endforeach (_lang)

# Convert the .po files to .qm files
foreach (_lang ${CLEMENTINE-LANGUAGES})
  set(_po_filename "${_lang}.po")
  set(_po_filepath "${CMAKE_CURRENT_SOURCE_DIR}/translations/${_po_filename}")
  set(_qm_filename "clementine_${_lang}.qm")
  set(_qm_filepath "${CMAKE_CURRENT_BINARY_DIR}/${_qm_filename}")
  
  add_custom_command(OUTPUT ${_qm_filepath}
    COMMAND lconvert ARGS ${_po_filepath} -o ${_qm_filepath} -of qm
    MAIN_DEPENDENCY ${_po_filepath}
  )
  set(CLEMENTINE-QM-FILES ${CLEMENTINE-QM-FILES} ${_qm_filepath})
endforeach (_lang)

# Generate a qrc file for the translations
set(CLEMENTINE-QM-RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)
file(WRITE ${CLEMENTINE-QM-RESOURCE} "<RCC><qresource prefix=\"/translations\">")
foreach(QM-FILE ${CLEMENTINE-QM-FILES})
  file(RELATIVE_PATH QM-RELATIVE-PATH ${CMAKE_CURRENT_BINARY_DIR} ${QM-FILE})
  file(APPEND ${CLEMENTINE-QM-RESOURCE} "<file>" ${QM-RELATIVE-PATH} "</file>")
endforeach(QM-FILE)
file(APPEND ${CLEMENTINE-QM-RESOURCE} "</qresource></RCC>")
qt4_add_resources(CLEMENTINE-SOURCES-RESOURCE ${CLEMENTINE-QM-RESOURCE})

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
)
target_link_libraries(clementine_lib
    qtsingleapplication
    lastfm
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
  include_directories(${GROWL}/Headers)
  include_directories(${SPARKLE}/Headers)
else (APPLE)
  target_link_libraries(clementine_lib qxt)
endif (APPLE)
add_dependencies(clementine_lib qtsingleapplication)

# Link against the qsqlite plugin on windows
if(WIN32 OR APPLE)
  set(3RDPARTY_SQLITE_LIBRARY qsqlite)
  target_link_libraries(clementine_lib qsqlite)
endif(WIN32 OR APPLE)

add_executable(clementine
   MACOSX_BUNDLE
   WIN32
   ${CLEMENTINE-WIN32-RESOURCES}
   main.cpp
)
target_link_libraries(clementine clementine_lib)

set_target_properties(clementine PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "../dist/Info.plist"
)
if (APPLE)
  configure_file(
      ../dist/clementine.icns
      ${PROJECT_BINARY_DIR}/clementine.app/Contents/Resources/clementine.icns
      COPY_ONLY)
  configure_file(
      ../dist/qt.conf
      ${PROJECT_BINARY_DIR}/clementine.app/Contents/Resources/qt.conf
      COPY_ONLY)
  configure_file(
      ../dist/sparkle_pub.pem
      ${PROJECT_BINARY_DIR}/clementine.app/Contents/Resources/sparkle_pub.pem
      COPY_ONLY)
  add_custom_target(bundle
      ${CMAKE_CURRENT_SOURCE_DIR}/../macdeploy.py ${PROJECT_BINARY_DIR}/clementine.app -f
      DEPENDS clementine
      )
  add_custom_target(dmg
      macdeployqt ${PROJECT_BINARY_DIR}/clementine.app -dmg
      DEPENDS clementine)
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

