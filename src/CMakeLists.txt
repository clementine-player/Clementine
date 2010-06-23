cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Woverloaded-virtual -Wall")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${SPARKLE})
include_directories(../3rdparty/gmock/gtest/include)
include_directories(../3rdparty/qtsingleapplication)
include_directories(../3rdparty/qtiocompressor)
include_directories(../3rdparty/qxt)
include_directories(../3rdparty/libprojectm)

include_directories(${GLIB_INCLUDE_DIRECTORIES})
include_directories(${LIBXML_INCLUDE_DIRECTORIES})
include_directories(${GOBJECT_INCLUDE_DIRECTORIES})

cmake_policy(SET CMP0011 NEW)
include(../cmake/AddEngine.cmake)
include(../cmake/ParseArguments.cmake)
include(../cmake/Translations.cmake)

set(SOURCES
  analyzers/analyzerbase.cpp
  analyzers/analyzercontainer.cpp
  analyzers/baranalyzer.cpp
  analyzers/blockanalyzer.cpp
  analyzers/boomanalyzer.cpp
  analyzers/sonogram.cpp
  analyzers/turbine.cpp

  core/albumcoverfetcher.cpp
  core/albumcoverloader.cpp
  core/backgroundthread.cpp
  core/commandlineoptions.cpp
  core/database.cpp
  core/fht.cpp
  core/globalshortcutbackend.cpp
  core/globalshortcuts.cpp
  core/gnomeglobalshortcutbackend.cpp
  core/mergedproxymodel.cpp
  core/networkaccessmanager.cpp
  core/player.cpp
  core/qxtglobalshortcutbackend.cpp
  core/scopedtransaction.cpp
  core/settingsprovider.cpp
  core/song.cpp
  core/songloader.cpp
  core/stylesheetloader.cpp
  core/taskmanager.cpp
  core/utilities.cpp

  engines/enginebase.cpp

  library/groupbydialog.cpp
  library/library.cpp
  library/librarybackend.cpp
  library/libraryconfig.cpp
  library/librarydirectorymodel.cpp
  library/libraryfilterwidget.cpp
  library/librarymodel.cpp
  library/libraryplaylistitem.cpp
  library/libraryquery.cpp
  library/libraryview.cpp
  library/librarywatcher.cpp

  playlist/playlist.cpp
  playlist/playlistbackend.cpp
  playlist/playlistcontainer.cpp
  playlist/playlistdelegates.cpp
  playlist/playlistfilter.cpp
  playlist/playlistheader.cpp
  playlist/playlistitem.cpp
  playlist/playlistmanager.cpp
  playlist/playlistsequence.cpp
  playlist/playlisttabbar.cpp
  playlist/playlistundocommands.cpp
  playlist/playlistview.cpp
  playlist/songloaderinserter.cpp
  playlist/songplaylistitem.cpp

  playlistparsers/asxparser.cpp
  playlistparsers/m3uparser.cpp
  playlistparsers/parserbase.cpp
  playlistparsers/playlistparser.cpp
  playlistparsers/plsparser.cpp
  playlistparsers/xmlparser.cpp
  playlistparsers/xspfparser.cpp

  radio/fixlastfm.cpp
  radio/lastfmconfig.cpp
  radio/lastfmservice.cpp
  radio/lastfmstationdialog.cpp
  radio/magnatuneconfig.cpp
  radio/magnatunedownloaddialog.cpp
  radio/magnatuneplaylistitem.cpp
  radio/magnatuneservice.cpp
  radio/radioitem.cpp
  radio/radiomodel.cpp
  radio/radioplaylistitem.cpp
  radio/radioservice.cpp
  radio/radioview.cpp
  radio/radioviewcontainer.cpp
  radio/savedradio.cpp
  radio/somafmservice.cpp

  transcoder/transcodedialog.cpp
  transcoder/transcoder.cpp
  transcoder/transcoderformats.cpp

  ui/about.cpp
  ui/addstreamdialog.cpp
  ui/albumcovermanager.cpp
  ui/albumcovermanagerlist.cpp
  ui/albumcoversearcher.cpp
  ui/edittagdialog.cpp
  ui/equalizer.cpp
  ui/globalshortcutgrabber.cpp
  ui/globalshortcutsconfig.cpp
  ui/iconloader.cpp
  ui/mainwindow.cpp
  ui/qtsystemtrayicon.cpp
  ui/settingsdialog.cpp
  ui/systemtrayicon.cpp

  widgets/autoexpandingtreeview.cpp
  widgets/busyindicator.cpp
  widgets/equalizerslider.cpp
  widgets/errordialog.cpp
  widgets/fileview.cpp
  widgets/fileviewlist.cpp
  widgets/lineedit.cpp
  widgets/multiloadingindicator.cpp
  widgets/nowplayingwidget.cpp
  widgets/osd.cpp
  widgets/osdpretty.cpp
  widgets/progressitemdelegate.cpp
  widgets/sliderwidget.cpp
  widgets/spinbox.cpp
  widgets/stickyslider.cpp
  widgets/trackslider.cpp
  widgets/tracksliderslider.cpp
)

