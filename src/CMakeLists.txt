
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wall -Wno-sign-compare -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-unused-private-field -Wno-unknown-warning-option --std=c++0x -U__STRICT_ANSI__")

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

# Activate fast QString concatenation
if (QT_VERSION_MINOR GREATER 5)
  if (QT_VERSION_MINOR GREATER 7)
    add_definitions(-DQT_USE_QSTRINGBUILDER)
  else(QT_VERSION_MINOR GREATER 7)
    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
  endif(QT_VERSION_MINOR GREATER 7)
endif(QT_VERSION_MINOR GREATER 5)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DBOOST_BIND_NO_PLACEHOLDERS)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${LIBPROJECTM_INCLUDE_DIRS})
include_directories(${QTSINGLEAPPLICATION_INCLUDE_DIRS})
include_directories(${QTIOCOMPRESSOR_INCLUDE_DIRS})
include_directories(${QXT_INCLUDE_DIRS})
include_directories(${SHA2_INCLUDE_DIRS})
include_directories(${CHROMAPRINT_INCLUDE_DIRS})
include_directories(${MYGPOQT_INCLUDE_DIRS})

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIR})

if(HAVE_LIBLASTFM)
  include_directories(${LASTFM_INCLUDE_DIRS})
endif(HAVE_LIBLASTFM)

if(HAVE_BREAKPAD)
  include_directories(../3rdparty/google-breakpad)
endif(HAVE_BREAKPAD)

include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-remote)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-remote)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-spotifyblob)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-spotifyblob)

cmake_policy(SET CMP0011 NEW)
include(../cmake/ParseArguments.cmake)
include(../cmake/Translations.cmake)

