/* This file is part of Clementine.
   Copyright 2021, Kenman Tsang <kentsangkm@pm.me>

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

#include "spotifywebapisearchprovider.h"

#include <qurl.h>

#include <iostream>

#include "internet/spotifywebapi/spotifywebapiservice.h"
#include "ui/iconloader.h"

namespace {
static constexpr int kNoRunningSearch = -1;
}

SpotifyWebApiSearchProvider::SpotifyWebApiSearchProvider(
    Application* app, SpotifyWebApiService* parent)
    : SearchProvider(app, parent),
      parent_{parent},
      last_search_id_{kNoRunningSearch} {
  Init("Spotify (Experimential)", "spotify_web_api",
       IconLoader::Load("spotify", IconLoader::Provider),
       WantsDelayedQueries | WantsSerialisedArtQueries | ArtIsProbablyRemote |
           CanGiveSuggestions);

  connect(parent, &SpotifyWebApiService::SearchFinished, this,
          &SpotifyWebApiSearchProvider::SearchFinishedSlot);
}

void SpotifyWebApiSearchProvider::SearchAsync(int id, const QString& query) {
  if (last_search_id_ != kNoRunningSearch) {
    // Cancel last pending search
    emit SearchFinished(last_search_id_);

    // Set the pending query
    last_search_id_ = id;
    last_query_ = query;

    // And wait for the current search to be completed
    return;
  }

  last_search_id_ = id;
  last_query_ = query;

  parent_->Search(last_search_id_, last_query_);
}

void SpotifyWebApiSearchProvider::SearchFinishedSlot(
    int searchId, const QList<Song>& apiResult) {
  ResultList ret;
  for (auto&& item : apiResult) {
    Result result{this};
    result.group_automatically_ = true;
    result.metadata_ = item;
    ret += result;
  }
  emit ResultsAvailable(searchId, ret);
  emit SearchFinished(searchId);

  // Search again if we have a pending query
  if (searchId != last_search_id_) {
    parent_->Search(last_search_id_, last_query_);
  } else {
    last_search_id_ = kNoRunningSearch;
  }
}

void SpotifyWebApiSearchProvider::LoadArtAsync(int id, const Result& result) {
  // TODO
}

void SpotifyWebApiSearchProvider::ShowConfig() {}

InternetService* SpotifyWebApiSearchProvider::internet_service() {
  return parent_;
}

QStringList SpotifyWebApiSearchProvider::GetSuggestions(int count) {
  // TODO
  return QStringList{};
}