set(HEADERS
  analyzers/analyzerbase.h
  analyzers/analyzercontainer.h
  analyzers/baranalyzer.h
  analyzers/blockanalyzer.h
  analyzers/boomanalyzer.h
  analyzers/sonogram.h
  analyzers/turbine.h

  core/albumcoverfetcher.h
  core/albumcoverloader.h
  core/backgroundthread.h
  core/database.h
  core/globalshortcuts.h
  core/gnomeglobalshortcutbackend.h
  core/mergedproxymodel.h
  core/networkaccessmanager.h
  core/player.h
  core/songloader.h
  core/taskmanager.h

  engines/enginebase.h

  library/groupbydialog.h
  library/library.h
  library/librarybackend.h
  library/libraryconfig.h
  library/librarydirectorymodel.h
  library/libraryfilterwidget.h
  library/librarymodel.h
  library/libraryview.h
  library/librarywatcher.h

  playlist/playlist.h
  playlist/playlistbackend.h
  playlist/playlistcontainer.h
  playlist/playlistdelegates.h
  playlist/playlistfilter.h
  playlist/playlistheader.h
  playlist/playlistmanager.h
  playlist/playlistsequence.h
  playlist/playlisttabbar.h
  playlist/playlistview.h
  playlist/songloaderinserter.h
  playlist/songmimedata.h

  playlistparsers/asxparser.h
  playlistparsers/m3uparser.h
  playlistparsers/parserbase.h
  playlistparsers/playlistparser.h
  playlistparsers/plsparser.h
  playlistparsers/xspfparser.h

  radio/lastfmconfig.h
  radio/lastfmservice.h
  radio/lastfmstationdialog.h
  radio/magnatuneconfig.h
  radio/magnatunedownloaddialog.h
  radio/magnatuneservice.h
  radio/radiomimedata.h
  radio/radiomodel.h
  radio/radioservice.h
  radio/radioview.h
  radio/radioviewcontainer.h
  radio/savedradio.h
  radio/somafmservice.h

  transcoder/transcodedialog.h
  transcoder/transcoder.h

  ui/about.h
  ui/addstreamdialog.h
  ui/albumcovermanager.h
  ui/albumcovermanagerlist.h
  ui/albumcoversearcher.h
  ui/edittagdialog.h
  ui/equalizer.h
  ui/globalshortcutgrabber.h
  ui/globalshortcutsconfig.h
  ui/mainwindow.h
  ui/qtsystemtrayicon.h
  ui/settingsdialog.h
  ui/systemtrayicon.h

  widgets/autoexpandingtreeview.h
  widgets/busyindicator.h
  widgets/equalizerslider.h
  widgets/errordialog.h
  widgets/fileview.h
  widgets/fileviewlist.h
  widgets/lineedit.h
  widgets/multiloadingindicator.h
  widgets/nowplayingwidget.h
  widgets/osd.h
  widgets/osdpretty.h
  widgets/progressitemdelegate.h
  widgets/sliderwidget.h
  widgets/spinbox.h
  widgets/stickyslider.h
  widgets/trackslider.h
)

