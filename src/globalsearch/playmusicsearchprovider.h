/* This file is part of Clementine.
   Copyright 2015, Marco Kirchner <kirchnermarco@gmail.com>
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef PLAYMUSICSEARCHPROVIDER_H
#define PLAYMUSICSEARCHPROVIDER_H

#include "searchprovider.h"
#include "covers/albumcoverloaderoptions.h"
#include "internet/playmusic/playmusicservice.h"

class AlbumCoverLoader;

class PlayMusicSearchProvider : public SearchProvider {
  Q_OBJECT

 public:
  explicit PlayMusicSearchProvider(Application* app, QObject* parent = nullptr);
  void Init(PlayMusicService* service);

  // SearchProvider
  void SearchAsync(int id, const QString& query);
  void LoadArtAsync(int id, const Result& result);
  InternetService* internet_service() { return service_; }

 private slots:
  void AlbumArtLoaded(quint64 id, const QImage& image);
  void SearchDone(int id, const SongList& songs);

 private:
  void MaybeSearchFinished(int id);

  PlayMusicService* service_;
  QMap<int, PendingState> pending_searches_;

  AlbumCoverLoaderOptions cover_loader_options_;
  QMap<quint64, int> cover_loader_tasks_;
};

#endif
