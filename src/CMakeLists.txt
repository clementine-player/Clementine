
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare")

option(BUILD_WERROR "Build with -Werror" ON)

if(BUILD_WERROR)
  if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif (LINUX)
endif(BUILD_WERROR)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(../3rdparty/gmock/gtest/include)
if(WIN32)
  include_directories(../3rdparty/qtwin)
endif(WIN32)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${LIBPROJECTM_INCLUDE_DIRS})
include_directories(${QTSINGLEAPPLICATION_INCLUDE_DIRS})
include_directories(${QTIOCOMPRESSOR_INCLUDE_DIRS})
include_directories(${QXT_INCLUDE_DIRS})
include_directories(${ECHONEST_INCLUDE_DIRS})

if(HAVE_LIBINDICATE)
  link_directories(${INDICATEQT_LIBRARY_DIRS})
  include_directories(${INDICATEQT_INCLUDE_DIRS})
endif(HAVE_LIBINDICATE)

if(HAVE_LIBLASTFM)
  include_directories(${LASTFM_INCLUDE_DIRS})
endif(HAVE_LIBLASTFM)

if(HAVE_SCRIPTING_PYTHON)
  include_directories(${PYTHON_INCLUDE_DIRS})
endif(HAVE_SCRIPTING_PYTHON)

