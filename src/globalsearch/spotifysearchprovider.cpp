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

#include <ctime>
#include <random>

#include "core/logging.h"
#include "internet/internetmodel.h"
#include "internet/spotifyserver.h"
#include "internet/spotifyservice.h"
#include "playlist/songmimedata.h"

namespace {
const int kSearchSongLimit = 5;
const int kSearchAlbumLimit = 20;
}

SpotifySearchProvider::SpotifySearchProvider(Application* app, QObject* parent)
    : SearchProvider(app, parent), server_(nullptr), service_(nullptr) {
  Init("Spotify", "spotify", QIcon(":icons/32x32/spotify.png"),
       WantsDelayedQueries | WantsSerialisedArtQueries | ArtIsProbablyRemote |
           CanShowConfig | CanGiveSuggestions);
}

SpotifyServer* SpotifySearchProvider::server() {
  if (server_) return server_;

  if (!service_) service_ = InternetModel::Service<SpotifyService>();

  if (service_->login_state() != SpotifyService::LoginState_LoggedIn)
    return nullptr;

  if (!service_->IsBlobInstalled())
    return nullptr;

  server_ = service_->server();
  connect(server_, SIGNAL(SearchResults(pb::spotify::SearchResponse)),
          SLOT(SearchFinishedSlot(pb::spotify::SearchResponse)));
  connect(server_, SIGNAL(ImageLoaded(QString, QImage)),
          SLOT(ArtLoadedSlot(QString, QImage)));
  connect(server_, SIGNAL(destroyed()), SLOT(ServerDestroyed()));
  connect(server_, SIGNAL(StarredLoaded(pb::spotify::LoadPlaylistResponse)),
          SLOT(SuggestionsLoaded(pb::spotify::LoadPlaylistResponse)));
  connect(server_,
          SIGNAL(ToplistBrowseResults(pb::spotify::BrowseToplistResponse)),
          SLOT(SuggestionsLoaded(pb::spotify::BrowseToplistResponse)));

  return server_;
}

void SpotifySearchProvider::ServerDestroyed() { server_ = nullptr; }

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
  s->Search(query_string, kSearchSongLimit, kSearchAlbumLimit);
  queries_[query_string] = state;
}

void SpotifySearchProvider::SearchFinishedSlot(
    const pb::spotify::SearchResponse& response) {
  QString query_string = QString::fromUtf8(response.request().query().c_str());
  QMap<QString, PendingState>::iterator it = queries_.find(query_string);
  if (it == queries_.end()) return;

  PendingState state = it.value();
  queries_.erase(it);

  ResultList ret;
  for (int i = 0; i < response.result_size(); ++i) {
    const pb::spotify::Track& track = response.result(i);

    Result result(this);
    SpotifyService::SongFromProtobuf(track, &result.metadata_);

    ret << result;
  }

  for (int i = 0; i < response.album_size(); ++i) {
    const pb::spotify::Album& album = response.album(i);

    for (int j = 0; j < album.track_size(); ++j) {
      Result result(this);
      SpotifyService::SongFromProtobuf(album.track(j), &result.metadata_);

      // Just use the album index as an id.
      result.metadata_.set_album_id(i);

      ret << result;
    }
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
  if (image_id.startsWith('/')) image_id.remove(0, 1);

  pending_art_[image_id] = id;
  s->LoadImage(image_id);
}

void SpotifySearchProvider::ArtLoadedSlot(const QString& id,
                                          const QImage& image) {
  QMap<QString, int>::iterator it = pending_art_.find(id);
  if (it == pending_art_.end()) return;

  const int orig_id = it.value();
  pending_art_.erase(it);

  emit ArtLoaded(orig_id, ScaleAndPad(image));
}

bool SpotifySearchProvider::IsLoggedIn() {
  if (server()) {
    return service_->IsLoggedIn();
  }
  return false;
}

void SpotifySearchProvider::ShowConfig() {
  if (service_) {
    return service_->ShowConfig();
  }
}

void SpotifySearchProvider::AddSuggestionFromTrack(
    const pb::spotify::Track& track) {
  if (!track.title().empty()) {
    suggestions_.insert(QString::fromUtf8(track.title().c_str()));
  }
  for (int j = 0; j < track.artist_size(); ++j) {
    if (!track.artist(j).empty()) {
      suggestions_.insert(QString::fromUtf8(track.artist(j).c_str()));
    }
  }
  if (!track.album().empty()) {
    suggestions_.insert(QString::fromUtf8(track.album().c_str()));
  }
}

void SpotifySearchProvider::AddSuggestionFromAlbum(
    const pb::spotify::Album& album) {
  AddSuggestionFromTrack(album.metadata());
  for (int i = 0; i < album.track_size(); ++i) {
    AddSuggestionFromTrack(album.track(i));
  }
}

void SpotifySearchProvider::SuggestionsLoaded(
    const pb::spotify::LoadPlaylistResponse& playlist) {
  for (int i = 0; i < playlist.track_size(); ++i) {
    AddSuggestionFromTrack(playlist.track(i));
  }
}

void SpotifySearchProvider::SuggestionsLoaded(
    const pb::spotify::BrowseToplistResponse& response) {
  for (int i = 0; i < response.track_size(); ++i) {
    AddSuggestionFromTrack(response.track(i));
  }
  for (int i = 0; i < response.album_size(); ++i) {
    AddSuggestionFromAlbum(response.album(i));
  }
}

void SpotifySearchProvider::LoadSuggestions() {
  if (!server()) {
    return;
  }
  server()->LoadStarred();
  server()->LoadToplist();
}

QStringList SpotifySearchProvider::GetSuggestions(int count) {
  if (suggestions_.empty()) {
    LoadSuggestions();
    return QStringList();
  }

  QStringList all_suggestions = suggestions_.toList();

  std::mt19937 gen(std::time(0));
  std::uniform_int_distribution<> random(0, all_suggestions.size() - 1);

  QSet<QString> candidates;

  const int max = qMin(count, all_suggestions.size());
  while (candidates.size() < max) {
    const int index = random(gen);
    candidates.insert(all_suggestions[index]);
  }
  return candidates.toList();
}