set(SOURCES
  analyzers/analyzerbase.cpp
  analyzers/analyzercontainer.cpp
  analyzers/baranalyzer.cpp
  analyzers/blockanalyzer.cpp
  analyzers/boomanalyzer.cpp
  analyzers/rainbowanalyzer.cpp
  analyzers/sonogram.cpp
  analyzers/turbine.cpp
  analyzers/fht.cpp

  core/appearance.cpp
  core/application.cpp
  core/backgroundstreams.cpp
  core/commandlineoptions.cpp
  core/crashreporting.cpp
  core/database.cpp
  core/deletefiles.cpp
  core/filesystemmusicstorage.cpp
  core/filesystemwatcherinterface.cpp
  core/globalshortcutbackend.cpp
  core/globalshortcuts.cpp
  core/gnomeglobalshortcutbackend.cpp
  core/mergedproxymodel.cpp
  core/metatypes.cpp
  core/multisortfilterproxy.cpp
  core/musicstorage.cpp
  core/network.cpp
  core/networkproxyfactory.cpp
  core/organise.cpp
  core/organiseformat.cpp
  core/player.cpp
  core/qtfslistener.cpp
  core/qxtglobalshortcutbackend.cpp
  core/scopedtransaction.cpp
  core/settingsprovider.cpp
  core/signalchecker.cpp
  core/song.cpp
  core/songloader.cpp
  core/stylesheetloader.cpp
  core/tagreaderclient.cpp
  core/taskmanager.cpp
  core/thread.cpp
  core/urlhandler.cpp
  core/utilities.cpp

  covers/albumcoverexporter.cpp
  covers/albumcoverfetcher.cpp
  covers/albumcoverfetchersearch.cpp
  covers/albumcoverloader.cpp
  covers/amazoncoverprovider.cpp
  covers/coverexportrunnable.cpp
  covers/coverprovider.cpp
  covers/coverproviders.cpp
  covers/coversearchstatistics.cpp
  covers/coversearchstatisticsdialog.cpp
  covers/currentartloader.cpp
  covers/kittenloader.cpp
  covers/musicbrainzcoverprovider.cpp

  devices/connecteddevice.cpp
  devices/devicedatabasebackend.cpp
  devices/devicelister.cpp
  devices/devicemanager.cpp
  devices/deviceproperties.cpp
  devices/devicestatefiltermodel.cpp
  devices/deviceview.cpp
  devices/deviceviewcontainer.cpp
  devices/filesystemdevice.cpp

  engines/devicefinder.cpp
  engines/enginebase.cpp
  engines/gstengine.cpp
  engines/gstenginepipeline.cpp
  engines/gstelementdeleter.cpp

  globalsearch/digitallyimportedsearchprovider.cpp
  globalsearch/globalsearch.cpp
  globalsearch/globalsearchitemdelegate.cpp
  globalsearch/globalsearchmodel.cpp
  globalsearch/globalsearchsettingspage.cpp
  globalsearch/globalsearchsortmodel.cpp
  globalsearch/globalsearchview.cpp
  globalsearch/icecastsearchprovider.cpp
  globalsearch/librarysearchprovider.cpp
  globalsearch/savedradiosearchprovider.cpp
  globalsearch/searchprovider.cpp
  globalsearch/searchproviderstatuswidget.cpp
  globalsearch/simplesearchprovider.cpp
  globalsearch/somafmsearchprovider.cpp
  globalsearch/intergalacticfmsearchprovider.cpp
  globalsearch/soundcloudsearchprovider.cpp
  globalsearch/spotifysearchprovider.cpp
  globalsearch/suggestionwidget.cpp
  globalsearch/urlsearchprovider.cpp

  internet/core/cloudfilesearchprovider.cpp
  internet/core/cloudfileservice.cpp
  internet/digitally/digitallyimportedclient.cpp
  internet/digitally/digitallyimportedservicebase.cpp
  internet/digitally/digitallyimportedsettingspage.cpp
  internet/digitally/digitallyimportedurlhandler.cpp
  internet/core/geolocator.cpp
  internet/icecast/icecastbackend.cpp
  internet/icecast/icecastfilterwidget.cpp
  internet/icecast/icecastmodel.cpp
  internet/icecast/icecastservice.cpp
  internet/core/internetmodel.cpp
  internet/core/internetplaylistitem.cpp
  internet/core/internetservice.cpp
  internet/core/internetshowsettingspage.cpp
  internet/core/internetview.cpp
  internet/core/internetviewcontainer.cpp
  internet/jamendo/jamendodynamicplaylist.cpp
  internet/jamendo/jamendoplaylistitem.cpp
  internet/jamendo/jamendoservice.cpp
  internet/core/localredirectserver.cpp
  internet/magnatune/magnatunedownloaddialog.cpp
  internet/magnatune/magnatuneplaylistitem.cpp
  internet/magnatune/magnatuneservice.cpp
  internet/magnatune/magnatunesettingspage.cpp
  internet/magnatune/magnatuneurlhandler.cpp
  internet/core/oauthenticator.cpp
  internet/internetradio/savedradio.cpp
  internet/core/searchboxwidget.cpp
  internet/somafm/somafmservice.cpp
  internet/somafm/somafmurlhandler.cpp
  internet/intergalacticfm/intergalacticfmservice.cpp
  internet/intergalacticfm/intergalacticfmurlhandler.cpp
  internet/soundcloud/soundcloudservice.cpp
  internet/soundcloud/soundcloudsettingspage.cpp
  internet/spotify/spotifyserver.cpp
  internet/spotify/spotifyservice.cpp
  internet/spotify/spotifysettingspage.cpp
  internet/subsonic/subsonicservice.cpp
  internet/subsonic/subsonicsettingspage.cpp
  internet/subsonic/subsonicurlhandler.cpp
  internet/subsonic/subsonicdynamicplaylist.cpp

  library/groupbydialog.cpp
  library/library.cpp
  library/librarybackend.cpp
  library/librarydirectorymodel.cpp
  library/libraryfilterwidget.cpp
  library/librarymodel.cpp
  library/libraryplaylistitem.cpp
  library/libraryquery.cpp
  library/librarysettingspage.cpp
  library/libraryview.cpp
  library/libraryviewcontainer.cpp
  library/librarywatcher.cpp
  library/savedgroupingmanager.cpp
  library/sqlrow.cpp

  musicbrainz/acoustidclient.cpp
  musicbrainz/chromaprinter.cpp
  musicbrainz/musicbrainzclient.cpp
  musicbrainz/tagfetcher.cpp

  networkremote/incomingdataparser.cpp
  networkremote/networkremote.cpp
  networkremote/networkremotehelper.cpp
  networkremote/outgoingdatacreator.cpp
  networkremote/remoteclient.cpp
  networkremote/songsender.cpp
  networkremote/zeroconf.cpp

  playlist/dynamicplaylistcontrols.cpp
  playlist/playlist.cpp
  playlist/playlistbackend.cpp
  playlist/playlistcontainer.cpp
  playlist/playlistdelegates.cpp
  playlist/playlistfilterparser.cpp
  playlist/playlistfilter.cpp
  playlist/playlistheader.cpp
  playlist/playlistitem.cpp
  playlist/playlistlistcontainer.cpp
  playlist/playlistlistmodel.cpp
  playlist/playlistlistview.cpp
  playlist/playlistmanager.cpp
  playlist/playlistsaveoptionsdialog.cpp
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
  playlistparsers/wplparser.cpp
  playlistparsers/xmlparser.cpp
  playlistparsers/xspfparser.cpp

  internet/podcasts/addpodcastbyurl.cpp
  internet/podcasts/addpodcastdialog.cpp
  internet/podcasts/addpodcastpage.cpp
  internet/podcasts/fixedopmlpage.cpp
  internet/podcasts/gpoddersearchpage.cpp
  internet/podcasts/gpoddersync.cpp
  internet/podcasts/gpoddertoptagsmodel.cpp
  internet/podcasts/gpoddertoptagspage.cpp
  internet/podcasts/itunessearchpage.cpp
  internet/podcasts/podcast.cpp
  internet/podcasts/podcastbackend.cpp
  internet/podcasts/podcastdiscoverymodel.cpp
  internet/podcasts/podcastdeleter.cpp
  internet/podcasts/podcastdownloader.cpp
  internet/podcasts/podcastepisode.cpp
  internet/podcasts/podcastinfowidget.cpp
  internet/podcasts/podcastservice.cpp
  internet/podcasts/podcastservicemodel.cpp
  internet/podcasts/podcastsettingspage.cpp
  internet/podcasts/podcastparser.cpp
  internet/podcasts/podcastupdater.cpp
  internet/podcasts/podcasturlloader.cpp

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

  songinfo/artistbiography.cpp
  songinfo/artistinfoview.cpp
  songinfo/collapsibleinfoheader.cpp
  songinfo/collapsibleinfopane.cpp
  songinfo/songinfobase.cpp
  songinfo/songinfofetcher.cpp
  songinfo/songinfoprovider.cpp
  songinfo/songinfosettingspage.cpp
  songinfo/songinfotextview.cpp
  songinfo/songinfoview.cpp
  songinfo/songkickconcerts.cpp
  songinfo/songkickconcertwidget.cpp
  songinfo/songplaystats.cpp
  songinfo/spotifyimages.cpp
  songinfo/streamdiscoverer.cpp
  songinfo/taglyricsinfoprovider.cpp
  songinfo/ultimatelyricslyric.cpp
  songinfo/ultimatelyricsprovider.cpp
  songinfo/ultimatelyricsreader.cpp

  transcoder/transcodedialog.cpp
  transcoder/transcoder.cpp
  transcoder/transcoderoptionsaac.cpp
  transcoder/transcoderoptionsdialog.cpp
  transcoder/transcoderoptionsflac.cpp
  transcoder/transcoderoptionsmp3.cpp
  transcoder/transcoderoptionsopus.cpp
  transcoder/transcoderoptionsspeex.cpp
  transcoder/transcoderoptionsvorbis.cpp
  transcoder/transcoderoptionswma.cpp
  transcoder/transcodersettingspage.cpp

  ui/about.cpp
  ui/addstreamdialog.cpp
  ui/albumcoverchoicecontroller.cpp
  ui/albumcoverexport.cpp
  ui/albumcovermanager.cpp
  ui/albumcovermanagerlist.cpp
  ui/albumcoversearcher.cpp
  ui/appearancesettingspage.cpp
  ui/backgroundstreamssettingspage.cpp
  ui/behavioursettingspage.cpp
  ui/console.cpp
  ui/coverfromurldialog.cpp
  ui/edittagdialog.cpp
  ui/equalizer.cpp
  ui/flowlayout.cpp
  ui/globalshortcutgrabber.cpp
  ui/globalshortcutssettingspage.cpp
  ui/iconloader.cpp
  ui/mainwindow.cpp
  ui/networkproxysettingspage.cpp
  ui/networkremotesettingspage.cpp
  ui/notificationssettingspage.cpp
  ui/organisedialog.cpp
  ui/organiseerrordialog.cpp
  ui/playbacksettingspage.cpp
  ui/qtsystemtrayicon.cpp
  ui/screensaver.cpp
  ui/settingsdialog.cpp
  ui/settingspage.cpp
  ui/standarditemiconloader.cpp
  ui/streamdetailsdialog.cpp
  ui/systemtrayicon.cpp
  ui/trackselectiondialog.cpp
  ui/windows7thumbbar.cpp

  widgets/autoexpandingtreeview.cpp
  widgets/busyindicator.cpp
  widgets/clickablelabel.cpp
  widgets/didyoumean.cpp
  widgets/elidedlabel.cpp
  widgets/equalizerslider.cpp
  widgets/errordialog.cpp
  widgets/fancytabwidget.cpp
  widgets/favoritewidget.cpp
  widgets/fileview.cpp
  widgets/fileviewlist.cpp
  widgets/forcescrollperpixel.cpp
  widgets/freespacebar.cpp
  widgets/fullscreenhypnotoad.cpp
  widgets/groupediconview.cpp
  widgets/lineedit.cpp
  widgets/linetextedit.cpp
  widgets/loginstatewidget.cpp
  widgets/multiloadingindicator.cpp
  widgets/nowplayingwidget.cpp
  widgets/osd.cpp
  widgets/osdpretty.cpp
  widgets/prettyimage.cpp
  widgets/prettyimageview.cpp
  widgets/progressitemdelegate.cpp
  widgets/ratingwidget.cpp
  widgets/renametablineedit.cpp
  widgets/sliderwidget.cpp
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
  analyzers/rainbowanalyzer.h
  analyzers/sonogram.h
  analyzers/turbine.h

  core/application.h
  core/backgroundstreams.h
  core/crashreporting.h
  core/database.h
  core/deletefiles.h
  core/filesystemwatcherinterface.h
  core/globalshortcuts.h
  core/globalshortcutbackend.h
  core/gnomeglobalshortcutbackend.h
  core/mergedproxymodel.h
  core/mimedata.h
  core/network.h
  core/organise.h
  core/player.h
  core/qtfslistener.h
  core/songloader.h
  core/tagreaderclient.h
  core/taskmanager.h
  core/urlhandler.h

  covers/albumcoverexporter.h
  covers/albumcoverfetcher.h
  covers/albumcoverfetchersearch.h
  covers/albumcoverloader.h
  covers/amazoncoverprovider.h
  covers/coverexportrunnable.h
  covers/coverprovider.h
  covers/coverproviders.h
  covers/coversearchstatisticsdialog.h
  covers/currentartloader.h
  covers/kittenloader.h
  covers/musicbrainzcoverprovider.h

  devices/connecteddevice.h
  devices/devicedatabasebackend.h
  devices/devicelister.h
  devices/devicemanager.h
  devices/deviceproperties.h
  devices/devicestatefiltermodel.h
  devices/deviceview.h
  devices/deviceviewcontainer.h
  devices/filesystemdevice.h

  engines/enginebase.h
  engines/gstengine.h
  engines/gstenginepipeline.h
  engines/gstelementdeleter.h

  globalsearch/globalsearch.h
  globalsearch/globalsearchmodel.h
  globalsearch/globalsearchsettingspage.h
  globalsearch/globalsearchview.h
  globalsearch/searchprovider.h
  globalsearch/simplesearchprovider.h
  globalsearch/soundcloudsearchprovider.h
  globalsearch/spotifysearchprovider.h
  globalsearch/suggestionwidget.h

  internet/core/cloudfileservice.h
  internet/digitally/digitallyimportedclient.h
  internet/digitally/digitallyimportedservicebase.h
  internet/digitally/digitallyimportedsettingspage.h
  internet/core/geolocator.h
  internet/icecast/icecastbackend.h
  internet/icecast/icecastfilterwidget.h
  internet/icecast/icecastmodel.h
  internet/icecast/icecastservice.h
  internet/core/internetmimedata.h
  internet/core/internetmodel.h
  internet/core/internetservice.h
  internet/core/internetshowsettingspage.h
  internet/core/internetsongmimedata.h
  internet/core/internetview.h
  internet/core/internetviewcontainer.h
  internet/jamendo/jamendodynamicplaylist.h
  internet/jamendo/jamendoservice.h
  internet/core/localredirectserver.h
  internet/magnatune/magnatunedownloaddialog.h
  internet/magnatune/magnatuneservice.h
  internet/magnatune/magnatunesettingspage.h
  internet/core/oauthenticator.h
  internet/internetradio/savedradio.h
  internet/core/scrobbler.h
  internet/core/searchboxwidget.h
  internet/somafm/somafmservice.h
  internet/somafm/somafmurlhandler.h
  internet/intergalacticfm/intergalacticfmservice.h
  internet/intergalacticfm/intergalacticfmurlhandler.h
  internet/soundcloud/soundcloudservice.h
  internet/soundcloud/soundcloudsettingspage.h
  internet/spotify/spotifyserver.h
  internet/spotify/spotifyservice.h
  internet/spotify/spotifysettingspage.h
  internet/subsonic/subsonicservice.h
  internet/subsonic/subsonicsettingspage.h
  internet/subsonic/subsonicurlhandler.h
  internet/subsonic/subsonicdynamicplaylist.h

  library/groupbydialog.h
  library/library.h
  library/librarybackend.h
  library/librarydirectorymodel.h
  library/libraryfilterwidget.h
  library/librarymodel.h
  library/librarysettingspage.h
  library/libraryview.h
  library/libraryviewcontainer.h
  library/librarywatcher.h
  library/savedgroupingmanager.h
  
  musicbrainz/acoustidclient.h
  musicbrainz/musicbrainzclient.h
  musicbrainz/tagfetcher.h
  
  networkremote/networkremotehelper.h
  networkremote/networkremote.h
  networkremote/incomingdataparser.h
  networkremote/outgoingdatacreator.h
  networkremote/remoteclient.h
  networkremote/songsender.h

  playlist/dynamicplaylistcontrols.h
  playlist/playlist.h
  playlist/playlistbackend.h
  playlist/playlistcontainer.h
  playlist/playlistdelegates.h
  playlist/playlistfilter.h
  playlist/playlistheader.h
  playlist/playlistitemmimedata.h
  playlist/playlistlistcontainer.h
  playlist/playlistlistmodel.h
  playlist/playlistlistview.h
  playlist/playlistmanager.h
  playlist/playlistsaveoptionsdialog.h
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

  internet/podcasts/addpodcastbyurl.h
  internet/podcasts/addpodcastdialog.h
  internet/podcasts/addpodcastpage.h
  internet/podcasts/fixedopmlpage.h
  internet/podcasts/gpoddersearchpage.h
  internet/podcasts/gpoddersync.h
  internet/podcasts/gpoddertoptagsmodel.h
  internet/podcasts/gpoddertoptagspage.h
  internet/podcasts/itunessearchpage.h
  internet/podcasts/podcastbackend.h
  internet/podcasts/podcastdiscoverymodel.h
  internet/podcasts/podcastdeleter.h
  internet/podcasts/podcastdownloader.h
  internet/podcasts/podcastinfowidget.h
  internet/podcasts/podcastservice.h
  internet/podcasts/podcastservicemodel.h
  internet/podcasts/podcastsettingspage.h
  internet/podcasts/podcastupdater.h
  internet/podcasts/podcasturlloader.h

  smartplaylists/generator.h
  smartplaylists/generatorinserter.h
  smartplaylists/generatormimedata.h
  smartplaylists/querywizardplugin.h
  smartplaylists/searchpreview.h
  smartplaylists/searchtermwidget.h
  smartplaylists/wizard.h
  smartplaylists/wizardplugin.h

  songinfo/artistbiography.h
  songinfo/artistinfoview.h
  songinfo/collapsibleinfoheader.h
  songinfo/collapsibleinfopane.h
  songinfo/songinfobase.h
  songinfo/songinfofetcher.h
  songinfo/songinfoprovider.h
  songinfo/songinfosettingspage.h
  songinfo/songinfotextview.h
  songinfo/songinfoview.h
  songinfo/songkickconcerts.h
  songinfo/songkickconcertwidget.h
  songinfo/songplaystats.h
  songinfo/spotifyimages.h
  songinfo/streamdiscoverer.h
  songinfo/taglyricsinfoprovider.h
  songinfo/ultimatelyricslyric.h
  songinfo/ultimatelyricsprovider.h
  songinfo/ultimatelyricsreader.h

  transcoder/transcodedialog.h
  transcoder/transcoder.h
  transcoder/transcoderoptionsdialog.h
  transcoder/transcoderoptionsmp3.h
  transcoder/transcodersettingspage.h

  ui/about.h
  ui/addstreamdialog.h
  ui/albumcoverchoicecontroller.h
  ui/albumcoverexport.h
  ui/albumcovermanager.h
  ui/albumcovermanagerlist.h
  ui/albumcoversearcher.h
  ui/appearancesettingspage.h
  ui/backgroundstreamssettingspage.h
  ui/behavioursettingspage.h
  ui/console.h
  ui/coverfromurldialog.h
  ui/edittagdialog.h
  ui/equalizer.h
  ui/globalshortcutgrabber.h
  ui/globalshortcutssettingspage.h
  ui/mainwindow.h
  ui/networkproxysettingspage.h
  ui/networkremotesettingspage.h
  ui/notificationssettingspage.h
  ui/organisedialog.h
  ui/organiseerrordialog.h
  ui/playbacksettingspage.h
  ui/qtsystemtrayicon.h
  ui/settingsdialog.h
  ui/settingspage.h
  ui/standarditemiconloader.h
  ui/streamdetailsdialog.h
  ui/systemtrayicon.h
  ui/trackselectiondialog.h
  ui/windows7thumbbar.h

  widgets/autoexpandingtreeview.h
  widgets/busyindicator.h
  widgets/clickablelabel.h
  widgets/didyoumean.h
  widgets/elidedlabel.h
  widgets/equalizerslider.h
  widgets/errordialog.h
  widgets/fancytabwidget.h
  widgets/favoritewidget.h
  widgets/fileview.h
  widgets/fileviewlist.h
  widgets/freespacebar.h
  widgets/groupediconview.h
  widgets/lineedit.h
  widgets/linetextedit.h
  widgets/loginstatewidget.h
  widgets/multiloadingindicator.h
  widgets/nowplayingwidget.h
  widgets/osd.h
  widgets/osdpretty.h
  widgets/prettyimage.h
  widgets/prettyimageview.h
  widgets/progressitemdelegate.h
  widgets/ratingwidget.h
  widgets/renametablineedit.h
  widgets/sliderwidget.h
  widgets/stickyslider.h
  widgets/stretchheaderview.h
  widgets/trackslider.h
  widgets/tracksliderpopup.h
  widgets/tracksliderslider.h
  widgets/widgetfadehelper.h
)

