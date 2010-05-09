/* This file is part of Clementine.

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

#ifndef MAGNATUNESERVICE_H
#define MAGNATUNESERVICE_H

#include <QXmlStreamReader>

#include "radioservice.h"

class QNetworkAccessManager;
class QSortFilterProxyModel;

class LibraryBackend;
class LibraryModel;

class MagnatuneService : public RadioService {
  Q_OBJECT

 public:
  MagnatuneService(RadioModel* parent);
  ~MagnatuneService();

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kDatabaseUrl;
  static const char* kSongsTable;
  static const char* kHomepage;

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  void StartLoading(const QUrl &url);

  void ShowContextMenu(RadioItem* item, const QModelIndex& index,
                       const QPoint& global_pos);

  bool SetupLibraryFilter(LibraryFilterWidget *) const;

 private slots:
  void UpdateTotalSongCount(int count) { total_song_count_ = count; }
  void ReloadDatabase();
  void ReloadDatabaseFinished();

  void AddToPlaylist();
  void Homepage();

 private:
  Song ReadTrack(QXmlStreamReader& reader);

 private:
  RadioItem* root_;
  QMenu* context_menu_;
  QModelIndex context_item_;

  QAction* add_to_playlist_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;

  int total_song_count_;

  QNetworkAccessManager* network_;
};

#endif // MAGNATUNESERVICE_H
