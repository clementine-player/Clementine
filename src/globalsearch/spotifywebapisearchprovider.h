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

#ifndef SPOTIFYWEBAPISEARCHPROVIDER_H
#define SPOTIFYWEBAPISEARCHPROVIDER_H

#include "core/song.h"
#include "searchprovider.h"

class SpotifyWebApiService;

class SpotifyWebApiSearchProvider : public SearchProvider {
  Q_OBJECT

 public:
  SpotifyWebApiSearchProvider(Application* app, SpotifyWebApiService* parent);

  void SearchAsync(int id, const QString& query) override;
  void LoadArtAsync(int id, const Result& result) override;
  QStringList GetSuggestions(int count) override;

  void ShowConfig() override;
  InternetService* internet_service() override;

 private slots:
  void SearchFinishedSlot(int id, const QList<Song>&);

 private:
  SpotifyWebApiService* parent_;
  int last_search_id_;
  QString last_query_;
};

#endif  // SPOTIFYWEBAPISEARCHPROVIDER_H
