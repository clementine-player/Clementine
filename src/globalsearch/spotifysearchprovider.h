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

#ifndef SPOTIFYSEARCHPROVIDER_H
#define SPOTIFYSEARCHPROVIDER_H

#include "searchprovider.h"
#include "spotifymessages.pb.h"

class SpotifyServer;
class SpotifyService;

class SpotifySearchProvider : public SearchProvider {
  Q_OBJECT

 public:
  SpotifySearchProvider(Application* app, QObject* parent = nullptr);

  void SearchAsync(int id, const QString& query);
  void LoadArtAsync(int id, const Result& result);
  QStringList GetSuggestions(int count);

  bool IsLoggedIn();
  void ShowConfig();

 private slots:
  void ServerDestroyed();
  void SearchFinishedSlot(const pb::spotify::SearchResponse& response);
  void ArtLoadedSlot(const QString& id, const QImage& image);
  void SuggestionsLoaded(const pb::spotify::LoadPlaylistResponse& response);
  void SuggestionsLoaded(const pb::spotify::BrowseToplistResponse& response);

 private:
  SpotifyServer* server();

  void LoadSuggestions();
  void AddSuggestionFromTrack(const pb::spotify::Track& track);
  void AddSuggestionFromAlbum(const pb::spotify::Album& album);

 private:
  SpotifyServer* server_;
  SpotifyService* service_;

  QMap<QString, PendingState> queries_;
  QMap<QString, int> pending_art_;
  QMap<QString, int> pending_tracks_;

  QSet<QString> suggestions_;
};

#endif  // SPOTIFYSEARCHPROVIDER_H
