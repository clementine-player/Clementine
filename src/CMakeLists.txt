
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

include_directories(${CMAKE_BINARY_DIR})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${LIBXML_INCLUDE_DIRS})
include_directories(${GOBJECT_INCLUDE_DIRS})
include_directories(${LIBPROJECTM_INCLUDE_DIRS})
include_directories(${QTSINGLEAPPLICATION_INCLUDE_DIRS})
include_directories(${QTIOCOMPRESSOR_INCLUDE_DIRS})
include_directories(${QXT_INCLUDE_DIRS})
include_directories(${ECHONEST_INCLUDE_DIRS})
include_directories(${SHA2_INCLUDE_DIRS})
include_directories(${CHROMAPRINT_INCLUDE_DIRS})
include_directories(${MYGPOQT_INCLUDE_DIRS})

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIR})

if(HAVE_LIBINDICATE)
  link_directories(${INDICATEQT_LIBRARY_DIRS})
  include_directories(${INDICATEQT_INCLUDE_DIRS})
endif(HAVE_LIBINDICATE)

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
  analyzers/nyancatanalyzer.cpp
  analyzers/sonogram.cpp
  analyzers/turbine.cpp

  core/appearance.cpp
  core/application.cpp
  core/backgroundstreams.cpp
  core/commandlineoptions.cpp
  core/crashreporting.cpp
  core/database.cpp
  core/deletefiles.cpp
  core/filesystemmusicstorage.cpp
  core/filesystemwatcherinterface.cpp
  core/fht.cpp
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
  covers/discogscoverprovider.cpp
  covers/kittenloader.cpp
  covers/musicbrainzcoverprovider.cpp

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

  globalsearch/digitallyimportedsearchprovider.cpp
  globalsearch/globalsearch.cpp
  globalsearch/globalsearchitemdelegate.cpp
  globalsearch/globalsearchmodel.cpp
  globalsearch/globalsearchsettingspage.cpp
  globalsearch/globalsearchsortmodel.cpp
  globalsearch/globalsearchview.cpp
  globalsearch/groovesharksearchprovider.cpp
  globalsearch/icecastsearchprovider.cpp
  globalsearch/librarysearchprovider.cpp
  globalsearch/savedradiosearchprovider.cpp
  globalsearch/searchprovider.cpp
  globalsearch/searchproviderstatuswidget.cpp
  globalsearch/simplesearchprovider.cpp
  globalsearch/somafmsearchprovider.cpp
  globalsearch/soundcloudsearchprovider.cpp
  globalsearch/spotifysearchprovider.cpp
  globalsearch/suggestionwidget.cpp
  globalsearch/urlsearchprovider.cpp

  internet/cloudfileservice.cpp
  internet/digitallyimportedclient.cpp
  internet/digitallyimportedservicebase.cpp
  internet/digitallyimportedsettingspage.cpp
  internet/digitallyimportedurlhandler.cpp
  internet/geolocator.cpp
  internet/groovesharkradio.cpp
  internet/groovesharkservice.cpp
  internet/groovesharksettingspage.cpp
  internet/groovesharkurlhandler.cpp
  internet/icecastbackend.cpp
  internet/icecastfilterwidget.cpp
  internet/icecastmodel.cpp
  internet/icecastservice.cpp
  internet/internetmodel.cpp
  internet/internetplaylistitem.cpp
  internet/internetservice.cpp
  internet/internetview.cpp
  internet/internetviewcontainer.cpp
  internet/jamendodynamicplaylist.cpp
  internet/jamendoplaylistitem.cpp
  internet/jamendoservice.cpp
  internet/localredirectserver.cpp
  internet/magnatunedownloaddialog.cpp
  internet/magnatuneplaylistitem.cpp
  internet/magnatuneservice.cpp
  internet/magnatunesettingspage.cpp
  internet/magnatuneurlhandler.cpp
  internet/oauthenticator.cpp
  internet/savedradio.cpp
  internet/searchboxwidget.cpp
  internet/somafmservice.cpp
  internet/somafmurlhandler.cpp
  internet/soundcloudservice.cpp
  internet/spotifyserver.cpp
  internet/spotifyservice.cpp
  internet/spotifysettingspage.cpp
  internet/subsonicservice.cpp
  internet/subsonicsettingspage.cpp
  internet/subsonicurlhandler.cpp

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

  podcasts/addpodcastbyurl.cpp
  podcasts/addpodcastdialog.cpp
  podcasts/addpodcastpage.cpp
  podcasts/fixedopmlpage.cpp
  podcasts/gpoddersearchpage.cpp
  podcasts/gpoddersync.cpp
  podcasts/gpoddertoptagsmodel.cpp
  podcasts/gpoddertoptagspage.cpp
  podcasts/itunessearchpage.cpp
  podcasts/podcast.cpp
  podcasts/podcastbackend.cpp
  podcasts/podcastdiscoverymodel.cpp
  podcasts/podcastdownloader.cpp
  podcasts/podcastepisode.cpp
  podcasts/podcastinfowidget.cpp
  podcasts/podcastservice.cpp
  podcasts/podcastservicemodel.cpp
  podcasts/podcastsettingspage.cpp
  podcasts/podcastparser.cpp
  podcasts/podcastupdater.cpp
  podcasts/podcasturlloader.cpp

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
  songinfo/songinfobase.cpp
  songinfo/songinfofetcher.cpp
  songinfo/songinfoprovider.cpp
  songinfo/songinfosettingspage.cpp
  songinfo/songinfotextview.cpp
  songinfo/songinfoview.cpp
  songinfo/songkickconcerts.cpp
  songinfo/songkickconcertwidget.cpp
  songinfo/songplaystats.cpp
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
  analyzers/nyancatanalyzer.h
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
  covers/discogscoverprovider.h
  covers/kittenloader.h
  covers/musicbrainzcoverprovider.h

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

  globalsearch/globalsearch.h
  globalsearch/globalsearchmodel.h
  globalsearch/globalsearchsettingspage.h
  globalsearch/globalsearchview.h
  globalsearch/groovesharksearchprovider.h
  globalsearch/searchprovider.h
  globalsearch/simplesearchprovider.h
  globalsearch/soundcloudsearchprovider.h
  globalsearch/spotifysearchprovider.h
  globalsearch/suggestionwidget.h

  internet/cloudfileservice.h
  internet/digitallyimportedclient.h
  internet/digitallyimportedservicebase.h
  internet/digitallyimportedsettingspage.h
  internet/geolocator.h
  internet/groovesharkservice.h
  internet/groovesharksettingspage.h
  internet/groovesharkurlhandler.h
  internet/icecastbackend.h
  internet/icecastfilterwidget.h
  internet/icecastmodel.h
  internet/icecastservice.h
  internet/internetmimedata.h
  internet/internetmodel.h
  internet/internetservice.h
  internet/internetsongmimedata.h
  internet/internetview.h
  internet/internetviewcontainer.h
  internet/jamendodynamicplaylist.h
  internet/jamendoservice.h
  internet/localredirectserver.h
  internet/magnatunedownloaddialog.h
  internet/magnatuneservice.h
  internet/magnatunesettingspage.h
  internet/oauthenticator.h
  internet/savedradio.h
  internet/searchboxwidget.h
  internet/somafmservice.h
  internet/somafmurlhandler.h
  internet/soundcloudservice.h
  internet/spotifyserver.h
  internet/spotifyservice.h
  internet/spotifysettingspage.h
  internet/subsonicservice.h
  internet/subsonicsettingspage.h
  internet/subsonicurlhandler.h

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

  musicbrainz/acoustidclient.h
  musicbrainz/musicbrainzclient.h
  musicbrainz/tagfetcher.h
  
  networkremote/networkremotehelper.h
  networkremote/networkremote.h
  networkremote/incomingdataparser.h
  networkremote/outgoingdatacreator.h
  networkremote/remoteclient.h

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

  podcasts/addpodcastbyurl.h
  podcasts/addpodcastdialog.h
  podcasts/addpodcastpage.h
  podcasts/fixedopmlpage.h
  podcasts/gpoddersearchpage.h
  podcasts/gpoddersync.h
  podcasts/gpoddertoptagsmodel.h
  podcasts/gpoddertoptagspage.h
  podcasts/itunessearchpage.h
  podcasts/podcastbackend.h
  podcasts/podcastdiscoverymodel.h
  podcasts/podcastdownloader.h
  podcasts/podcastinfowidget.h
  podcasts/podcastservice.h
  podcasts/podcastservicemodel.h
  podcasts/podcastsettingspage.h
  podcasts/podcastupdater.h
  podcasts/podcasturlloader.h

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
  songinfo/songinfobase.h
  songinfo/songinfofetcher.h
  songinfo/songinfoprovider.h
  songinfo/songinfosettingspage.h
  songinfo/songinfotextview.h
  songinfo/songinfoview.h
  songinfo/songkickconcerts.h
  songinfo/songkickconcertwidget.h
  songinfo/songplaystats.h
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

  globalsearch/globalsearchsettingspage.ui
  globalsearch/globalsearchview.ui
  globalsearch/searchproviderstatuswidget.ui
  globalsearch/suggestionwidget.ui

  internet/digitallyimportedsettingspage.ui
  internet/groovesharksettingspage.ui
  internet/icecastfilterwidget.ui
  internet/internetviewcontainer.ui
  internet/magnatunedownloaddialog.ui
  internet/magnatunesettingspage.ui
  internet/searchboxwidget.ui
  internet/spotifysettingspage.ui
  internet/subsonicsettingspage.ui

  library/groupbydialog.ui
  library/libraryfilterwidget.ui
  library/librarysettingspage.ui
  library/libraryviewcontainer.ui

  playlist/dynamicplaylistcontrols.ui
  playlist/playlistcontainer.ui
  playlist/playlistlistcontainer.ui
  playlist/playlistsequence.ui
  playlist/queuemanager.ui

  podcasts/addpodcastbyurl.ui
  podcasts/addpodcastdialog.ui
  podcasts/gpoddersearchpage.ui
  podcasts/itunessearchpage.ui
  podcasts/podcastinfowidget.ui
  podcasts/podcastsettingspage.ui

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
    globalsearch/lastfmsearchprovider.cpp
    internet/fixlastfm.cpp
    internet/lastfmcompat.cpp
    internet/lastfmservice.cpp
    internet/lastfmsettingspage.cpp
    internet/lastfmstationdialog.cpp
    internet/lastfmurlhandler.cpp
    songinfo/echonestsimilarartists.cpp
    songinfo/echonesttags.cpp
    songinfo/lastfmtrackinfoprovider.cpp
    songinfo/tagwidget.cpp
  HEADERS
    covers/lastfmcoverprovider.h
    internet/lastfmservice.h
    internet/lastfmsettingspage.h
    internet/lastfmstationdialog.h
    songinfo/echonestsimilarartists.h
    songinfo/echonesttags.h
    songinfo/lastfmtrackinfoprovider.h
    songinfo/tagwidget.h
  UI
    internet/lastfmsettingspage.ui
    internet/lastfmstationdialog.ui
)