cmake_policy(SET CMP0011 NEW)
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

  core/albumcoverloader.cpp
  core/artloader.cpp
  core/backgroundstreams.cpp
  core/backgroundthread.cpp
  core/commandlineoptions.cpp
  core/database.cpp
  core/deletefiles.cpp
  core/encoding.cpp
  core/filesystemmusicstorage.cpp
  core/fht.cpp
  core/fmpsparser.cpp
  core/globalshortcutbackend.cpp
  core/globalshortcuts.cpp
  core/gnomeglobalshortcutbackend.cpp
  core/kittenloader.cpp
  core/mergedproxymodel.cpp
  core/musicstorage.cpp
  core/network.cpp
  core/networkproxyfactory.cpp
  core/organise.cpp
  core/organiseformat.cpp
  core/player.cpp
  core/qxtglobalshortcutbackend.cpp
  core/scopedtransaction.cpp
  core/settingsprovider.cpp
  core/song.cpp
  core/songloader.cpp
  core/stylesheetloader.cpp
  core/taskmanager.cpp
  core/utilities.cpp

  devices/connecteddevice.cpp
  devices/devicedatabasebackend.cpp
  devices/devicelister.cpp
  devices/devicemanager.cpp
  devices/deviceproperties.cpp
  devices/devicestatefiltermodel.cpp
  devices/deviceview.cpp
  devices/filesystemdevice.cpp

  engines/enginebase.cpp
  engines/gstengine.cpp
  engines/gstenginepipeline.cpp
  engines/gstelementdeleter.cpp

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
  library/libraryviewcontainer.cpp
  library/librarywatcher.cpp
  library/sqlrow.cpp

  musicbrainz/fingerprinter.cpp
  musicbrainz/musicbrainzclient.cpp
  musicbrainz/musicdnsclient.cpp
  musicbrainz/tagfetcher.cpp

  playlist/dynamicplaylistcontrols.cpp
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
  playlist/queue.cpp
  playlist/queuemanager.cpp
  playlist/songloaderinserter.cpp
  playlist/songplaylistitem.cpp

  playlistparsers/asxparser.cpp
  playlistparsers/asxiniparser.cpp
  playlistparsers/cueparser.cpp
  playlistparsers/m3uparser.cpp
  playlistparsers/parserbase.cpp
  playlistparsers/playlistparser.cpp
  playlistparsers/plsparser.cpp
  playlistparsers/xmlparser.cpp
  playlistparsers/xspfparser.cpp

  radio/icecastbackend.cpp
  radio/icecastfilterwidget.cpp
  radio/icecastmodel.cpp
  radio/icecastservice.cpp
  radio/jamendodynamicplaylist.cpp
  radio/jamendoplaylistitem.cpp
  radio/jamendoservice.cpp
  radio/magnatuneconfig.cpp
  radio/magnatunedownloaddialog.cpp
  radio/magnatuneplaylistitem.cpp
  radio/magnatuneservice.cpp
  radio/radiomodel.cpp
  radio/radioplaylistitem.cpp
  radio/radioservice.cpp
  radio/radioview.cpp
  radio/radioviewcontainer.cpp
  radio/savedradio.cpp
  radio/somafmservice.cpp

  scripting/installscriptdialog.cpp
  scripting/languageengine.cpp
  scripting/script.cpp
  scripting/scriptarchive.cpp
  scripting/scriptdialog.cpp
  scripting/scriptinfo.cpp
  scripting/scriptinterface.cpp
  scripting/scriptmanager.cpp
  scripting/uiinterface.cpp

  smartplaylists/generator.cpp
  smartplaylists/generatorinserter.cpp
  smartplaylists/querygenerator.cpp
  smartplaylists/querywizardplugin.cpp
  smartplaylists/search.cpp
  smartplaylists/searchpreview.cpp
  smartplaylists/searchterm.cpp
  smartplaylists/searchtermwidget.cpp
  smartplaylists/wizard.cpp
  smartplaylists/wizardplugin.cpp

  songinfo/artistinfoview.cpp
  songinfo/collapsibleinfoheader.cpp
  songinfo/collapsibleinfopane.cpp
  songinfo/echonestbiographies.cpp
  songinfo/echonestimages.cpp
  songinfo/lyricsettings.cpp
  songinfo/songinfobase.cpp
  songinfo/songinfofetcher.cpp
  songinfo/songinfoprovider.cpp
  songinfo/songinfotextview.cpp
  songinfo/songinfoview.cpp
  songinfo/songplaystats.cpp
  songinfo/ultimatelyricsprovider.cpp
  songinfo/ultimatelyricsreader.cpp

  transcoder/transcodedialog.cpp
  transcoder/transcoder.cpp

  ui/about.cpp
  ui/addstreamdialog.cpp
  ui/albumcoverchoicecontroller.cpp
  ui/coverfromurldialog.cpp
  ui/edittagdialog.cpp
  ui/equalizer.cpp
  ui/flowlayout.cpp
  ui/globalshortcutgrabber.cpp
  ui/globalshortcutsconfig.cpp
  ui/iconloader.cpp
  ui/mainwindow.cpp
  ui/organisedialog.cpp
  ui/organiseerrordialog.cpp
  ui/qtsystemtrayicon.cpp
  ui/screensaver.cpp
  ui/settingsdialog.cpp
  ui/systemtrayicon.cpp
  ui/trackselectiondialog.cpp
  ui/windows7thumbbar.cpp

  widgets/autoexpandingtreeview.cpp
  widgets/busyindicator.cpp
  widgets/clickablelabel.cpp
  widgets/elidedlabel.cpp
  widgets/equalizerslider.cpp
  widgets/errordialog.cpp
  widgets/fancytabwidget.cpp
  widgets/fileview.cpp
  widgets/fileviewlist.cpp
  widgets/freespacebar.cpp
  widgets/fullscreenhypnotoad.cpp
  widgets/lineedit.cpp
  widgets/linetextedit.cpp
  widgets/multiloadingindicator.cpp
  widgets/nowplayingwidget.cpp
  widgets/osd.cpp
  widgets/osdpretty.cpp
  widgets/prettyimage.cpp
  widgets/prettyimageview.cpp
  widgets/progressitemdelegate.cpp
  widgets/ratingwidget.cpp
  widgets/sliderwidget.cpp
  widgets/spinbox.cpp
  widgets/stickyslider.cpp
  widgets/stretchheaderview.cpp
  widgets/stylehelper.cpp
  widgets/trackslider.cpp
  widgets/tracksliderpopup.cpp
  widgets/tracksliderslider.cpp
  widgets/widgetfadehelper.cpp
)