set(UI
  covers/coversearchstatisticsdialog.ui

  devices/deviceproperties.ui
  devices/deviceviewcontainer.ui

  globalsearch/globalsearchsettingspage.ui
  globalsearch/globalsearchview.ui
  globalsearch/searchproviderstatuswidget.ui
  globalsearch/suggestionwidget.ui

  internet/digitally/digitallyimportedsettingspage.ui
  internet/icecast/icecastfilterwidget.ui
  internet/core/internetshowsettingspage.ui
  internet/core/internetviewcontainer.ui
  internet/magnatune/magnatunedownloaddialog.ui
  internet/magnatune/magnatunesettingspage.ui
  internet/core/searchboxwidget.ui
  internet/soundcloud/soundcloudsettingspage.ui
  internet/spotify/spotifysettingspage.ui
  internet/subsonic/subsonicsettingspage.ui

  library/groupbydialog.ui
  library/libraryfilterwidget.ui
  library/librarysettingspage.ui
  library/libraryviewcontainer.ui
  library/savedgroupingmanager.ui

  playlist/dynamicplaylistcontrols.ui
  playlist/playlistcontainer.ui
  playlist/playlistsaveoptionsdialog.ui
  playlist/playlistlistcontainer.ui
  playlist/playlistsequence.ui
  playlist/queuemanager.ui

  internet/podcasts/addpodcastbyurl.ui
  internet/podcasts/addpodcastdialog.ui
  internet/podcasts/gpoddersearchpage.ui
  internet/podcasts/itunessearchpage.ui
  internet/podcasts/podcastinfowidget.ui
  internet/podcasts/podcastsettingspage.ui

  smartplaylists/querysearchpage.ui
  smartplaylists/querysortpage.ui
  smartplaylists/searchpreview.ui
  smartplaylists/searchtermwidget.ui
  smartplaylists/wizardfinishpage.ui

  songinfo/songkickconcertwidget.ui
  songinfo/songinfosettingspage.ui

  transcoder/transcodedialog.ui
  transcoder/transcodelogdialog.ui
  transcoder/transcoderoptionsaac.ui
  transcoder/transcoderoptionsdialog.ui
  transcoder/transcoderoptionsflac.ui
  transcoder/transcoderoptionsmp3.ui
  transcoder/transcoderoptionsopus.ui
  transcoder/transcoderoptionsspeex.ui
  transcoder/transcoderoptionsvorbis.ui
  transcoder/transcoderoptionswma.ui
  transcoder/transcodersettingspage.ui

  ui/about.ui
  ui/addstreamdialog.ui
  ui/albumcoverexport.ui
  ui/albumcovermanager.ui
  ui/albumcoversearcher.ui
  ui/appearancesettingspage.ui
  ui/backgroundstreamssettingspage.ui
  ui/behavioursettingspage.ui
  ui/console.ui
  ui/coverfromurldialog.ui
  ui/edittagdialog.ui
  ui/equalizer.ui
  ui/globalshortcutgrabber.ui
  ui/globalshortcutssettingspage.ui
  ui/mainwindow.ui
  ui/networkproxysettingspage.ui
  ui/networkremotesettingspage.ui
  ui/notificationssettingspage.ui
  ui/organisedialog.ui
  ui/organiseerrordialog.ui
  ui/playbacksettingspage.ui
  ui/settingsdialog.ui
  ui/streamdetailsdialog.ui
  ui/trackselectiondialog.ui

  widgets/equalizerslider.ui
  widgets/errordialog.ui
  widgets/fileview.ui
  widgets/loginstatewidget.ui
  widgets/osdpretty.ui
  widgets/trackslider.ui

  wiimotedev/wiimotesettingspage.ui
  wiimotedev/wiimoteshortcutgrabber.ui
)