optional_source(HAVE_SPOTIFY_DOWNLOADER
  SOURCES
    internet/spotifyblobdownloader.cpp
  HEADERS
    internet/spotifyblobdownloader.h
  INCLUDE_DIRECTORIES
    ${QCA_INCLUDE_DIRS}
)

# Platform specific - OS X
optional_source(APPLE
  INCLUDE_DIRECTORIES
    ${GROWL}/Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/google-breakpad/client/mac/build/Release/Breakpad.framework
  SOURCES
    core/macfslistener.mm
    core/macglobalshortcutbackend.mm
    core/mac_startup.mm
    core/scoped_nsautorelease_pool.mm
    devices/macdevicelister.mm
    networkremote/bonjour.mm
    ui/globalshortcutgrabber.mm
    ui/macscreensaver.cpp
    ui/macsystemtrayicon.mm
    widgets/osd_mac.mm
  HEADERS
    core/macfslistener.h
    core/macglobalshortcutbackend.h
    devices/macdevicelister.h
    ui/macsystemtrayicon.h
)

# Platform specific - Windows
optional_source(WIN32
  SOURCES
    networkremote/tinysvcmdns.cpp
    widgets/osd_win.cpp
  INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/3rdparty/tinysvcmdns
)

# Platform specific - X11
optional_source(LINUX SOURCES widgets/osd_x11.cpp)

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
    qt4_add_dbus_interface(SOURCES
        dbus/org.freedesktop.UDisks.xml
        dbus/udisks)
    qt4_add_dbus_interface(SOURCES
        dbus/org.freedesktop.UDisks.Device.xml
        dbus/udisksdevice)
  endif(HAVE_DEVICEKIT)

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
    core/mpris1.cpp
    core/mpris2.cpp
    networkremote/avahi.cpp
    ui/dbusscreensaver.cpp
  HEADERS
    core/mpris.h
    core/mpris1.h
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
  HEADERS
    devices/cddadevice.h
    devices/cddalister.h
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
    internet/googledriveclient.cpp
    internet/googledriveservice.cpp
    internet/googledrivesettingspage.cpp
    internet/googledriveurlhandler.cpp
  HEADERS
    internet/googledriveclient.h
    internet/googledriveservice.h
    internet/googledrivesettingspage.h
    internet/googledriveurlhandler.h
  UI
    internet/googledrivesettingspage.ui
)