set(HEADERS
  analyzers/analyzerbase.h
  analyzers/analyzercontainer.h
  analyzers/baranalyzer.h
  analyzers/blockanalyzer.h
  analyzers/boomanalyzer.h
  analyzers/sonogram.h
  analyzers/turbine.h

  core/albumcoverloader.h
  core/artloader.h
  core/backgroundstreams.h
  core/backgroundthread.h
  core/database.h
  core/deletefiles.h
  core/globalshortcuts.h
  core/globalshortcutbackend.h
  core/gnomeglobalshortcutbackend.h
  core/kittenloader.h
  core/mergedproxymodel.h
  core/mimedata.h
  core/network.h
  core/organise.h
  core/player.h
  core/songloader.h
  core/taskmanager.h

  devices/connecteddevice.h
  devices/devicedatabasebackend.h
  devices/devicelister.h
  devices/devicemanager.h
  devices/deviceproperties.h
  devices/devicestatefiltermodel.h
  devices/deviceview.h
  devices/filesystemdevice.h

  engines/enginebase.h
  engines/gstengine.h
  engines/gstenginepipeline.h
  engines/gstelementdeleter.h

  library/groupbydialog.h
  library/library.h
  library/librarybackend.h
  library/libraryconfig.h
  library/librarydirectorymodel.h
  library/libraryfilterwidget.h
  library/librarymodel.h
  library/libraryview.h
  library/libraryviewcontainer.h
  library/librarywatcher.h

  musicbrainz/musicbrainzclient.h
  musicbrainz/musicdnsclient.h
  musicbrainz/tagfetcher.h

  playlist/dynamicplaylistcontrols.h
  playlist/playlist.h
  playlist/playlistbackend.h
  playlist/playlistcontainer.h
  playlist/playlistdelegates.h
  playlist/playlistfilter.h
  playlist/playlistheader.h
  playlist/playlistitemmimedata.h
  playlist/playlistmanager.h
  playlist/playlistsequence.h
  playlist/playlisttabbar.h
  playlist/playlistview.h
  playlist/queue.h
  playlist/queuemanager.h
  playlist/songloaderinserter.h
  playlist/songmimedata.h

  playlistparsers/asxparser.h
  playlistparsers/asxiniparser.h
  playlistparsers/cueparser.h
  playlistparsers/m3uparser.h
  playlistparsers/parserbase.h
  playlistparsers/playlistparser.h
  playlistparsers/plsparser.h
  playlistparsers/xspfparser.h

  radio/icecastbackend.h
  radio/icecastfilterwidget.h
  radio/icecastmodel.h
  radio/icecastservice.h
  radio/jamendodynamicplaylist.h
  radio/jamendoservice.h
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

  scripting/installscriptdialog.h
  scripting/languageengine.h
  scripting/scriptdialog.h
  scripting/scriptinterface.h
  scripting/scriptmanager.h
  scripting/uiinterface.h

  smartplaylists/generator.h
  smartplaylists/generatorinserter.h
  smartplaylists/generatormimedata.h
  smartplaylists/querywizardplugin.h
  smartplaylists/searchpreview.h
  smartplaylists/searchtermwidget.h
  smartplaylists/wizard.h
  smartplaylists/wizardplugin.h

  songinfo/artistinfoview.h
  songinfo/collapsibleinfoheader.h
  songinfo/collapsibleinfopane.h
  songinfo/echonestbiographies.h
  songinfo/echonestimages.h
  songinfo/lyricsettings.h
  songinfo/songinfobase.h
  songinfo/songinfofetcher.h
  songinfo/songinfoprovider.h
  songinfo/songinfotextview.h
  songinfo/songinfoview.h
  songinfo/songplaystats.h
  songinfo/ultimatelyricsprovider.h
  songinfo/ultimatelyricsreader.h

  transcoder/transcodedialog.h
  transcoder/transcoder.h

  ui/about.h
  ui/addstreamdialog.h
  ui/albumcoverchoicecontroller.h
  ui/coverfromurldialog.h
  ui/edittagdialog.h
  ui/equalizer.h
  ui/globalshortcutgrabber.h
  ui/globalshortcutsconfig.h
  ui/mainwindow.h
  ui/organisedialog.h
  ui/organiseerrordialog.h
  ui/qtsystemtrayicon.h
  ui/settingsdialog.h
  ui/systemtrayicon.h
  ui/trackselectiondialog.h
  ui/windows7thumbbar.h

  widgets/autoexpandingtreeview.h
  widgets/busyindicator.h
  widgets/clickablelabel.h
  widgets/elidedlabel.h
  widgets/equalizerslider.h
  widgets/errordialog.h
  widgets/fancytabwidget.h
  widgets/fileview.h
  widgets/fileviewlist.h
  widgets/freespacebar.h
  widgets/lineedit.h
  widgets/linetextedit.h
  widgets/multiloadingindicator.h
  widgets/nowplayingwidget.h
  widgets/osd.h
  widgets/osdpretty.h
  widgets/prettyimage.h
  widgets/prettyimageview.h
  widgets/progressitemdelegate.h
  widgets/ratingwidget.h
  widgets/sliderwidget.h
  widgets/spinbox.h
  widgets/stickyslider.h
  widgets/stretchheaderview.h
  widgets/trackslider.h
  widgets/tracksliderpopup.h
  widgets/tracksliderslider.h
  widgets/widgetfadehelper.h
)

