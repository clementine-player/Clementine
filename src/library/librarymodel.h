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

#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QNetworkDiskCache>

#include "libraryitem.h"
#include "libraryquery.h"
#include "librarywatcher.h"
#include "sqlrow.h"
#include "core/simpletreemodel.h"
#include "core/song.h"
#include "covers/albumcoverloaderoptions.h"
#include "engines/engine_fwd.h"
#include "playlist/playlistmanager.h"
#include "smartplaylists/generator_fwd.h"

class Application;
class AlbumCoverLoader;
class LibraryDirectoryModel;
class LibraryBackend;
namespace smart_playlists {
class Search;
}

class QSettings;

class LibraryModel : public SimpleTreeModel<LibraryItem> {
  Q_OBJECT
  Q_ENUMS(GroupBy);

 public:
  LibraryModel(LibraryBackend* backend, Application* app,
               QObject* parent = nullptr);
  ~LibraryModel();

  static const char* kSmartPlaylistsMimeType;
  static const char* kSmartPlaylistsSettingsGroup;
  static const char* kSmartPlaylistsArray;
  static const char* kSavedGroupingsSettingsGroup;
  static const int kSmartPlaylistsVersion;
  static const int kPrettyCoverSize;
  static const qint64 kIconCacheSize;

  enum Role {
    Role_Type = Qt::UserRole + 1,
    Role_ContainerType,
    Role_SortText,
    Role_Key,
    Role_Artist,
    Role_IsDivider,
    Role_Editable,
    LastRole
  };

  // These values get saved in QSettings - don't change them
  enum GroupBy {
    GroupBy_None = 0,
    GroupBy_Artist = 1,
    GroupBy_Album = 2,
    GroupBy_YearAlbum = 3,
    GroupBy_Year = 4,
    GroupBy_Composer = 5,
    GroupBy_Genre = 6,
    GroupBy_AlbumArtist = 7,
    GroupBy_FileType = 8,
    GroupBy_Performer = 9,
    GroupBy_Grouping = 10,
    GroupBy_Bitrate = 11,
    GroupBy_Disc = 12,
    GroupBy_OriginalYearAlbum = 13,
    GroupBy_OriginalYear = 14,
  };

  struct Grouping {
    Grouping(GroupBy f = GroupBy_None, GroupBy s = GroupBy_None,
             GroupBy t = GroupBy_None)
        : first(f), second(s), third(t) {}

    GroupBy first;
    GroupBy second;
    GroupBy third;

    const GroupBy& operator[](int i) const;
    GroupBy& operator[](int i);
    bool operator==(const Grouping& other) const {
      return first == other.first && second == other.second &&
             third == other.third;
    }
    bool operator!=(const Grouping& other) const { return !(*this == other); }
  };

  struct QueryResult {
    QueryResult() : create_va(false) {}

    SqlRowList rows;
    bool create_va;
  };

  LibraryBackend* backend() const { return backend_; }
  LibraryDirectoryModel* directory_model() const { return dir_model_; }

  typedef QList<smart_playlists::GeneratorPtr> GeneratorList;
  typedef QList<GeneratorList> DefaultGenerators;

  // Call before Init()
  void set_show_smart_playlists(bool show_smart_playlists) {
    show_smart_playlists_ = show_smart_playlists;
  }
  void set_default_smart_playlists(const DefaultGenerators& defaults) {
    default_smart_playlists_ = defaults;
  }
  void set_show_various_artists(bool show_various_artists) {
    show_various_artists_ = show_various_artists;
  }

  // Get information about the library
  void GetChildSongs(LibraryItem* item, QList<QUrl>* urls, SongList* songs,
                     QSet<int>* song_ids) const;
  SongList GetChildSongs(const QModelIndex& index) const;
  SongList GetChildSongs(const QModelIndexList& indexes) const;

  // Might be accurate
  int total_song_count() const { return total_song_count_; }

  // Smart playlists
  smart_playlists::GeneratorPtr CreateGenerator(const QModelIndex& index) const;
  void AddGenerator(smart_playlists::GeneratorPtr gen);
  void UpdateGenerator(const QModelIndex& index,
                       smart_playlists::GeneratorPtr gen);
  void DeleteGenerator(const QModelIndex& index);

  // QAbstractItemModel
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool canFetchMore(const QModelIndex& parent) const;

  // Whether or not to use album cover art, if it exists, in the library view
  void set_pretty_covers(bool use_pretty_covers);
  bool use_pretty_covers() const { return use_pretty_covers_; }

  // Whether or not to show letters heading in the library view
  void set_show_dividers(bool show_dividers);

  // Save the current grouping
  void SaveGrouping(QString name);

  // Utility functions for manipulating text
  static QString TextOrUnknown(const QString& text);
  static QString PrettyYearAlbum(int year, const QString& album);
  static QString SortText(QString text);
  static QString SortTextForArtist(QString artist);
  static QString SortTextForNumber(int year);
  static QString SortTextForSong(const Song& song);

signals:
  void TotalSongCountUpdated(int count);
  void GroupingChanged(const LibraryModel::Grouping& g);

