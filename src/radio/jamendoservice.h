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

#ifndef JAMENDOSERVICE_H
#define JAMENDOSERVICE_H

#include "radioservice.h"

#include <QXmlStreamReader>

#include "core/song.h"

class LibraryBackend;
class LibraryFilterWidget;
class LibraryModel;
class NetworkAccessManager;
class QIODevice;
class QMenu;
class QSortFilterProxyModel;

class JamendoService : public RadioService {
  Q_OBJECT
 public:
  JamendoService(RadioModel* parent);
  ~JamendoService();

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  QWidget* HeaderWidget() const;

  static const char* kServiceName;
  static const char* kDirectoryUrl;
  static const char* kMp3StreamUrl;
  static const char* kOggStreamUrl;
  static const char* kAlbumCoverUrl;

  static const char* kSongsTable;
  static const char* kFtsTable;

  static const char* kSettingsGroup;

  static const int kBatchSize;
  static const int kApproxDatabaseSize;

 private:
  void DownloadDirectory();
  void ParseDirectory(QIODevice* device) const;

  SongList ReadArtist(QXmlStreamReader* reader) const;
  SongList ReadAlbum(const QString& artist, QXmlStreamReader* reader) const;
  Song ReadTrack(const QString& artist,
                 const QString& album,
                 const QString& album_cover,
                 QXmlStreamReader* reader) const;

  void EnsureMenuCreated();

 private slots:
  void DownloadDirectoryProgress(qint64 received, qint64 total);
  void DownloadDirectoryFinished();
  void ParseDirectoryFinished();
  void UpdateTotalSongCount(int count);

 private:
  NetworkAccessManager* network_;
  RadioItem* root_;

  LibraryBackend* library_backend_;
  LibraryFilterWidget* library_filter_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;

  int load_database_task_id_;

  int total_song_count_;
};

#endif