set(UI
  devices/deviceproperties.ui

  library/groupbydialog.ui
  library/libraryconfig.ui
  library/libraryfilterwidget.ui
  library/libraryviewcontainer.ui

  playlist/dynamicplaylistcontrols.ui
  playlist/playlistcontainer.ui
  playlist/playlistsequence.ui
  playlist/queuemanager.ui

  radio/icecastfilterwidget.ui
  radio/magnatuneconfig.ui
  radio/magnatunedownloaddialog.ui
  radio/radioviewcontainer.ui

  remote/remoteconfig.ui

  scripting/installscriptdialog.ui
  scripting/scriptdialog.ui

  smartplaylists/querysearchpage.ui
  smartplaylists/querysortpage.ui
  smartplaylists/searchpreview.ui
  smartplaylists/searchtermwidget.ui
  smartplaylists/wizardfinishpage.ui

  songinfo/lyricsettings.ui

  transcoder/transcodedialog.ui
  transcoder/transcodelogdialog.ui

  ui/about.ui
  ui/addstreamdialog.ui
  ui/coverfromurldialog.ui
  ui/edittagdialog.ui
  ui/equalizer.ui
  ui/globalshortcutgrabber.ui
  ui/globalshortcutsconfig.ui
  ui/mainwindow.ui
  ui/organisedialog.ui
  ui/organiseerrordialog.ui
  ui/settingsdialog.ui
  ui/trackselectiondialog.ui
  ui/wiimotedevshortcutsconfig.ui
  ui/wiimotedevshortcutgrabber.ui

  widgets/equalizerslider.ui
  widgets/errordialog.ui
  widgets/fileview.ui
  widgets/osdpretty.ui
  widgets/trackslider.ui
)

set(RESOURCES
  ../data/data.qrc
)