set(UI
  library/groupbydialog.ui
  library/libraryconfig.ui
  library/libraryfilterwidget.ui

  playlist/playlistcontainer.ui
  playlist/playlistsequence.ui

  radio/lastfmconfig.ui
  radio/lastfmstationdialog.ui
  radio/magnatuneconfig.ui
  radio/magnatunedownloaddialog.ui
  radio/radioviewcontainer.ui

  transcoder/transcodedialog.ui
  transcoder/transcodelogdialog.ui

  ui/about.ui
  ui/addstreamdialog.ui
  ui/albumcovermanager.ui
  ui/albumcoversearcher.ui
  ui/edittagdialog.ui
  ui/equalizer.ui
  ui/globalshortcutgrabber.ui
  ui/globalshortcutsconfig.ui
  ui/mainwindow.ui
  ui/settingsdialog.ui

  widgets/equalizerslider.ui
  widgets/errordialog.ui
  widgets/fileview.ui
  widgets/multiloadingindicator.ui
  widgets/osdpretty.ui
  widgets/trackslider.ui
)

set(RESOURCES
  ../data/data.qrc
)

set(LINGUAS "All" CACHE STRING "A space-seperated list of translations to compile in to Clementine, or \"None\".")
if ("${LINGUAS}" STREQUAL "All")
  set (LANGUAGES
    ar
    cs
    da
    de
    el
    en_CA
    en_GB
    es
    fi
    fr
    gl
    it
    kk
    nb
    oc
    pl
    pt
    pt_BR
    ro
    ru
    sk
    sv
    tr
    uk
    zh_CN
    zh_TW
  )
else ("${LINGUAS}" STREQUAL "All")
  if (NOT "${LINGUAS}" OR "${LINGUAS}" STREQUAL "None")
    set (LANGUAGES "")
  else (NOT "${LINGUAS}" OR "${LINGUAS}" STREQUAL "None")
    string(REGEX MATCHALL [a-zA-Z_]+
           LANGUAGES ${LINGUAS})
  endif (NOT "${LINGUAS}" OR "${LINGUAS}" STREQUAL "None")
endif ("${LINGUAS}" STREQUAL "All")

# Engines
set(GST_ENGINE_SRC engines/gstengine.cpp engines/gstequalizer.cpp engines/gstenginepipeline.cpp)
set(GST_ENGINE_MOC engines/gstengine.h engines/gstenginepipeline.h)
set(GST_ENGINE_LIB GSTREAMER GSTREAMER_BASE)
set(XINE_ENGINE_SRC engines/xine-engine.cpp engines/xine-scope.c)
set(XINE_ENGINE_MOC engines/xine-engine.h)

add_engine(gst "${GST_ENGINE_LIB}" "${GST_ENGINE_SRC}" "${GST_ENGINE_MOC}" ON)
add_engine(vlc LIBVLC engines/vlcengine.cpp engines/vlcengine.h OFF)
add_engine(xine LIBXINE "${XINE_ENGINE_SRC}" "${XINE_ENGINE_MOC}" OFF)
add_engine(qt-phonon QT_PHONON engines/phononengine.cpp engines/phononengine.h OFF)
print_engines()

# Visualisations
if(ENABLE_VISUALISATIONS)
  option(VISUALISATIONS_USE_INSTALL_PREFIX "Look for visualisations in CMAKE_INSTALL_PREFIX" ON)

  list(APPEND SOURCES
    visualisations/projectmpresetmodel.cpp
    visualisations/projectmvisualisation.cpp
    visualisations/visualisationcontainer.cpp
    visualisations/visualisationoverlay.cpp
    visualisations/visualisationselector.cpp
  )
  list(APPEND HEADERS
    visualisations/projectmpresetmodel.h
    visualisations/projectmvisualisation.h
    visualisations/visualisationcontainer.h
    visualisations/visualisationoverlay.h
    visualisations/visualisationselector.h
  )
  list(APPEND UI
    visualisations/visualisationoverlay.ui
    visualisations/visualisationselector.ui
  )
endif(ENABLE_VISUALISATIONS)

# OSDs
if(APPLE)
  list(APPEND SOURCES widgets/osd_mac.mm)
  list(APPEND SOURCES core/macglobalshortcutbackend.mm)
  list(APPEND SOURCES ui/macsystemtrayicon.mm)
  list(APPEND HEADERS core/macglobalshortcutbackend.h)
  list(APPEND HEADERS ui/macsystemtrayicon.h)
  include_directories(${GROWL}/Headers)
