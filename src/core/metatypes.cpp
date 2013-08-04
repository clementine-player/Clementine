#include "metatypes.h"

#include <QMetaType>
#include <QNetworkCookie>

#include "config.h"
#include "covers/albumcoverfetcher.h"
#include "engines/enginebase.h"
#include "globalsearch/searchprovider.h"
#include "internet/digitallyimportedclient.h"
#include "internet/geolocator.h"
#include "internet/somafmservice.h"
#include "library/directory.h"
#include "playlist/playlist.h"
#include "podcasts/podcastepisode.h"
#include "podcasts/podcast.h"
#include "ui/equalizer.h"

#ifdef HAVE_DBUS
#include <QDBusMetaType>
#include "core/mpris2.h"
#include "dbus/metatypes.h"
#endif

class GstBuffer;
class GstElement;
class GstEnginePipeline;
class QNetworkReply;

void RegisterMetaTypes() {
  qRegisterMetaType<ColumnAlignmentMap>("ColumnAlignmentMap");
  qRegisterMetaType<const char*>("const char*");
  qRegisterMetaType<CoverSearchResult>("CoverSearchResult");
  qRegisterMetaType<CoverSearchResults>("CoverSearchResults");
  qRegisterMetaType<DigitallyImportedClient::Channel>("DigitallyImportedClient::Channel");
  qRegisterMetaType<Directory>("Directory");
  qRegisterMetaType<DirectoryList>("DirectoryList");
  qRegisterMetaType<Engine::SimpleMetaBundle>("Engine::SimpleMetaBundle");
  qRegisterMetaType<Engine::State>("Engine::State");
  qRegisterMetaType<Engine::TrackChangeFlags>("Engine::TrackChangeFlags");
  qRegisterMetaType<Equalizer::Params>("Equalizer::Params");
  qRegisterMetaType<Geolocator::LatLng>("Geolocator::LatLng");
  qRegisterMetaType<GstBuffer*>("GstBuffer*");
  qRegisterMetaType<GstElement*>("GstElement*");
  qRegisterMetaType<GstEnginePipeline*>("GstEnginePipeline*");
  qRegisterMetaType<PlaylistItemList>("PlaylistItemList");
  qRegisterMetaType<PlaylistItemPtr>("PlaylistItemPtr");
  qRegisterMetaType<PodcastEpisodeList>("PodcastEpisodeList");
  qRegisterMetaType<PodcastList>("PodcastList");
  qRegisterMetaType<QList<CoverSearchResult> >("QList<CoverSearchResult>");
  qRegisterMetaType<QList<PlaylistItemPtr> >("QList<PlaylistItemPtr>");
  qRegisterMetaType<PlaylistSequence::RepeatMode>("PlaylistSequence::RepeatMode");
  qRegisterMetaType<PlaylistSequence::ShuffleMode>("PlaylistSequence::ShuffleMode");
  qRegisterMetaType<QList<PodcastEpisode> >("QList<PodcastEpisode>");
  qRegisterMetaType<QList<Podcast> >("QList<Podcast>");
  qRegisterMetaType<QList<QNetworkCookie> >("QList<QNetworkCookie>");
  qRegisterMetaType<QList<Song> >("QList<Song>");
  qRegisterMetaType<QNetworkCookie>("QNetworkCookie");
  qRegisterMetaType<QNetworkReply*>("QNetworkReply*");
  qRegisterMetaType<QNetworkReply**>("QNetworkReply**");
  qRegisterMetaType<SearchProvider::ResultList>("SearchProvider::ResultList");
  qRegisterMetaType<SearchProvider::Result>("SearchProvider::Result");
  qRegisterMetaType<smart_playlists::GeneratorPtr>("smart_playlists::GeneratorPtr");
  qRegisterMetaType<SomaFMService::Stream>("SomaFMService::Stream");
  qRegisterMetaType<SongList>("SongList");
  qRegisterMetaType<Song>("Song");
  qRegisterMetaTypeStreamOperators<DigitallyImportedClient::Channel>("DigitallyImportedClient::Channel");
  qRegisterMetaTypeStreamOperators<Equalizer::Params>("Equalizer::Params");
  qRegisterMetaTypeStreamOperators<QMap<int, int> >("ColumnAlignmentMap");
  qRegisterMetaTypeStreamOperators<SomaFMService::Stream>("SomaFMService::Stream");
  qRegisterMetaType<SubdirectoryList>("SubdirectoryList");
  qRegisterMetaType<Subdirectory>("Subdirectory");
  qRegisterMetaType<QList<QUrl> >("QList<QUrl>");

#ifdef HAVE_DBUS
  qDBusRegisterMetaType<QImage>();
  qDBusRegisterMetaType<TrackMetadata>();
  qDBusRegisterMetaType<TrackIds>();
  qDBusRegisterMetaType<QList<QByteArray> >();
  qDBusRegisterMetaType<MprisPlaylist>();
  qDBusRegisterMetaType<MaybePlaylist>();
  qDBusRegisterMetaType<MprisPlaylistList>();
#endif
}