set(LINGUAS "All" CACHE STRING "A space-seperated list of translations to compile in to Clementine, or \"None\".")
if (LINGUAS STREQUAL "All")
  # build LANGUAGES from all existing .po files
  file(GLOB pofiles translations/*.po)
  foreach(pofile ${pofiles})
    get_filename_component(lang ${pofile} NAME_WE)
    list(APPEND LANGUAGES ${lang})
  endforeach(pofile)
else (LINGUAS STREQUAL "All")
  if (NOT LINGUAS OR LINGUAS STREQUAL "None")
    set (LANGUAGES "")
  else (NOT LINGUAS OR LINGUAS STREQUAL "None")
    string(REGEX MATCHALL [a-zA-Z_]+
           LANGUAGES ${LINGUAS})
  endif (NOT LINGUAS OR LINGUAS STREQUAL "None")
endif (LINGUAS STREQUAL "All")

option(USE_INSTALL_PREFIX "Look for data in CMAKE_INSTALL_PREFIX" ON)

# Visualisations
if(ENABLE_VISUALISATIONS)

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

# Lastfm
if(HAVE_LIBLASTFM)
  list(APPEND SOURCES
    core/albumcoverfetcher.cpp
    radio/fixlastfm.cpp
    radio/lastfmconfig.cpp
    radio/lastfmservice.cpp
    radio/lastfmstationdialog.cpp
    songinfo/echonestsimilarartists.cpp
    songinfo/echonesttags.cpp
    songinfo/lastfmtrackinfoprovider.cpp
    songinfo/tagwidget.cpp
    ui/albumcovermanager.cpp
    ui/albumcovermanagerlist.cpp
    ui/albumcoversearcher.cpp
  )
  list(APPEND HEADERS
    core/albumcoverfetcher.h
    radio/lastfmconfig.h
    radio/lastfmservice.h
    radio/lastfmstationdialog.h
    songinfo/echonestsimilarartists.h
    songinfo/echonesttags.h
    songinfo/lastfmtrackinfoprovider.h
    songinfo/tagwidget.h
    ui/albumcovermanager.h
    ui/albumcovermanagerlist.h
    ui/albumcoversearcher.h
  )
  list(APPEND UI
    radio/lastfmconfig.ui
    radio/lastfmstationdialog.ui
    ui/albumcovermanager.ui
    ui/albumcoversearcher.ui
  )
endif(HAVE_LIBLASTFM)

if(APPLE)
  list(APPEND HEADERS core/macglobalshortcutbackend.h)
  list(APPEND HEADERS devices/macdevicelister.h)
  list(APPEND HEADERS ui/macscreensaver.h)
  list(APPEND HEADERS ui/macsystemtrayicon.h)
  list(APPEND HEADERS widgets/maclineedit.h)
  list(APPEND SOURCES core/macglobalshortcutbackend.mm)
  list(APPEND SOURCES devices/macdevicelister.mm)
  list(APPEND SOURCES ui/macscreensaver.cpp)
  list(APPEND SOURCES ui/macsystemtrayicon.mm)
  list(APPEND SOURCES widgets/maclineedit.mm)
  list(APPEND SOURCES widgets/osd_mac.mm)
  include_directories(${GROWL}/Headers)
else(APPLE)
  if(WIN32)
    list(APPEND SOURCES widgets/osd_win.cpp)
  else(WIN32)
    list(APPEND SOURCES widgets/osd_x11.cpp)
  endif(WIN32)
endif(APPLE)

# DBUS and MPRIS - Linux specific
if(HAVE_DBUS)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dbus)

  # Hack to get it to generate interfaces without namespaces - required
  # because otherwise org::freedesktop::UDisks and
  # org::freedesktop::UDisks::Device conflict.
  list(APPEND QT_DBUSXML2CPP_EXECUTABLE -N)

  # MPRIS DBUS interfaces
  qt4_add_dbus_adaptor(SOURCES
      dbus/org.freedesktop.MediaPlayer.player.xml
      core/mpris1.h mpris::Mpris1Player core/mpris_player MprisPlayer)
  qt4_add_dbus_adaptor(SOURCES
      dbus/org.freedesktop.MediaPlayer.root.xml
      core/mpris1.h mpris::Mpris1Root core/mpris_root MprisRoot)
  qt4_add_dbus_adaptor(SOURCES
      dbus/org.freedesktop.MediaPlayer.tracklist.xml
      core/mpris1.h mpris::Mpris1TrackList core/mpris_tracklist MprisTrackList)

  # MPRIS 2.0 DBUS interfaces
  qt4_add_dbus_adaptor(SOURCES
      dbus/org.mpris.MediaPlayer2.Player.xml
      core/mpris2.h mpris::Mpris2 core/mpris2_player Mpris2Player)
  qt4_add_dbus_adaptor(SOURCES
      dbus/org.mpris.MediaPlayer2.xml
      core/mpris2.h mpris::Mpris2 core/mpris2_root Mpris2Root)
  qt4_add_dbus_adaptor(SOURCES
      dbus/org.mpris.MediaPlayer2.TrackList.xml
      core/mpris2.h mpris::Mpris2 core/mpris2_tracklist Mpris2TrackList)

  # org.freedesktop.Notifications DBUS interface
  qt4_add_dbus_interface(SOURCES
      dbus/org.freedesktop.Notifications.xml
      dbus/notification)

  # org.gnome.SettingsDaemon interface
  qt4_add_dbus_interface(SOURCES
      dbus/org.gnome.SettingsDaemon.MediaKeys.xml
      dbus/gnomesettingsdaemon)


  # DeviceKit DBUS interfaces
  if(HAVE_DEVICEKIT)
    qt4_add_dbus_interface(SOURCES
        dbus/org.freedesktop.UDisks.xml
        dbus/udisks)
    qt4_add_dbus_interface(SOURCES
        dbus/org.freedesktop.UDisks.Device.xml
        dbus/udisksdevice)
  endif(HAVE_DEVICEKIT)

  # MPRIS source
  list(APPEND SOURCES core/mpris.cpp core/mpris1.cpp core/mpris2.cpp)
  list(APPEND HEADERS core/mpris.h   core/mpris1.h   core/mpris2.h)

  # Wiimotedev interface classes
  if(ENABLE_WIIMOTEDEV)
    qt4_add_dbus_interface(SOURCES
        dbus/org.wiimotedev.deviceEvents.xml
        dbus/wiimotedev)

    list(APPEND SOURCES wiimotedev/shortcuts.cpp)
    list(APPEND HEADERS wiimotedev/shortcuts.h)

    list(APPEND SOURCES ui/wiimotedevshortcutgrabber.cpp)
    list(APPEND HEADERS ui/wiimotedevshortcutgrabber.h)


    list(APPEND SOURCES ui/wiimotedevshortcutsconfig.cpp)
    list(APPEND HEADERS ui/wiimotedevshortcutsconfig.h)
  endif(ENABLE_WIIMOTEDEV)

  # DeviceKit lister source
  if(HAVE_DEVICEKIT)
    list(APPEND SOURCES devices/devicekitlister.cpp)
    list(APPEND HEADERS devices/devicekitlister.h)
  endif(HAVE_DEVICEKIT)

  # Gnome Screensaver DBus interface
  list(APPEND SOURCES ui/dbusscreensaver.cpp)
endif(HAVE_DBUS)

# Libgpod device backend
if(HAVE_LIBGPOD)
  include_directories(${LIBGPOD_INCLUDE_DIRS})

  list(APPEND SOURCES devices/gpoddevice.cpp devices/gpodloader.cpp)
  list(APPEND HEADERS devices/gpoddevice.h devices/gpodloader.h)
endif(HAVE_LIBGPOD)

# GIO device backend
if(HAVE_GIO)
  include_directories(${GIO_INCLUDE_DIRS})

  list(APPEND SOURCES devices/giolister.cpp)
  list(APPEND HEADERS devices/giolister.h)
endif(HAVE_GIO)

# libimobiledevice backend and device
if(HAVE_IMOBILEDEVICE)
  include_directories(${IMOBILEDEVICE_INCLUDE_DIRS})
  include_directories(${PLIST_INCLUDE_DIRS})
  include_directories(${PLISTPP_INCLUDE_DIRS})

  list(APPEND SOURCES devices/afcdevice.cpp)
  list(APPEND SOURCES devices/afcfile.cpp)
  list(APPEND SOURCES devices/afctransfer.cpp)
  list(APPEND SOURCES devices/ilister.cpp)
  list(APPEND SOURCES devices/imobiledeviceconnection.cpp)

  list(APPEND HEADERS devices/afcdevice.h)
  list(APPEND HEADERS devices/afcfile.h)
  list(APPEND HEADERS devices/afctransfer.h)
  list(APPEND HEADERS devices/ilister.h)
endif(HAVE_IMOBILEDEVICE)

# mtp device
if(HAVE_LIBMTP)
  include_directories(${LIBMTP_INCLUDE_DIRS})

  list(APPEND SOURCES devices/mtpconnection.cpp)
  list(APPEND SOURCES devices/mtpdevice.cpp)
  list(APPEND SOURCES devices/mtploader.cpp)

  list(APPEND SOURCES devices/mtpconnection.h)
  list(APPEND HEADERS devices/mtpdevice.h)
  list(APPEND HEADERS devices/mtploader.h)
endif(HAVE_LIBMTP)

# Windows media lister
IF(WIN32)
  list(APPEND SOURCES devices/wmdmdevice.cpp)
  list(APPEND SOURCES devices/wmdmlister.cpp)
  list(APPEND SOURCES devices/wmdmloader.cpp)
  list(APPEND SOURCES devices/wmdmprogress.cpp)
  list(APPEND SOURCES devices/wmdmthread.cpp)

  list(APPEND HEADERS devices/wmdmdevice.h)
  list(APPEND HEADERS devices/wmdmlister.h)
  list(APPEND HEADERS devices/wmdmloader.h)
ENDIF(WIN32)

# Mac specific startup stuff
if(APPLE)
  list(APPEND SOURCES core/mac_startup.mm)
endif(APPLE)

if(HAVE_SCRIPTING_PYTHON)
  list(APPEND SOURCES
    scripting/python/pythonengine.cpp
    scripting/python/pythonscript.cpp
  )

  list(APPEND HEADERS
    scripting/python/pythonengine.h
  )

  add_sip_binding(SOURCES
    scripting/python/clementine.sip
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineLibraryBackendAlbum.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementinePlaylistItemOptions.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementinePlaylistItemPtr.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementinePlaylistItemSpecialLoadResult.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineTaskManagerTask.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineQList0100Directory.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineQList0100LibraryBackendAlbum.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineQList0100PlaylistItemPtr.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineQList0100Song.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineQList0100Subdirectory.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineQList0100TaskManagerTask.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipclementineQList0101Playlist.cpp
  )
endif(HAVE_SCRIPTING_PYTHON)

if(HAVE_REMOTE)
  list(APPEND HEADERS
    remote/portforwarder.h
    remote/remote.h
    remote/remoteconfig.h
  )
  list(APPEND SOURCES
    remote/portforwarder.h
    remote/remote.cpp
    remote/remoteconfig.cpp
  )
endif(HAVE_REMOTE)

# OS-specific sources that should be searched for translatable strings even
# if they're not compiled
list(APPEND OTHER_SOURCES
  core/macglobalshortcutbackend.h
  core/macglobalshortcutbackend.mm
  core/modelfuturewatcher.h
  core/mpris.cpp
  core/mpris.h
  core/mpris1.cpp
  core/mpris1.h
  core/mpris2.cpp
  core/mpris2.h
  devices/afcdevice.cpp
  devices/afcdevice.h
  devices/afcfile.cpp
  devices/afcfile.h
  devices/afctransfer.cpp
  devices/afctransfer.h
  devices/devicekitlister.h
  devices/devicekitlister.cpp
  devices/gpoddevice.cpp
  devices/gpoddevice.h
  devices/gpodloader.cpp
  devices/gpodloader.h
  devices/ilister.cpp
  devices/ilister.h
  devices/imobiledeviceconnection.cpp
  devices/imobiledeviceconnection.h
  devices/mtpconnection.cpp
  devices/mtpconnection.h
  devices/mtpdevice.cpp
  devices/mtpdevice.h
  devices/mtploader.cpp
  devices/mtploader.h
  devices/wmdmlister.cpp
  devices/wmdmlister.h
  devices/wmdmloader.h
  devices/wmdmloader.cpp
  remote/remote.cpp
  remote/remote.h
  remote/remoteconfig.cpp
  remote/remoteconfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/ui_remoteconfig.h
  ui/macsystemtrayicon.h
  ui/macsystemtrayicon.mm
  ui/wiimotedevshortcutsconfig.cpp
  ui/wiimotedevshortcutsconfig.h
  ui/wiimotedevshortcutgrabber.cpp
  ui/wiimotedevshortcutgrabber.h
  ${CMAKE_CURRENT_BINARY_DIR}/ui_wiimotedevshortcutsconfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/ui_wiimotedevshortcutgrabber.h
  widgets/osd_mac.mm
  widgets/osd_win.cpp
  widgets/osd_x11.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/version.h)

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
  chardet
  ${ECHONEST_LIBRARIES}
  ${GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${QT_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${QTSINGLEAPPLICATION_LIBRARIES}
  ${QTSINGLECOREAPPLICATION_LIBRARIES}
  ${QTIOCOMPRESSOR_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  dl
  z
)

if(ENABLE_VISUALISATIONS)
  target_link_libraries(clementine_lib ${LIBPROJECTM_LIBRARIES})
endif(ENABLE_VISUALISATIONS)

if(HAVE_LIBLASTFM)
  target_link_libraries(clementine_lib ${LASTFM_LIBRARIES})
endif(HAVE_LIBLASTFM)

if(HAVE_LIBGPOD)
  target_link_libraries(clementine_lib ${LIBGPOD_LIBRARIES})
endif(HAVE_LIBGPOD)

if(HAVE_GIO)
  target_link_libraries(clementine_lib ${GIO_LIBRARIES})
endif(HAVE_GIO)

if(HAVE_LIBARCHIVE)
  target_link_libraries(clementine_lib ${ARCHIVE_LIBRARIES})
endif(HAVE_LIBARCHIVE)

if(HAVE_IMOBILEDEVICE)
  target_link_libraries(clementine_lib
    ${IMOBILEDEVICE_LIBRARIES}
    ${PLIST_LIBRARIES}
    ${USBMUXD_LIBRARIES}
    gstafcsrc
  )
  link_directories(${USBMUXD_LIBRARY_DIRS})
endif(HAVE_IMOBILEDEVICE)

if(HAVE_LIBMTP)
  target_link_libraries(clementine_lib ${LIBMTP_LIBRARIES})
endif(HAVE_LIBMTP)

if(HAVE_LIBINDICATE)
  target_link_libraries(clementine_lib ${INDICATEQT_LIBRARIES})
endif(HAVE_LIBINDICATE)

if(HAVE_SCRIPTING_PYTHON)
  if(WIN32)
    target_link_libraries(clementine_lib
      ${PYQT_QTCORE_LIBRARY}
      ${PYQT_QTGUI_LIBRARY}
      ${PYQT_QTNETWORK_LIBRARY}
      ${PYQT_QT_LIBRARY}
      ${PYQT_QPYGUI_LIBRARY}
      ${PYQT_QPYCORE_LIBRARY}
      ${SIP_LIBRARY}
    )
  endif(WIN32)

  target_link_libraries(clementine_lib ${PYTHON_LIBRARIES})
endif(HAVE_SCRIPTING_PYTHON)

if(HAVE_REMOTE)
  target_link_libraries(clementine_lib ${GLOOX_LIBRARIES})
  target_link_libraries(clementine_lib xrme)
  target_link_libraries(clementine_lib portfwd)
  target_link_libraries(clementine_lib qjson)
  link_directories(${GLOOX_LIBRARY_DIRS})
endif(HAVE_REMOTE)

if (APPLE)
  target_link_libraries(clementine_lib
    ${GROWL}
    /System/Library/Frameworks/AppKit.framework
    /System/Library/Frameworks/Carbon.framework
    /System/Library/Frameworks/DiskArbitration.framework
    /System/Library/Frameworks/Foundation.framework
    /System/Library/Frameworks/IOKit.framework
  )
  if (HAVE_SPARKLE)
    include_directories(${SPARKLE}/Headers)
    target_link_libraries(clementine_lib ${SPARKLE})
  endif (HAVE_SPARKLE)
else (APPLE)
  target_link_libraries(clementine_lib ${QXT_LIBRARIES})
endif (APPLE)

# Link against the qsqlite plugin on windows and mac
if(HAVE_STATIC_SQLITE)
  set(3RDPARTY_SQLITE_LIBRARY qsqlite)
  target_link_libraries(clementine_lib qsqlite)
endif(HAVE_STATIC_SQLITE)

if (WIN32)
  target_link_libraries(clementine_lib
    ${ZLIB_LIBRARIES}
    ${MSWMDM_LIBRARIES}
    ${SAC_SHIM_LIBRARIES}
    ${QTSPARKLE_LIBRARIES}
    qtwin
  )
endif (WIN32)

if (LINUX)
  # Hack: the Gold linker pays attention to the order that libraries are
  # specified on the link line.  -lX11 and -ldl are provided earlier in the link
  # command but they're actually used by libraries that appear after them, so
  # they end up getting ignored.  This appends them to the very end of the link
  # line, ensuring they're always used.
  target_link_libraries(clementine_lib -lX11 -ldl)
endif (LINUX)

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

  if (HAVE_SCRIPTING_PYTHON)
    install(DIRECTORY "/Library/Python/2.6/site-packages/PyQt4/uic"
      DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/PlugIns/PyQt4")
    install(FILES "/Library/Python/2.6/site-packages/PyQt4/__init__.py"
      DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/PlugIns/PyQt4")
  endif (HAVE_SCRIPTING_PYTHON)


  if (HAVE_SPARKLE)
    install(DIRECTORY "${SPARKLE}/Versions/Current/Resources"
      DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Frameworks/Sparkle.framework")
  endif (HAVE_SPARKLE)

  install(DIRECTORY "${GROWL}/Versions/Current/Resources"
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Frameworks/Growl.framework")

  add_custom_command(TARGET clementine
    POST_BUILD
    COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/../dist/macdeploy.py ${PROJECT_BINARY_DIR}/clementine.app -f
  )

  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_RPM}.dmg
    ${CMAKE_COMMAND} -E remove -f ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_RPM}.dmg
    COMMAND macdeployqt clementine.app -no-plugins -dmg
    COMMAND ${CMAKE_COMMAND} -E rename
      ${PROJECT_BINARY_DIR}/clementine.dmg
      ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_RPM}.dmg
    DEPENDS clementine
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
  add_custom_target(dmg
    DEPENDS ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_RPM}.dmg)
else (APPLE)
  install(TARGETS clementine
    RUNTIME DESTINATION bin
  )
endif (APPLE)
