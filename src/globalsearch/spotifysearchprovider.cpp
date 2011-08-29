/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "spotifysearchprovider.h"
#include "core/logging.h"
#include "internet/internetmodel.h"
#include "internet/spotifyserver.h"
#include "internet/spotifyservice.h"
#include "playlist/songmimedata.h"
#include "spotifyblob/common/spotifymessagehandler.h"

SpotifySearchProvider::SpotifySearchProvider(QObject* parent)
  : SearchProvider(parent),
    server_(NULL),
    service_(NULL)
{
  Init("Spotify", QIcon(":icons/svg/spotify.svg"), true, true);
}

SpotifyServer* SpotifySearchProvider::server() {
  if (server_)
    return server_;

  if (!service_)
    service_ = InternetModel::Service<SpotifyService>();

  if (service_->login_state() != SpotifyService::LoginState_LoggedIn)
    return NULL;

  server_ = service_->server();
  connect(server_, SIGNAL(SearchResults(spotify_pb::SearchResponse)),
          SLOT(SearchFinishedSlot(spotify_pb::SearchResponse)));
  connect(server_, SIGNAL(ImageLoaded(QString,QImage)),
          SLOT(ArtLoadedSlot(QString,QImage)));
  connect(server_, SIGNAL(AlbumBrowseResults(spotify_pb::BrowseAlbumResponse)),
          SLOT(AlbumBrowseResponse(spotify_pb::BrowseAlbumResponse)));
  connect(server_, SIGNAL(destroyed()), SLOT(ServerDestroyed()));

  return service_->server();
}

void SpotifySearchProvider::ServerDestroyed() {
  server_ = NULL;
}

void SpotifySearchProvider::SearchAsync(int id, const QString& query) {
  SpotifyServer* s = server();
  if (!s) {
    emit SearchFinished(id);
    return;
  }

  PendingState state;
  state.orig_id_ = id;
  state.tokens_ = TokenizeQuery(query);

  const QString query_string = state.tokens_.join(" ");
  s->Search(query_string, 5, 5);
  queries_[query_string] = state;
}

void SpotifySearchProvider::SearchFinishedSlot(const spotify_pb::SearchResponse& response) {
  QString query_string = QString::fromUtf8(response.request().query().c_str());
  QMap<QString, PendingState>::iterator it = queries_.find(query_string);
  if (it == queries_.end())
    return;

  PendingState state = it.value();
  queries_.erase(it);

  ResultList ret;
  for (int i=0; i < response.result_size() ; ++i) {
    const spotify_pb::Track& track = response.result(i);

    Result result(this);
    result.type_ = Result::Type_Track;
    SpotifyService::SongFromProtobuf(track, &result.metadata_);
    result.match_quality_ = MatchQuality(state.tokens_, result.metadata_.title());

    ret << result;
  }

  for (int i=0 ; i<response.album_size() ; ++i) {
    const spotify_pb::Track& track = response.album(i);

    Result result(this);
    result.type_ = Result::Type_Album;
    SpotifyService::SongFromProtobuf(track, &result.metadata_);
    result.match_quality_ =
        qMin(MatchQuality(state.tokens_, result.metadata_.album()),
             MatchQuality(state.tokens_, result.metadata_.artist()));
    result.album_size_ = 0;
    ret << result;
  }

  emit ResultsAvailable(state.orig_id_, ret);
  emit SearchFinished(state.orig_id_);
}

void SpotifySearchProvider::LoadArtAsync(int id, const Result& result) {
  SpotifyServer* s = server();
  if (!s) {
    emit ArtLoaded(id, QImage());
    return;
  }

  QString image_id = QUrl(result.metadata_.art_automatic()).path();
  if (image_id.startsWith('/'))
    image_id.remove(0, 1);

  pending_art_[image_id] = id;
  s->LoadImage(image_id);
}

void SpotifySearchProvider::ArtLoadedSlot(const QString& id, const QImage& image) {
  QMap<QString, int>::iterator it = pending_art_.find(id);
  if (it == pending_art_.end())
    return;

  const int orig_id = it.value();
  pending_art_.erase(it);

  emit ArtLoaded(orig_id, ScaleAndPad(image));
}

void SpotifySearchProvider::LoadTracksAsync(int id, const Result& result) {
  switch (result.type_) {
  case Result::Type_Track: {
    SongMimeData* mime_data = new SongMimeData;
    mime_data->songs = SongList() << result.metadata_;
    emit TracksLoaded(id, mime_data);
    break;
  }

  case Result::Type_Album: {
    SpotifyServer* s = server();
    if (!s) {
      emit TracksLoaded(id, NULL);
      return;
    }

    QString uri = result.metadata_.url().toString();

    pending_tracks_[uri] = id;
    s->AlbumBrowse(uri);
    break;
  }
  }
}

void SpotifySearchProvider::AlbumBrowseResponse(const spotify_pb::BrowseAlbumResponse& response) {
  QString uri = QStringFromStdString(response.uri());
  QMap<QString, int>::iterator it = pending_tracks_.find(uri);
  if (it == pending_tracks_.end())
    return;

  const int orig_id = it.value();
  pending_tracks_.erase(it);

  SongMimeData* mime_data = new SongMimeData;

  for (int i=0 ; i<response.track_size() ; ++i) {
    Song song;
    SpotifyService::SongFromProtobuf(response.track(i), &song);
    mime_data->songs << song;
  }

  emit TracksLoaded(orig_id, mime_data);
}