else(APPLE)
  if(WIN32)
    list(APPEND SOURCES widgets/osd_win.cpp)
  else(WIN32)
    list(APPEND SOURCES widgets/osd_x11.cpp)
  endif(WIN32)
endif(APPLE)

list(APPEND OTHER_SOURCES
  core/macglobalshortcutbackend.h
  core/macglobalshortcutbackend.mm
  ui/macsystemtrayicon.h
  ui/macsystemtrayicon.mm
  widgets/osd_mac.mm
  widgets/osd_win.cpp
  widgets/osd_x11.cpp
)

# MPRIS - Linux specific
if(NOT APPLE AND NOT WIN32)
  qt4_add_dbus_adaptor(SOURCES
      ../data/org.freedesktop.MediaPlayer.player.xml
      core/player.h Player core/mpris_player MprisPlayer)
  qt4_add_dbus_adaptor(SOURCES
      ../data/org.freedesktop.MediaPlayer.root.xml
      core/mpris.h MPRIS core/mpris_root MprisRoot)
  qt4_add_dbus_adaptor(SOURCES
      ../data/org.freedesktop.MediaPlayer.tracklist.xml
      core/player.h Player core/mpris_tracklist MprisTrackList)

  # org.freedesktop.Notifications
  qt4_add_dbus_interface(SOURCES
      ../data/org.freedesktop.Notifications.xml
      core/notification)
  list(APPEND SOURCES core/mpris.cpp)
  list(APPEND HEADERS core/mpris.h)
endif(NOT APPLE AND NOT WIN32)

# Mac specific startup stuff
if(APPLE)
  list(APPEND SOURCES core/mac_startup.mm)
endif(APPLE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})
qt4_add_resources(QRC ${RESOURCES})

add_pot(
  ${CMAKE_CURRENT_SOURCE_DIR}/translations/header
  ${CMAKE_CURRENT_SOURCE_DIR}/translations/translations.pot
  ${SOURCES} ${MOC} ${UIC} ${OTHER_SOURCES}
)
add_po(PO clementine_
  POT ${CMAKE_CURRENT_SOURCE_DIR}/translations/translations.pot
  LANGUAGES ${LANGUAGES}
  DIRECTORY translations
)

add_library(clementine_lib STATIC
  ${SOURCES}
  ${MOC}
  ${UIC}
  ${QRC}
  ${PO}
)

add_dependencies(clementine_lib pot)

target_link_libraries(clementine_lib
  qtsingleapplication
  qtiocompressor
  chardet
  ${GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${QT_LIBRARIES}
  ${ENGINE_LIBRARIES}
  ${LASTFM_LIBRARIES}
)

if(ENABLE_VISUALISATIONS)
  target_link_libraries(clementine_lib projectM)
endif(ENABLE_VISUALISATIONS)

if (APPLE)
  target_link_libraries(clementine_lib
    ${GROWL}
    ${SPARKLE}
    z
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
  install(FILES ../dist/clementine.icns
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources")
  install(FILES ../dist/qt.conf
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources")
  install(FILES ../dist/sparkle_pub.pem
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources")

  install(DIRECTORY "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources/"
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources")
  install(DIRECTORY "${SPARKLE}/Versions/Current/Resources"
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Frameworks/Sparkle.framework")

  add_custom_target(bundle
    ${CMAKE_CURRENT_SOURCE_DIR}/../macdeploy.py ${PROJECT_BINARY_DIR}/clementine.app -f
    DEPENDS clementine
  )
  add_custom_target(dmg
    ${CMAKE_COMMAND} -E remove -f ${PROJECT_BINARY_DIR}/clementine.dmg 
    COMMAND macdeployqt clementine.app -no-plugins -dmg
    DEPENDS clementine
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
else (APPLE)
  install(TARGETS clementine
    RUNTIME DESTINATION bin
  )

  install(FILES ../dist/clementine_64.png
    DESTINATION share/icons/hicolor/64x64/apps/
    RENAME application-x-clementine.png
  )

  install(FILES ../dist/clementine.desktop
    DESTINATION share/applications
  )
endif (APPLE)