set(RESOURCES
  ../data/data.qrc
)

set(OTHER_SOURCES)

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
    string(REGEX MATCHALL [a-zA-Z_@]+
           LANGUAGES ${LINGUAS})
  endif (NOT LINGUAS OR LINGUAS STREQUAL "None")
endif (LINGUAS STREQUAL "All")

option(USE_INSTALL_PREFIX "Look for data in CMAKE_INSTALL_PREFIX" ON)

# Visualisations
optional_source(ENABLE_VISUALISATIONS
  SOURCES
    visualisations/projectmpresetmodel.cpp
    visualisations/projectmvisualisation.cpp
    visualisations/visualisationcontainer.cpp
    visualisations/visualisationoverlay.cpp
    visualisations/visualisationselector.cpp
  HEADERS
    visualisations/projectmpresetmodel.h
    visualisations/projectmvisualisation.h
    visualisations/visualisationcontainer.h
    visualisations/visualisationoverlay.h
    visualisations/visualisationselector.h
  UI
    visualisations/visualisationoverlay.ui
    visualisations/visualisationselector.ui
)

# Lastfm
optional_source(HAVE_LIBLASTFM
  SOURCES
    covers/lastfmcoverprovider.cpp
    internet/lastfm/fixlastfm.cpp
    internet/lastfm/lastfmcompat.cpp
    internet/lastfm/lastfmservice.cpp
    internet/lastfm/lastfmsettingspage.cpp
    songinfo/lastfmtrackinfoprovider.cpp
    songinfo/tagwidget.cpp
  HEADERS
    covers/lastfmcoverprovider.h
    internet/lastfm/lastfmservice.h
    internet/lastfm/lastfmsettingspage.h
    songinfo/lastfmtrackinfoprovider.h
    songinfo/tagwidget.h
  UI
    internet/lastfm/lastfmsettingspage.ui
)