# Ubuntu One file support
optional_source(HAVE_UBUNTU_ONE
  SOURCES
    internet/ubuntuoneauthenticator.cpp
    internet/ubuntuoneservice.cpp
    internet/ubuntuonesettingspage.cpp
    internet/ubuntuoneurlhandler.cpp
  HEADERS
    internet/ubuntuoneauthenticator.h
    internet/ubuntuoneservice.h
    internet/ubuntuonesettingspage.h
    internet/ubuntuoneurlhandler.h
  UI
    internet/ubuntuonesettingspage.ui
)

# Dropbox support
optional_source(HAVE_DROPBOX
  SOURCES
    internet/dropboxauthenticator.cpp
    internet/dropboxservice.cpp
    internet/dropboxsettingspage.cpp
    internet/dropboxurlhandler.cpp
  HEADERS
    internet/dropboxauthenticator.h
    internet/dropboxservice.h
    internet/dropboxsettingspage.h
    internet/dropboxurlhandler.h
  UI
    internet/dropboxsettingspage.ui
)

# Skydrive support
optional_source(HAVE_SKYDRIVE
  SOURCES
    internet/skydriveservice.cpp
    internet/skydriveurlhandler.cpp
  HEADERS
    internet/skydriveservice.h
    internet/skydriveurlhandler.h
)