 public slots:
  void SetFilterAge(int age);
  void SetFilterText(const QString& text);
  void SetFilterQueryMode(QueryOptions::QueryMode query_mode);

  void SetGroupBy(const LibraryModel::Grouping& g);
  const LibraryModel::Grouping& GetGroupBy() const { return group_by_; }
  void Init(bool async = true);
  void Reset();
  void ResetAsync();

 protected:
  void LazyPopulate(LibraryItem* item) { LazyPopulate(item, true); }
  void LazyPopulate(LibraryItem* item, bool signal);

 private slots:
  // From LibraryBackend
  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);
  void SongsSlightlyChanged(const SongList& songs);
  void TotalSongCountUpdatedSlot(int count);

  // Called after ResetAsync
  void ResetAsyncQueryFinished(QFuture<LibraryModel::QueryResult> future);

  void AlbumArtLoaded(quint64 id, const QImage& image);

 private:
  // Provides some optimisations for loading the list of items in the root.
  // This gets called a lot when filtering the playlist, so it's nice to be
  // able to do it in a background thread.
  QueryResult RunQuery(LibraryItem* parent);
  void PostQuery(LibraryItem* parent, const QueryResult& result, bool signal);

  bool HasCompilations(const LibraryQuery& query);

  void BeginReset();

  // Functions for working with queries and creating items.
  // When the model is reset or when a node is lazy-loaded the Library
  // constructs a database query to populate the items.  Filters are added
  // for each parent item, restricting the songs returned to a particular
  // album or artist for example.
  static void InitQuery(GroupBy type, LibraryQuery* q);
  void FilterQuery(GroupBy type, LibraryItem* item, LibraryQuery* q);

  // Items can be created either from a query that's been run to populate a
  // node, or by a spontaneous SongsDiscovered emission from the backend.
  LibraryItem* ItemFromQuery(GroupBy type, bool signal, bool create_divider,
                             LibraryItem* parent, const SqlRow& row,
                             int container_level);
  LibraryItem* ItemFromSong(GroupBy type, bool signal, bool create_divider,
                            LibraryItem* parent, const Song& s,
                            int container_level);

  // The "Various Artists" node is an annoying special case.
  LibraryItem* CreateCompilationArtistNode(bool signal, LibraryItem* parent);

  // Smart playlists are shown in another top-level node
  void CreateSmartPlaylists();
  void SaveGenerator(QSettings* s, int i,
                     smart_playlists::GeneratorPtr generator) const;
  void ItemFromSmartPlaylist(const QSettings& s, bool notify) const;

  // Helpers for ItemFromQuery and ItemFromSong
  LibraryItem* InitItem(GroupBy type, bool signal, LibraryItem* parent,
                        int container_level);
  void FinishItem(GroupBy type, bool signal, bool create_divider,
                  LibraryItem* parent, LibraryItem* item);

  QString DividerKey(GroupBy type, LibraryItem* item) const;
  QString DividerDisplayText(GroupBy type, const QString& key) const;

  // Helpers
  QString AlbumIconPixmapCacheKey(const QModelIndex& index) const;
  QVariant AlbumIcon(const QModelIndex& index);
  QVariant data(const LibraryItem* item, int role) const;
  bool CompareItems(const LibraryItem* a, const LibraryItem* b) const;

 private:
  LibraryBackend* backend_;
  Application* app_;
  LibraryDirectoryModel* dir_model_;
  bool show_smart_playlists_;
  DefaultGenerators default_smart_playlists_;
  bool show_various_artists_;

  int total_song_count_;

  QueryOptions query_options_;
  Grouping group_by_;

  // Keyed on database ID
  QMap<int, LibraryItem*> song_nodes_;

  // Keyed on whatever the key is for that level - artist, album, year, etc.
  QMap<QString, LibraryItem*> container_nodes_[3];

  // Keyed on a letter, a year, a century, etc.
  QMap<QString, LibraryItem*> divider_nodes_;

  // Only applies if smart playlists are set to on
  LibraryItem* smart_playlist_node_;

  QIcon artist_icon_;
  QIcon album_icon_;
  // used as a generic icon to show when no cover art is found,
  // fixed to the same size as the artwork (32x32)
  QPixmap no_cover_icon_;
  QIcon playlists_dir_icon_;
  QIcon playlist_icon_;

  QNetworkDiskCache* icon_cache_;

  int init_task_id_;

  bool use_pretty_covers_;
  bool show_dividers_;

  AlbumCoverLoaderOptions cover_loader_options_;

  typedef QPair<LibraryItem*, QString> ItemAndCacheKey;
  QMap<quint64, ItemAndCacheKey> pending_art_;
  QSet<QString> pending_cache_keys_;
};

Q_DECLARE_METATYPE(LibraryModel::Grouping);

QDataStream& operator<<(QDataStream& s, const LibraryModel::Grouping& g);
QDataStream& operator>>(QDataStream& s, LibraryModel::Grouping& g);

#endif  // LIBRARYMODEL_H