optional_source(HAVE_SPOTIFY_DOWNLOADER
  SOURCES
    internet/spotify/spotifyblobdownloader.cpp
  HEADERS
    internet/spotify/spotifyblobdownloader.h
  INCLUDE_DIRECTORIES
    ${CRYPTOPP_INCLUDE_DIRS}
)

# Platform specific - OS X
optional_source(APPLE
  INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/google-breakpad/client/mac/build/Release/Breakpad.framework
  SOURCES
    core/macfslistener.mm
    core/macglobalshortcutbackend.mm
    core/mac_startup.mm
    engines/osxdevicefinder.cpp
    networkremote/bonjour.mm
    ui/globalshortcutgrabber.mm
    ui/macscreensaver.cpp
    ui/macsystemtrayicon.mm
    widgets/osd_mac.mm
  HEADERS
    core/macfslistener.h
    core/macglobalshortcutbackend.h
    ui/macsystemtrayicon.h
)

if(APPLE)
  optional_source(HAVE_LIBMTP
   SOURCES
      devices/macdevicelister.mm
    HEADERS
      devices/macdevicelister.h
  )
endif()

# Platform specific - Windows
optional_source(WIN32
  SOURCES
    engines/directsounddevicefinder.cpp
    networkremote/tinysvcmdns.cpp
    ui/windowsscreensaver.cpp
    widgets/osd_win.cpp
  INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/3rdparty/tinysvcmdns
)

# Platform specific - X11
optional_source(LINUX SOURCES widgets/osd_x11.cpp)

