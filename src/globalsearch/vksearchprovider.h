/* This file is part of Clementine.
   Copyright 2013, Vlad Maltsev <shedwardx@gmail.com>

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


#ifndef VKSEARCHPROVIDER_H
#define VKSEARCHPROVIDER_H

#include "internet/vkservice.h"

#include "searchprovider.h"

class VkSearchProvider : public SearchProvider {
  Q_OBJECT

public:
  VkSearchProvider(Application* app, QObject* parent = 0);
  void Init(VkService* service);
  void SearchAsync(int id, const QString& query);
  bool IsLoggedIn();
  void ShowConfig();
  InternetService* internet_service() { return service_; }

public slots:
  void SongSearchResult(const SearchID& id, SongList songs);
  void GroupSearchResult(const SearchID& rid, const MusicOwnerList& groups);

private:
  bool songs_recived;
  bool groups_recived;
  void MaybeSearchFinished(int id);
  void ClearSimilarSongs(SongList& list);
  VkService* service_;
  QMap<int, PendingState> pending_searches_;
};

#endif  // VKSEARCHPROVIDER_H
