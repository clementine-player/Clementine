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
#include "internet/internetmodel.h"
#include "internet/spotifyserver.h"
#include "internet/spotifyservice.h"

SpotifySearchProvider::SpotifySearchProvider(QObject* parent)
  : SearchProvider(parent),
    server_(NULL),
    service_(NULL)
{
  Init("Spotify", QIcon(":icons/svg/spotify.svg"), true);
}

SpotifyServer* SpotifySearchProvider::server() {
  if (server_)
    return server_;

  if (!service_)
    service_ = InternetModel::Service<SpotifyService>();

  if (service_->login_state() != SpotifyService::LoginState_LoggedIn)
    return NULL;

  server_ = service_->server();
  connect(server_, SIGNAL(SearchResults(protobuf::SearchResponse)),
          SLOT(SearchFinishedSlot(protobuf::SearchResponse)));
  connect(server_, SIGNAL(ImageLoaded(QString,QImage)),
          SLOT(ArtLoadedSlot(QString,QImage)));
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
  s->Search(query_string, 25);
  queries_[query_string] = state;
}

void SpotifySearchProvider::SearchFinishedSlot(const protobuf::SearchResponse& response) {
  QString query_string = QString::fromUtf8(response.request().query().c_str());
  QMap<QString, PendingState>::iterator it = queries_.find(query_string);
  if (it == queries_.end())
    return;

  PendingState state = it.value();
  queries_.erase(it);

  ResultList ret;
  for (int i = 0; i < response.result_size(); ++i) {
    const protobuf::Track& track = response.result(i);

    Result result(this);
    result.type_ = Result::Type_Track;
    SpotifyService::SongFromProtobuf(track, &result.metadata_);
    result.match_quality_ = MatchQuality(state.tokens_, result.metadata_.title());

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

  QString image_id = result.metadata_.url().path();
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
  emit TracksLoaded(id, SongList());
}