# DBUS and MPRIS - Linux specific
if(HAVE_DBUS)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dbus)

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

  # MPRIS 2.1 DBUS interfaces
  qt4_add_dbus_adaptor(SOURCES
      dbus/org.mpris.MediaPlayer2.Playlists.xml
      core/mpris2.h mpris::Mpris2 core/mpris2_playlists Mpris2Playlists)

  # org.freedesktop.Notifications DBUS interface
  qt4_add_dbus_interface(SOURCES
      dbus/org.freedesktop.Notifications.xml
      dbus/notification)

  # org.gnome.SettingsDaemon interface
  qt4_add_dbus_interface(SOURCES
      dbus/org.gnome.SettingsDaemon.MediaKeys.xml
      dbus/gnomesettingsdaemon)

  # org.freedesktop.Avahi.Server interface
  add_custom_command(
      OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahiserver.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahiserver.h
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE}
      dbus/org.freedesktop.Avahi.Server.xml
      -p ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahiserver
      -i dbus/metatypes.h
      DEPENDS dbus/org.freedesktop.Avahi.Server.xml
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  list(APPEND HEADERS ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahiserver.h)
  list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahiserver.cpp)

  # org.freedesktop.Avahi.EntryGroup interface
  add_custom_command(
      OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahientrygroup.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahientrygroup.h
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE}
      dbus/org.freedesktop.Avahi.EntryGroup.xml
      -p ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahientrygroup
      -i dbus/metatypes.h
      DEPENDS dbus/org.freedesktop.Avahi.EntryGroup.xml
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  list(APPEND HEADERS ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahientrygroup.h)
  list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dbus/avahientrygroup.cpp)

  # DeviceKit DBUS interfaces
  if(HAVE_DEVICEKIT)
    set_source_files_properties(dbus/org.freedesktop.UDisks.xml
        PROPERTIES NO_NAMESPACE dbus/udisks)
    set_source_files_properties(dbus/org.freedesktop.UDisks.Device.xml
        PROPERTIES NO_NAMESPACE dbus/udisksdevice)
    qt4_add_dbus_interface(SOURCES
        dbus/org.freedesktop.UDisks.xml
        dbus/udisks)
    qt4_add_dbus_interface(SOURCES
        dbus/org.freedesktop.UDisks.Device.xml
        dbus/udisksdevice)
  endif(HAVE_DEVICEKIT)

  if(HAVE_UDISKS2)
      set_source_files_properties(dbus/org.freedesktop.DBus.ObjectManager.xml
          PROPERTIES NO_NAMESPACE dbus/objectmanager INCLUDE dbus/metatypes.h)
      set_source_files_properties(dbus/org.freedesktop.UDisks2.Filesystem.xml
          PROPERTIES NO_NAMESPACE dbus/udisks2filesystem INCLUDE dbus/metatypes.h)
      set_source_files_properties(dbus/org.freedesktop.UDisks2.Block.xml
          PROPERTIES NO_NAMESPACE dbus/udisks2block INCLUDE dbus/metatypes.h)
      set_source_files_properties(dbus/org.freedesktop.UDisks2.Drive.xml
          PROPERTIES NO_NAMESPACE dbus/udisks2drive INCLUDE dbus/metatypes.h)
      set_source_files_properties(dbus/org.freedesktop.UDisks2.Job.xml
          PROPERTIES NO_NAMESPACE dbus/udisks2job INCLUDE dbus/metatypes.h)
      qt4_add_dbus_interface(SOURCES
          dbus/org.freedesktop.DBus.ObjectManager.xml
          dbus/objectmanager)
      qt4_add_dbus_interface(SOURCES
          dbus/org.freedesktop.UDisks2.Filesystem.xml
          dbus/udisks2filesystem)
      qt4_add_dbus_interface(SOURCES
          dbus/org.freedesktop.UDisks2.Block.xml
          dbus/udisks2block)
      qt4_add_dbus_interface(SOURCES
          dbus/org.freedesktop.UDisks2.Drive.xml
          dbus/udisks2drive)
      qt4_add_dbus_interface(SOURCES
          dbus/org.freedesktop.UDisks2.Job.xml
          dbus/udisks2job)
  endif(HAVE_UDISKS2)

  # Wiimotedev interface classes
  if(ENABLE_WIIMOTEDEV)
    qt4_add_dbus_interface(SOURCES
        dbus/org.wiimotedev.deviceEvents.xml
        dbus/wiimotedev)
  endif(ENABLE_WIIMOTEDEV)
endif(HAVE_DBUS)

optional_source(HAVE_DBUS
  SOURCES
    core/mpris.cpp
    core/mpris2.cpp
    networkremote/avahi.cpp
    ui/dbusscreensaver.cpp
  HEADERS
    core/mpris.h
    core/mpris2.h
)

optional_source(HAVE_WIIMOTEDEV
  SOURCES
    wiimotedev/shortcuts.cpp
    wiimotedev/wiimotesettingspage.cpp
    wiimotedev/wiimoteshortcutgrabber.cpp
  HEADERS
    wiimotedev/shortcuts.h
    wiimotedev/wiimotesettingspage.h
    wiimotedev/wiimoteshortcutgrabber.h
)

optional_source(HAVE_DEVICEKIT
  SOURCES devices/devicekitlister.cpp
  HEADERS devices/devicekitlister.h
)

optional_source(HAVE_UDISKS2
  SOURCES devices/udisks2lister.cpp
  HEADERS devices/udisks2lister.h
)

# Libgpod device backend
optional_source(HAVE_LIBGPOD
  INCLUDE_DIRECTORIES ${LIBGPOD_INCLUDE_DIRS}
  SOURCES
    devices/gpoddevice.cpp
    devices/gpodloader.cpp
  HEADERS
    devices/gpoddevice.h
    devices/gpodloader.h
)

# GIO device backend
optional_source(HAVE_GIO
  INCLUDE_DIRECTORIES ${GIO_INCLUDE_DIRS}
  SOURCES devices/giolister.cpp
  HEADERS devices/giolister.h
)

# CDIO backend and device
optional_source(HAVE_AUDIOCD
  SOURCES
    devices/cddadevice.cpp
    devices/cddalister.cpp
    devices/cddasongloader.cpp
    ripper/ripcddialog.cpp
    ripper/ripper.cpp
  HEADERS
    devices/cddadevice.h
    devices/cddalister.h
    devices/cddasongloader.h
    ripper/ripcddialog.h
    ripper/ripper.h
  UI
    ripper/ripcddialog.ui
)

