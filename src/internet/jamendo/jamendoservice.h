/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_JAMENDO_JAMENDOSERVICE_H_
#define INTERNET_JAMENDO_JAMENDOSERVICE_H_

#include <QXmlStreamReader>

#include "core/song.h"
#include "internet/core/internetservice.h"

class LibraryBackend;
class LibraryFilterWidget;
class LibraryModel;
class LibrarySearchProvider;
class NetworkAccessManager;
class SearchProvider;

class QIODevice;
class QMenu;
class QSortFilterProxyModel;

class JamendoService : public InternetService {
  Q_OBJECT

 public:
  JamendoService(Application* app, InternetModel* parent);
  ~JamendoService();

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

  void ShowContextMenu(const QPoint& global_pos);

  QWidget* HeaderWidget() const;

  LibraryBackend* library_backend() const { return library_backend_.get(); }

  static const char* kServiceName;
  static const char* kDirectoryUrl;
  static const char* kMp3StreamUrl;
  static const char* kOggStreamUrl;
  static const char* kAlbumCoverUrl;
  static const char* kAlbumInfoUrl;
  static const char* kDownloadAlbumUrl;
  static const char* kHomepage;

  static const char* kSongsTable;
  static const char* kFtsTable;
  static const char* kTrackIdsTable;
  static const char* kTrackIdsColumn;

  static const char* kSettingsGroup;

  static const int kBatchSize;
  static const int kApproxDatabaseSize;

 private:
  void ParseDirectory(QIODevice* device) const;

  typedef QList<int> TrackIdList;

  SongList ReadArtist(QXmlStreamReader* reader, TrackIdList* track_ids) const;
  SongList ReadAlbum(const QString& artist, QXmlStreamReader* reader,
                     TrackIdList* track_ids) const;
  Song ReadTrack(const QString& artist, const QString& album,
                 const QString& album_cover, int album_id,
                 QXmlStreamReader* reader, TrackIdList* track_ids) const;
  void InsertTrackIds(const TrackIdList& ids) const;

  void EnsureMenuCreated();

 private slots:
  void DownloadDirectory();
  void DownloadDirectoryProgress(qint64 received, qint64 total);
  void DownloadDirectoryFinished();
  void ParseDirectoryFinished();
  void UpdateTotalSongCount(int count);

  void AlbumInfo();
  void DownloadAlbum();
  void Homepage();

  void SearchProviderToggled(const SearchProvider* provider, bool enabled);

 private:
  NetworkAccessManager* network_;

  QAction* album_info_;
  QAction* download_album_;

  std::shared_ptr<LibraryBackend> library_backend_;
  LibraryFilterWidget* library_filter_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;
  LibrarySearchProvider* search_provider_;

  int load_database_task_id_;

  int total_song_count_;

  bool accepted_download_;
};

#endif  // INTERNET_JAMENDO_JAMENDOSERVICE_H_
