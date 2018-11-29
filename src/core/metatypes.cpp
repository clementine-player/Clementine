/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Andreas <asfa194@gmail.com>
   Copyright 2013, pie.or.paj <pie.or.paj@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>
   Copyright 2014, Maltsev Vlad <shedwardx@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "metatypes.h"

#include <QFileInfo>
#include <QMetaType>
#include <QNetworkCookie>

#include "config.h"
#include "covers/albumcoverfetcher.h"
#include "engines/enginebase.h"
#include "engines/gstengine.h"
#include "globalsearch/searchprovider.h"
#include "internet/core/geolocator.h"
#include "internet/digitally/digitallyimportedclient.h"
#include "internet/intergalacticfm/intergalacticfmservice.h"
#include "internet/podcasts/podcast.h"
#include "internet/podcasts/podcastepisode.h"
#include "internet/somafm/somafmservice.h"
#include "library/directory.h"
#include "playlist/playlist.h"
#include "songinfo/collapsibleinfopane.h"
#include "ui/equalizer.h"

#ifdef HAVE_DBUS
#include <QDBusMetaType>
#include "core/mpris2.h"
#include "dbus/metatypes.h"
#endif

class GstEnginePipeline;
class QNetworkReply;

void RegisterMetaTypes() {
  qRegisterMetaType<CollapsibleInfoPane::Data>("CollapsibleInfoPane::Data");
  qRegisterMetaType<ColumnAlignmentMap>("ColumnAlignmentMap");
  qRegisterMetaType<const char*>("const char*");
  qRegisterMetaType<CoverSearchResult>("CoverSearchResult");
  qRegisterMetaType<CoverSearchResults>("CoverSearchResults");
  qRegisterMetaType<DigitallyImportedClient::Channel>(
      "DigitallyImportedClient::Channel");
  qRegisterMetaType<Directory>("Directory");
  qRegisterMetaType<DirectoryList>("DirectoryList");
  qRegisterMetaType<Engine::SimpleMetaBundle>("Engine::SimpleMetaBundle");
  qRegisterMetaType<Engine::State>("Engine::State");
  qRegisterMetaType<Engine::TrackChangeFlags>("Engine::TrackChangeFlags");
  qRegisterMetaType<Equalizer::Params>("Equalizer::Params");
  qRegisterMetaType<Geolocator::LatLng>("Geolocator::LatLng");
  qRegisterMetaType<GstBuffer*>("GstBuffer*");
  qRegisterMetaType<GstElement*>("GstElement*");
  qRegisterMetaType<GstEngine::OutputDetails>("GstEngine::OutputDetails");
  qRegisterMetaType<GstEnginePipeline*>("GstEnginePipeline*");
  qRegisterMetaType<PlaylistItemList>("PlaylistItemList");
  qRegisterMetaType<PlaylistItemPtr>("PlaylistItemPtr");
  qRegisterMetaType<PodcastEpisodeList>("PodcastEpisodeList");
  qRegisterMetaType<PodcastList>("PodcastList");
  qRegisterMetaType<QList<CoverSearchResult>>("QList<CoverSearchResult>");
  qRegisterMetaType<QList<int>>("QList<int>");
  qRegisterMetaType<QList<PlaylistItemPtr>>("QList<PlaylistItemPtr>");
  qRegisterMetaType<PlaylistSequence::RepeatMode>(
      "PlaylistSequence::RepeatMode");
  qRegisterMetaType<PlaylistSequence::ShuffleMode>(
      "PlaylistSequence::ShuffleMode");
  qRegisterMetaType<QAbstractSocket::SocketState>(
      "QAbstractSocket::SocketState");
  qRegisterMetaType<QList<PodcastEpisode>>("QList<PodcastEpisode>");
  qRegisterMetaType<QList<Podcast>>("QList<Podcast>");
  qRegisterMetaType<QList<QNetworkCookie>>("QList<QNetworkCookie>");
  qRegisterMetaType<QList<Song>>("QList<Song>");
  qRegisterMetaType<QNetworkCookie>("QNetworkCookie");
  qRegisterMetaType<QNetworkReply*>("QNetworkReply*");
  qRegisterMetaType<QNetworkReply**>("QNetworkReply**");
  qRegisterMetaType<SearchProvider::ResultList>("SearchProvider::ResultList");
  qRegisterMetaType<SearchProvider::Result>("SearchProvider::Result");
  qRegisterMetaType<smart_playlists::GeneratorPtr>(
      "smart_playlists::GeneratorPtr");
  qRegisterMetaType<SomaFMService::Stream>("SomaFMService::Stream");
  qRegisterMetaType<IntergalacticFMService::Stream>(
      "IntergalacticFMService::Stream");
  qRegisterMetaType<SongList>("SongList");
  qRegisterMetaType<Song>("Song");
  qRegisterMetaTypeStreamOperators<DigitallyImportedClient::Channel>(
      "DigitallyImportedClient::Channel");
  qRegisterMetaTypeStreamOperators<Equalizer::Params>("Equalizer::Params");
  qRegisterMetaTypeStreamOperators<QMap<int, int>>("ColumnAlignmentMap");
  qRegisterMetaTypeStreamOperators<SomaFMService::Stream>(
      "SomaFMService::Stream");
  qRegisterMetaTypeStreamOperators<IntergalacticFMService::Stream>(
      "IntergalacticFMService::Stream");
  qRegisterMetaType<SubdirectoryList>("SubdirectoryList");
  qRegisterMetaType<Subdirectory>("Subdirectory");
  qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
  qRegisterMetaType<QFileInfo>("QFileInfo");

#ifdef HAVE_DBUS
  qDBusRegisterMetaType<TrackMetadata>();
  qDBusRegisterMetaType<TrackIds>();
  qDBusRegisterMetaType<QList<QByteArray>>();
  qDBusRegisterMetaType<MprisPlaylist>();
  qDBusRegisterMetaType<MaybePlaylist>();
  qDBusRegisterMetaType<MprisPlaylistList>();

  qDBusRegisterMetaType<InterfacesAndProperties>();
  qDBusRegisterMetaType<ManagedObjectList>();
#ifdef HAVE_X11
  qDBusRegisterMetaType<QImage>();
#endif
#endif
}