# mtp device
optional_source(HAVE_LIBMTP
  INCLUDE_DIRECTORIES ${LIBMTP_INCLUDE_DIRS}
  SOURCES
    devices/mtpconnection.cpp
    devices/mtpdevice.cpp
    devices/mtploader.cpp
  HEADERS
    devices/mtpdevice.h
    devices/mtploader.h
)

# Moodbar support
optional_source(HAVE_MOODBAR
  SOURCES
    moodbar/moodbarbuilder.cpp
    moodbar/moodbarcontroller.cpp
    moodbar/moodbaritemdelegate.cpp
    moodbar/moodbarloader.cpp
    moodbar/moodbarpipeline.cpp
    moodbar/moodbarproxystyle.cpp
    moodbar/moodbarrenderer.cpp
  HEADERS
    moodbar/moodbarcontroller.h
    moodbar/moodbaritemdelegate.h
    moodbar/moodbarloader.h
    moodbar/moodbarpipeline.h
    moodbar/moodbarproxystyle.h
)

# Google Drive support
optional_source(HAVE_GOOGLE_DRIVE
  SOURCES
    internet/googledrive/googledriveclient.cpp
    internet/googledrive/googledriveservice.cpp
    internet/googledrive/googledrivesettingspage.cpp
    internet/googledrive/googledriveurlhandler.cpp
  HEADERS
    internet/googledrive/googledriveclient.h
    internet/googledrive/googledriveservice.h
    internet/googledrive/googledrivesettingspage.h
    internet/googledrive/googledriveurlhandler.h
  UI
    internet/googledrive/googledrivesettingspage.ui
)

# Dropbox support
optional_source(HAVE_DROPBOX
  SOURCES
    internet/dropbox/dropboxservice.cpp
    internet/dropbox/dropboxsettingspage.cpp
    internet/dropbox/dropboxurlhandler.cpp
  HEADERS
    internet/dropbox/dropboxservice.h
    internet/dropbox/dropboxsettingspage.h
    internet/dropbox/dropboxurlhandler.h
  UI
    internet/dropbox/dropboxsettingspage.ui
)

# Skydrive support
optional_source(HAVE_SKYDRIVE
  SOURCES
    internet/skydrive/skydriveservice.cpp
    internet/skydrive/skydrivesettingspage.cpp
    internet/skydrive/skydriveurlhandler.cpp
  HEADERS
    internet/skydrive/skydriveservice.h
    internet/skydrive/skydrivesettingspage.h
    internet/skydrive/skydriveurlhandler.h
  UI
    internet/skydrive/skydrivesettingspage.ui
)

# Box support
optional_source(HAVE_BOX
  SOURCES
    internet/box/boxservice.cpp
    internet/box/boxsettingspage.cpp
    internet/box/boxurlhandler.cpp
  HEADERS
    internet/box/boxservice.h
    internet/box/boxsettingspage.h
    internet/box/boxurlhandler.h
  UI
    internet/box/boxsettingspage.ui
)

# Vk.com support
optional_source(HAVE_VK
  INCLUDE_DIRECTORIES 
    ${VREEN_INCLUDE_DIRS}
  SOURCES
    globalsearch/vksearchprovider.cpp
    internet/vk/vkconnection.cpp
    internet/vk/vkmusiccache.cpp
    internet/vk/vksearchdialog.cpp
    internet/vk/vkservice.cpp
    internet/vk/vksettingspage.cpp
    internet/vk/vkurlhandler.cpp
  HEADERS
    globalsearch/vksearchprovider.h
    internet/vk/vkconnection.h
    internet/vk/vkmusiccache.h
    internet/vk/vksearchdialog.h
    internet/vk/vkservice.h
    internet/vk/vksettingspage.h
    internet/vk/vkurlhandler.h
  UI
    internet/vk/vksearchdialog.ui
    internet/vk/vksettingspage.ui
)

# Seafile support
optional_source(HAVE_SEAFILE
  SOURCES
    internet/seafile/seafileservice.cpp
    internet/seafile/seafilesettingspage.cpp
    internet/seafile/seafileurlhandler.cpp
    internet/seafile/seafiletree.cpp
  HEADERS
    internet/seafile/seafileservice.h
    internet/seafile/seafilesettingspage.h
    internet/seafile/seafileurlhandler.h
    internet/seafile/seafiletree.h
  UI
    internet/seafile/seafilesettingspage.ui
)

# Pulse audio integration
optional_source(HAVE_LIBPULSE
  INCLUDE_DIRECTORIES
    ${LIBPULSE_INCLUDE_DIRS}
  SOURCES
    engines/pulsedevicefinder.cpp
)

# Hack to add Clementine to the Unity system tray whitelist
optional_source(LINUX
  SOURCES core/ubuntuunityhack.cpp
  HEADERS core/ubuntuunityhack.h
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/version.h)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_wrap_ui(UIC ${UI})
qt4_add_resources(QRC ${RESOURCES})

add_pot(POT
  ${CMAKE_CURRENT_SOURCE_DIR}/translations/header
  ${CMAKE_CURRENT_SOURCE_DIR}/translations/translations.pot
  ${SOURCES} ${MOC} ${UIC} ${OTHER_SOURCES}
  ../data/oauthsuccess.html
)
add_po(PO clementine_
  LANGUAGES ${LANGUAGES}
  DIRECTORY translations
)

add_library(clementine_lib STATIC
  ${SOURCES}
  ${MOC}
  ${UIC}
  ${QRC}
  ${POT}
  ${PO}
  ${OTHER_UIC_SOURCES}
)

add_dependencies(clementine_lib pot)