# Box support
optional_source(HAVE_BOX
  SOURCES
    internet/boxservice.cpp
    internet/boxsettingspage.cpp
    internet/boxurlhandler.cpp
  HEADERS
    internet/boxservice.h
    internet/boxsettingspage.h
    internet/boxurlhandler.h
  UI
    internet/boxsettingspage.ui
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
  ${ECHONEST_LIBRARIES}
  ${GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${QJSON_LIBRARIES}
  ${QT_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_APP_LIBRARIES}
  ${GSTREAMER_TAG_LIBRARIES}
  ${QTSINGLEAPPLICATION_LIBRARIES}
  ${QTSINGLECOREAPPLICATION_LIBRARIES}
  ${QTIOCOMPRESSOR_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${SQLITE_LIBRARIES}
  z
  Qocoa
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

if(HAVE_AUDIOCD)
  target_link_libraries(clementine_lib ${CDIO_LIBRARIES})
  target_link_libraries(clementine_lib ${GSTREAMER_CDDA_LIBRARIES})
endif(HAVE_AUDIOCD)

if(HAVE_MOODBAR)
  target_link_libraries(clementine_lib gstmoodbar)
endif()

if(HAVE_LIBMTP)
  target_link_libraries(clementine_lib ${LIBMTP_LIBRARIES})
endif(HAVE_LIBMTP)

if(HAVE_LIBINDICATE)
  target_link_libraries(clementine_lib ${INDICATEQT_LIBRARIES})
endif(HAVE_LIBINDICATE)

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
    ${QCA_LIBRARIES}
  )
  link_directories(${QCA_LIBRARY_DIRS})
endif(HAVE_SPOTIFY_DOWNLOADER)

if (APPLE)
  target_link_libraries(clementine_lib
    ${GROWL}
    /System/Library/Frameworks/AppKit.framework
    /System/Library/Frameworks/Carbon.framework
    /System/Library/Frameworks/DiskArbitration.framework
    /System/Library/Frameworks/Foundation.framework
    /System/Library/Frameworks/IOKit.framework
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

  install(DIRECTORY "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources/"
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Resources")

  if (HAVE_SPARKLE)
    install(DIRECTORY "${SPARKLE}/Versions/Current/Resources"
      DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Frameworks/Sparkle.framework")
  endif (HAVE_SPARKLE)

  install(DIRECTORY "${GROWL}/Versions/Current/Resources"
    DESTINATION "${CMAKE_BINARY_DIR}/clementine.app/Contents/Frameworks/Growl.framework")

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
      codesign -s ${APPLE_DEVELOPER_ID} -fv ${PROJECT_BINARY_DIR}/clementine.app
      DEPENDS clementine
      VERBATIM
    )
  endif()


  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_SPARKLE}.dmg
    ${CMAKE_COMMAND} -E remove -f ${PROJECT_BINARY_DIR}/clementine-${CLEMENTINE_VERSION_SPARKLE}.dmg
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
