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

#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include "librarywatcher.h"
#include "libraryquery.h"
#include "libraryitem.h"
#include "core/backgroundthread.h"
#include "core/simpletreemodel.h"
#include "core/song.h"
#include "engines/engine_fwd.h"
#include "smartplaylists/generator_fwd.h"

#include <boost/scoped_ptr.hpp>

class LibraryDirectoryModel;
class LibraryBackend;
namespace smart_playlists { class Search; }

class QSettings;

class LibraryModel : public SimpleTreeModel<LibraryItem> {
  Q_OBJECT
  Q_ENUMS(GroupBy);

 public:
  LibraryModel(LibraryBackend* backend, QObject* parent = 0);
  ~LibraryModel();

  static const char* kSmartPlaylistsMimeType;
  static const char* kSmartPlaylistsSettingsGroup;

  enum Role {
    Role_Type = Qt::UserRole + 1,
    Role_ContainerType,
    Role_SortText,
    Role_Key,
    Role_Artist,

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
  };

  struct Grouping {
    Grouping(GroupBy f = GroupBy_None,
             GroupBy s = GroupBy_None,
             GroupBy t = GroupBy_None)
               : first(f), second(s), third(t) {}

    GroupBy first;
    GroupBy second;
    GroupBy third;

    const GroupBy& operator [](int i) const;
    GroupBy& operator [](int i);
    bool operator ==(const Grouping& other) const {
      return first == other.first &&
             second == other.second &&
             third == other.third;
    }
  };

  LibraryBackend* backend() const { return backend_; }
  LibraryDirectoryModel* directory_model() const { return dir_model_; }

  // Call before Init()
  void set_show_smart_playlists(bool show_smart_playlists) { show_smart_playlists_ = show_smart_playlists; }

  // Get information about the library
  void GetChildSongs(LibraryItem* item, QList<QUrl>* urls, SongList* songs,
                     QSet<int>* song_ids) const;
  SongList GetChildSongs(const QModelIndex& index) const;
  SongList GetChildSongs(const QModelIndexList& indexes) const;

  smart_playlists::GeneratorPtr CreateGenerator(const QModelIndex& index) const;

  // QAbstractItemModel
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool canFetchMore(const QModelIndex &parent) const;

 signals:
  void TotalSongCountUpdated(int count);
  void GroupingChanged(const LibraryModel::Grouping& g);

 public slots:
  void SetFilterAge(int age);
  void SetFilterText(const QString& text);
  void SetGroupBy(const LibraryModel::Grouping& g);
  void Init();

 protected:
  void LazyPopulate(LibraryItem* item) { LazyPopulate(item, true); }
  void LazyPopulate(LibraryItem* item, bool signal);

 private slots:
  // From LibraryBackend
  void SongsDiscovered(const SongList& songs);
  void SongsDeleted(const SongList& songs);
  void SongsStatisticsChanged(const SongList& songs);
  void Reset();

 private:
  void Initialise();

  // Functions for working with queries and creating items.
  // When the model is reset or when a node is lazy-loaded the Library
  // constructs a database query to populate the items.  Filters are added
  // for each parent item, restricting the songs returned to a particular
  // album or artist for example.
  void InitQuery(GroupBy type, LibraryQuery* q);
  void FilterQuery(GroupBy type, LibraryItem* item, LibraryQuery* q);

  // Items can be created either from a query that's been run to populate a
  // node, or by a spontaneous SongsDiscovered emission from the backend.
  LibraryItem* ItemFromQuery(GroupBy type, bool signal, bool create_divider,
                             LibraryItem* parent, const LibraryQuery& q,
                             int container_level);
  LibraryItem* ItemFromSong(GroupBy type, bool signal, bool create_divider,
                            LibraryItem* parent, const Song& s,
                            int container_level);

  // The "Various Artists" node is an annoying special case.
  LibraryItem* CreateCompilationArtistNode(bool signal, LibraryItem* parent);

  // Smart playlists are shown in another top-level node
  void CreateSmartPlaylists();
  void SaveDefaultGenerator(QSettings* s, int i, const QString& name,
                            const smart_playlists::Search& search) const;
  void SaveGenerator(QSettings* s, int i, smart_playlists::GeneratorPtr generator) const;

  // Helpers for ItemFromQuery and ItemFromSong
  LibraryItem* InitItem(GroupBy type, bool signal, LibraryItem* parent,
                        int container_level);
  void FinishItem(GroupBy type, bool signal, bool create_divider,
                  LibraryItem* parent, LibraryItem* item);

  // Functions for manipulating text
  QString TextOrUnknown(const QString& text) const;
  QString PrettyYearAlbum(int year, const QString& album) const;

  QString SortText(QString text) const;
  QString SortTextForArtist(QString artist) const;
  QString SortTextForYear(int year) const;
  QString SortTextForSong(const Song& song) const;

  QString DividerKey(GroupBy type, LibraryItem* item) const;
  QString DividerDisplayText(GroupBy type, const QString& key) const;

  // Helpers
  QVariant data(const LibraryItem* item, int role) const;
  bool CompareItems(const LibraryItem* a, const LibraryItem* b) const;

 private:
  LibraryBackend* backend_;
  LibraryDirectoryModel* dir_model_;
  bool show_smart_playlists_;

  QueryOptions query_options_;
  Grouping group_by_;

  // Keyed on database ID
  QMap<int, LibraryItem*> song_nodes_;

  // Keyed on whatever the key is for that level - artist, album, year, etc.
  QMap<QString, LibraryItem*> container_nodes_[3];

  // Keyed on a letter, a year, a century, etc.
  QMap<QString, LibraryItem*> divider_nodes_;

  // Only applies if the first level is "artist"
  LibraryItem* compilation_artist_node_;

  // Only applies if smart playlists are set to on
  LibraryItem* smart_playlist_node_;

  QIcon artist_icon_;
  QIcon album_icon_;
  QIcon no_cover_icon_;
  QIcon playlists_dir_icon_;
  QIcon playlist_icon_;
};

Q_DECLARE_METATYPE(LibraryModel::Grouping);

#endif // LIBRARYMODEL_H