target_link_libraries(clementine_lib
  clementine-spotifyblob-messages
  libclementine-common
  libclementine-tagreader
  libclementine-remote
  ${SHA2_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  ${MYGPOQT_LIBRARIES}
  ${CHROMAPRINT_LIBRARIES}
  ${GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GIO_LIBRARIES}
  ${QJSON_LIBRARIES}
  ${QT_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_APP_LIBRARIES}
  ${GSTREAMER_TAG_LIBRARIES}
  ${GSTREAMER_PBUTILS_LIBRARIES}
  ${QTSINGLEAPPLICATION_LIBRARIES}
  ${QTSINGLECOREAPPLICATION_LIBRARIES}
  ${QTIOCOMPRESSOR_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${SQLITE_LIBRARIES}
  z
  Qocoa
)

if(HAVE_VK)
  target_link_libraries(clementine_lib ${VREEN_LIBRARIES})
endif(HAVE_VK)

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

if(HAVE_AUDIOCD)
  target_link_libraries(clementine_lib ${CDIO_LIBRARIES})
endif(HAVE_AUDIOCD)

if(HAVE_MOODBAR)
  target_link_libraries(clementine_lib gstmoodbar)
endif()

if(HAVE_LIBMTP)
  target_link_libraries(clementine_lib ${LIBMTP_LIBRARIES})
endif(HAVE_LIBMTP)

if(HAVE_BREAKPAD)
  if (LINUX)
    target_link_libraries(clementine_lib breakpad)
  elseif (APPLE)
    add_dependencies(clementine_lib breakpad)
    target_link_libraries(clementine_lib
        "-F${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/google-breakpad/client/mac/build/Release"
        "-framework Breakpad")
  endif (LINUX)
endif(HAVE_BREAKPAD)

if(HAVE_SPOTIFY_DOWNLOADER)
  target_link_libraries(clementine_lib
    ${CRYPTOPP_LIBRARIES}
  )
  link_directories(${CRYPTOPP_LIBRARY_DIRS})
endif(HAVE_SPOTIFY_DOWNLOADER)

if(HAVE_LIBPULSE)
  target_link_libraries(clementine_lib ${LIBPULSE_LIBRARIES})
endif()

if (APPLE)
  target_link_libraries(clementine_lib
    "-framework AppKit"
    "-framework Carbon"
    "-framework CoreAudio"
    "-framework DiskArbitration"
    "-framework Foundation"
    "-framework IOKit"
    "-framework ScriptingBridge"
  )
  target_link_libraries(clementine_lib ${SPMEDIAKEYTAP_LIBRARIES})
  if (HAVE_SPARKLE)
    include_directories(${SPARKLE}/Headers)
    target_link_libraries(clementine_lib ${SPARKLE})
  endif (HAVE_SPARKLE)
else (APPLE)
  target_link_libraries(clementine_lib ${QXT_LIBRARIES})
endif (APPLE)

set(3RDPARTY_SQLITE_LIBRARY qsqlite)
target_link_libraries(clementine_lib qsqlite)

if (WIN32)
  target_link_libraries(clementine_lib
    ${ZLIB_LIBRARIES}
    ${QTSPARKLE_LIBRARIES}
    tinysvcmdns
    qtwin
    dsound
    ${QT_QTGUI_LIBRARY}
  )
endif (WIN32)

if (UNIX AND NOT APPLE)
  # Hack: the Gold linker pays attention to the order that libraries are
  # specified on the link line.  -lX11 and -ldl are provided earlier in the link
  # command but they're actually used by libraries that appear after them, so
  # they end up getting ignored.  This appends them to the very end of the link
  # line, ensuring they're always used.
  find_package(X11)
  if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    target_link_libraries(clementine_lib ${X11_X11_LIB})
  else ()
    target_link_libraries(clementine_lib ${X11_X11_LIB} ${CMAKE_DL_LIBS})
  endif ()
endif ()

add_dependencies(clementine_lib qtsingleapplication)


###############################################################################

set(EXECUTABLE_OUTPUT_PATH ..)

# Show the console window in debug mode on Windows
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_WIN32_CONSOLE)
  set(CLEMENTINE-WIN32-FLAG WIN32)
endif (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_WIN32_CONSOLE)

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

if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  target_link_libraries(clementine execinfo)
endif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

target_link_libraries(clementine
  clementine_lib
)

# macdeploy.py relies on the blob being built first.
if(HAVE_SPOTIFY_BLOB)
  add_dependencies(clementine clementine-spotifyblob)
endif(HAVE_SPOTIFY_BLOB)
add_dependencies(clementine clementine-tagreader)

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
  install(FILES ../dist/cacert.pem
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources")

  install(DIRECTORY "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources/"
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources")

  if (HAVE_BREAKPAD)
    install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/google-breakpad/client/mac/build/Release/Breakpad.framework
        DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Frameworks"
        PATTERN "*.h" EXCLUDE
        PATTERN "Inspector" PERMISSIONS OWNER_EXECUTE OWNER_READ
                                        GROUP_EXECUTE GROUP_READ
                                        WORLD_READ WORLD_EXECUTE
        PATTERN "crash_report_sender" PERMISSIONS OWNER_EXECUTE OWNER_READ
                                                  GROUP_EXECUTE GROUP_READ
                                                  WORLD_READ WORLD_EXECUTE)
  endif (HAVE_BREAKPAD)

  add_custom_command(TARGET clementine
    POST_BUILD
    COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/../dist/macdeploy.py ${PROJECT_BINARY_DIR}/clementine.app -f
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )

  add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/clementine.breakpad
    COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/google-breakpad/tools/mac/dump_syms/build/Release/dump_syms
    -a i386 ${PROJECT_BINARY_DIR}/clementine.app/Contents/MacOS/clementine
    > ${PROJECT_BINARY_DIR}/clementine.breakpad
    DEPENDS clementine)
  add_custom_target(clementine-breakpad
    DEPENDS ${PROJECT_BINARY_DIR}/clementine.breakpad)

  if (APPLE_DEVELOPER_ID)
    add_custom_target(
      sign
      COMMAND
      ${PROJECT_SOURCE_DIR}/dist/codesign.py ${APPLE_DEVELOPER_ID} ${PROJECT_BINARY_DIR}/clementine.app
      DEPENDS clementine
      VERBATIM
    )
  endif()


  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_SPARKLE}.dmg
    COMMAND ${CMAKE_COMMAND} -E remove -f ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_SPARKLE}.dmg
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../dist/create-dmg.sh ${PROJECT_BINARY_DIR}/clementine.app
    COMMAND ${CMAKE_COMMAND} -E rename
      ${PROJECT_BINARY_DIR}/clementine.dmg
      ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_SPARKLE}.dmg
    DEPENDS clementine
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
  add_custom_target(dmg
    DEPENDS ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_SPARKLE}.dmg)
else (APPLE)
  install(TARGETS clementine
    RUNTIME DESTINATION bin
  )
endif (APPLE)
