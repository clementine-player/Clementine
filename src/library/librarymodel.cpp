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

#include "librarymodel.h"

#include <functional>

#include <QFuture>
#include <QIODevice>
#include <QMetaEnum>
#include <QNetworkCacheMetaData>
#include <QNetworkDiskCache>
#include <QPixmapCache>
#include <QSettings>
#include <QStringList>
#include <QUrl>
#include <QtConcurrentRun>

#include "librarybackend.h"
#include "libraryitem.h"
#include "librarydirectorymodel.h"
#include "libraryview.h"
#include "sqlrow.h"
#include "core/application.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "covers/albumcoverloader.h"
#include "playlist/songmimedata.h"
#include "smartplaylists/generator.h"
#include "smartplaylists/generatormimedata.h"
#include "smartplaylists/querygenerator.h"
#include "ui/iconloader.h"

using std::placeholders::_1;
using std::placeholders::_2;

using smart_playlists::Generator;
using smart_playlists::GeneratorMimeData;
using smart_playlists::GeneratorPtr;
using smart_playlists::QueryGenerator;

const char* LibraryModel::kSmartPlaylistsMimeType =
    "application/x-clementine-smart-playlist-generator";
const char* LibraryModel::kSmartPlaylistsSettingsGroup =
    "SerialisedSmartPlaylists";
const char* LibraryModel::kSavedGroupingsSettingsGroup = "SavedGroupings";
const int LibraryModel::kSmartPlaylistsVersion = 4;
const int LibraryModel::kPrettyCoverSize = 32;
const qint64 LibraryModel::kIconCacheSize = 100000000;  //~100MB

static bool IsArtistGroupBy(const LibraryModel::GroupBy by) {
  return by == LibraryModel::GroupBy_Artist ||
         by == LibraryModel::GroupBy_AlbumArtist;
}

static bool IsCompilationArtistNode(const LibraryItem* node) {
  return node == node->parent->compilation_artist_node_;
}

LibraryModel::LibraryModel(LibraryBackend* backend, Application* app,
                           QObject* parent)
    : SimpleTreeModel<LibraryItem>(new LibraryItem(this), parent),
      backend_(backend),
      app_(app),
      dir_model_(new LibraryDirectoryModel(backend, this)),
      show_smart_playlists_(false),
      show_various_artists_(true),
      total_song_count_(0),
      artist_icon_(IconLoader::Load("x-clementine-artist", IconLoader::Base)),
      album_icon_(IconLoader::Load("x-clementine-album", IconLoader::Base)),
      playlists_dir_icon_(IconLoader::Load("folder-sound", IconLoader::Base)),
      playlist_icon_(IconLoader::Load("x-clementine-albums", IconLoader::Base)),
      icon_cache_(new QNetworkDiskCache(this)),
      init_task_id_(-1),
      use_pretty_covers_(false),
      show_dividers_(true) {
  root_->lazy_loaded = true;

  group_by_[0] = GroupBy_Artist;
  group_by_[1] = GroupBy_Album;
  group_by_[2] = GroupBy_None;

  cover_loader_options_.desired_height_ = kPrettyCoverSize;
  cover_loader_options_.pad_output_image_ = true;
  cover_loader_options_.scale_output_image_ = true;

  connect(app_->album_cover_loader(), SIGNAL(ImageLoaded(quint64, QImage)),
          SLOT(AlbumArtLoaded(quint64, QImage)));

  icon_cache_->setCacheDirectory(
      Utilities::GetConfigPath(Utilities::Path_CacheRoot) + "/pixmapcache");
  icon_cache_->setMaximumCacheSize(LibraryModel::kIconCacheSize);

  QIcon nocover = IconLoader::Load("nocover", IconLoader::Other);
  no_cover_icon_ = nocover.pixmap(nocover.availableSizes().last()).scaled(
                           kPrettyCoverSize, kPrettyCoverSize, 
                           Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);

  connect(backend_, SIGNAL(SongsDiscovered(SongList)),
          SLOT(SongsDiscovered(SongList)));
  connect(backend_, SIGNAL(SongsDeleted(SongList)),
          SLOT(SongsDeleted(SongList)));
  connect(backend_, SIGNAL(SongsStatisticsChanged(SongList)),
          SLOT(SongsSlightlyChanged(SongList)));
  connect(backend_, SIGNAL(SongsRatingChanged(SongList)),
          SLOT(SongsSlightlyChanged(SongList)));
  connect(backend_, SIGNAL(DatabaseReset()), SLOT(Reset()));
  connect(backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(TotalSongCountUpdatedSlot(int)));

  backend_->UpdateTotalSongCountAsync();
}

LibraryModel::~LibraryModel() { delete root_; }

void LibraryModel::set_pretty_covers(bool use_pretty_covers) {
  if (use_pretty_covers != use_pretty_covers_) {
    use_pretty_covers_ = use_pretty_covers;
    Reset();
  }
}

void LibraryModel::set_show_dividers(bool show_dividers) {
  if (show_dividers != show_dividers_) {
    show_dividers_ = show_dividers;
    Reset();
  }
}

void LibraryModel::SaveGrouping(QString name) {
  qLog(Debug) << "Model, save to: " << name;

  QByteArray buffer;
  QDataStream ds(&buffer, QIODevice::WriteOnly);
  ds << group_by_;

  QSettings s;
  s.beginGroup(kSavedGroupingsSettingsGroup);
  s.setValue(name, buffer);
}

void LibraryModel::Init(bool async) {
  if (async) {
    // Show a loading indicator in the model.
    LibraryItem* loading =
        new LibraryItem(LibraryItem::Type_LoadingIndicator, root_);
    loading->display_text = tr("Loading...");
    loading->lazy_loaded = true;
    reset();

    // Show a loading indicator in the status bar too.
    init_task_id_ = app_->task_manager()->StartTask(tr("Loading songs"));

    ResetAsync();
  } else {
    Reset();
  }
}

void LibraryModel::SongsDiscovered(const SongList& songs) {
  for (const Song& song : songs) {
    // Sanity check to make sure we don't add songs that are outside the user's
    // filter
    if (!query_options_.Matches(song)) continue;

    // Hey, we've already got that one!
    if (song_nodes_.contains(song.id())) continue;

    // Before we can add each song we need to make sure the required container
    // items already exist in the tree.  These depend on which "group by"
    // settings the user has on the library.  Eg. if the user grouped by
    // artist and album, we would need to make sure nodes for the song's artist
    // and album were already in the tree.

    // Find parent containers in the tree
    LibraryItem* container = root_;
    for (int i = 0; i < 3; ++i) {
      GroupBy type = group_by_[i];
      if (type == GroupBy_None) break;

      // Special case: if the song is a compilation and the current GroupBy
      // level is Artists, then we want the Various Artists node :(
      if (IsArtistGroupBy(type) && song.is_compilation()) {
        if (container->compilation_artist_node_ == nullptr)
          CreateCompilationArtistNode(true, container);
        container = container->compilation_artist_node_;
      } else {
        // Otherwise find the proper container at this level based on the
        // item's key
        QString key;
        switch (type) {
          case GroupBy_Album:
            key = song.album();
            break;
          case GroupBy_Artist:
            key = song.artist();
            break;
          case GroupBy_Composer:
            key = song.composer();
            break;
          case GroupBy_Performer:
            key = song.performer();
            break;
          case GroupBy_Disc:
            key = QString::number(song.disc());
            break;
          case GroupBy_Grouping:
            key = song.grouping();
            break;
          case GroupBy_Genre:
            key = song.genre();
            break;
          case GroupBy_AlbumArtist:
            key = song.effective_albumartist();
            break;
          case GroupBy_Year:
            key = QString::number(qMax(0, song.year()));
            break;
          case GroupBy_OriginalYear:
            key = QString::number(qMax(0, song.effective_originalyear()));
            break;
          case GroupBy_YearAlbum:
            key = PrettyYearAlbum(qMax(0, song.year()), song.album());
            break;
          case GroupBy_OriginalYearAlbum:
            key = PrettyYearAlbum(qMax(0, song.effective_originalyear()),
                                  song.album());
            break;
          case GroupBy_FileType:
            key = song.filetype();
            break;
          case GroupBy_Bitrate:
            key = song.bitrate();
            break;
          case GroupBy_None:
            qLog(Error) << "GroupBy_None";
            break;
        }

        // Does it exist already?
        if (!container_nodes_[i].contains(key)) {
          // Create the container
          container_nodes_[i][key] =
              ItemFromSong(type, true, i == 0, container, song, i);
        }
        container = container_nodes_[i][key];
      }

      // If we just created the damn thing then we don't need to continue into
      // it any further because it'll get lazy-loaded properly later.
      if (!container->lazy_loaded) break;
    }

    if (!container->lazy_loaded) continue;

    // We've gone all the way down to the deepest level and everything was
    // already lazy loaded, so now we have to create the song in the container.
    song_nodes_[song.id()] =
        ItemFromSong(GroupBy_None, true, false, container, song, -1);
  }
}

void LibraryModel::SongsSlightlyChanged(const SongList& songs) {
  // This is called if there was a minor change to the songs that will not
  // normally require the library to be restructured.  We can just update our
  // internal cache of Song objects without worrying about resetting the model.
  for (const Song& song : songs) {
    if (song_nodes_.contains(song.id())) {
      song_nodes_[song.id()]->metadata = song;
    }
  }
}

LibraryItem* LibraryModel::CreateCompilationArtistNode(bool signal,
                                                       LibraryItem* parent) {
  if (signal)
    beginInsertRows(ItemToIndex(parent), parent->children.count(),
                    parent->children.count());

  parent->compilation_artist_node_ =
      new LibraryItem(LibraryItem::Type_Container, parent);
  parent->compilation_artist_node_->compilation_artist_node_ = nullptr;
  parent->compilation_artist_node_->key = tr("Various artists");
  parent->compilation_artist_node_->sort_text = " various";
  parent->compilation_artist_node_->container_level =
      parent->container_level + 1;

  if (signal) endInsertRows();

  return parent->compilation_artist_node_;
}

QString LibraryModel::DividerKey(GroupBy type, LibraryItem* item) const {
  // Items which are to be grouped under the same divider must produce the
  // same divider key.  This will only get called for top-level items.

  if (item->sort_text.isEmpty()) return QString();

  switch (type) {
    case GroupBy_Album:
    case GroupBy_Artist:
    case GroupBy_Composer:
    case GroupBy_Performer:
    case GroupBy_Disc:
    case GroupBy_Grouping:
    case GroupBy_Genre:
    case GroupBy_AlbumArtist:
    case GroupBy_FileType: {
      QChar c = item->sort_text[0];
      if (c.isDigit()) return "0";
      if (c == ' ') return QString();
      if (c.decompositionTag() != QChar::NoDecomposition)
        return QChar(c.decomposition()[0]);
      return c;
    }

    case GroupBy_Year:
    case GroupBy_OriginalYear:
      return SortTextForNumber(item->sort_text.toInt() / 10 * 10);

    case GroupBy_YearAlbum:
      return SortTextForNumber(item->metadata.year());

    case GroupBy_OriginalYearAlbum:
      return SortTextForNumber(item->metadata.effective_originalyear());

    case GroupBy_Bitrate:
      return SortTextForNumber(item->metadata.bitrate());

    case GroupBy_None:
      return QString();
  }
  qLog(Error) << "Unknown GroupBy type" << type << "for item"
              << item->display_text;
  return QString();
}

QString LibraryModel::DividerDisplayText(GroupBy type,
                                         const QString& key) const {
  // Pretty display text for the dividers.

  switch (type) {
    case GroupBy_Album:
    case GroupBy_Artist:
    case GroupBy_Composer:
    case GroupBy_Performer:
    case GroupBy_Disc:
    case GroupBy_Grouping:
    case GroupBy_Genre:
    case GroupBy_AlbumArtist:
    case GroupBy_FileType:
      if (key == "0") return "0-9";
      return key.toUpper();

    case GroupBy_YearAlbum:
    case GroupBy_OriginalYearAlbum:
      if (key == "0000") return tr("Unknown");
      return key.toUpper();

    case GroupBy_Year:
    case GroupBy_OriginalYear:
      if (key == "0000") return tr("Unknown");
      return QString::number(key.toInt());  // To remove leading 0s

    case GroupBy_Bitrate:
      if (key == "000") return tr("Unknown");
      return QString::number(key.toInt());  // To remove leading 0s

    case GroupBy_None:
      // fallthrough
      ;
  }
  qLog(Error) << "Unknown GroupBy type" << type << "for divider key" << key;
  return QString();
}

void LibraryModel::SongsDeleted(const SongList& songs) {
  // Delete the actual song nodes first, keeping track of each parent so we
  // might check to see if they're empty later.
  QSet<LibraryItem*> parents;
  for (const Song& song : songs) {
    if (song_nodes_.contains(song.id())) {
      LibraryItem* node = song_nodes_[song.id()];

      if (node->parent != root_) parents << node->parent;

      beginRemoveRows(ItemToIndex(node->parent), node->row, node->row);
      node->parent->Delete(node->row);
      song_nodes_.remove(song.id());
      endRemoveRows();
    } else {
      // If we get here it means some of the songs we want to delete haven't
      // been lazy-loaded yet.  This is bad, because it would mean that to
      // clean up empty parents we would need to lazy-load them all
      // individually to see if they're empty.  This can take a very long time,
      // so better to just reset the model and be done with it.
      Reset();
      return;
    }
  }

  // Now delete empty parents
  QSet<QString> divider_keys;
  while (!parents.isEmpty()) {
    // Since we are going to remove elements from the container, we
    // need a copy to iterate over. If we iterate over the original,
    // the behavior will be undefined.
    QSet<LibraryItem*> parents_copy = parents;
    for (LibraryItem* node : parents_copy) {
      parents.remove(node);
      if (node->children.count() != 0) continue;

      // Consider its parent for the next round
      if (node->parent != root_) parents << node->parent;

      // Maybe consider its divider node
      if (node->container_level == 0)
        divider_keys << DividerKey(group_by_[0], node);

      // Special case the Various Artists node
      if (IsCompilationArtistNode(node))
        node->parent->compilation_artist_node_ = nullptr;
      else
        container_nodes_[node->container_level].remove(node->key);

      // It was empty - delete it
      beginRemoveRows(ItemToIndex(node->parent), node->row, node->row);
      node->parent->Delete(node->row);
      endRemoveRows();
    }
  }

  // Delete empty dividers
  for (const QString& divider_key : divider_keys) {
    if (!divider_nodes_.contains(divider_key)) continue;

    // Look to see if there are any other items still under this divider
    bool found = false;
    for (LibraryItem* node : container_nodes_[0].values()) {
      if (DividerKey(group_by_[0], node) == divider_key) {
        found = true;
        break;
      }
    }

    if (found) continue;

    // Remove the divider
    int row = divider_nodes_[divider_key]->row;
    beginRemoveRows(ItemToIndex(root_), row, row);
    root_->Delete(row);
    endRemoveRows();
    divider_nodes_.remove(divider_key);
  }
}

QString LibraryModel::AlbumIconPixmapCacheKey(const QModelIndex& index) const {
  QStringList path;
  QModelIndex index_copy(index);
  while (index_copy.isValid()) {
    path.prepend(index_copy.data().toString());
    index_copy = index_copy.parent();
  }

  return "libraryart:" + path.join("/");
}

QVariant LibraryModel::AlbumIcon(const QModelIndex& index) {
  LibraryItem* item = IndexToItem(index);
  if (!item) return no_cover_icon_;

  // Check the cache for a pixmap we already loaded.
  const QString cache_key = AlbumIconPixmapCacheKey(index);
  QPixmap cached_pixmap;
  if (QPixmapCache::find(cache_key, &cached_pixmap)) {
    return cached_pixmap;
  }

  // Try to load it from the disk cache
  std::unique_ptr<QIODevice> cache(icon_cache_->data(QUrl(cache_key)));
  if (cache) {
    QImage cached_pixmap;
    if (cached_pixmap.load(cache.get(), "XPM")) {
      QPixmapCache::insert(cache_key, QPixmap::fromImage(cached_pixmap));
      return QPixmap::fromImage(cached_pixmap);
    }
  }

  // Maybe we're loading a pixmap already?
  if (pending_cache_keys_.contains(cache_key)) {
    return no_cover_icon_;
  }

  // No art is cached and we're not loading it already.  Load art for the first
  // Song in the album.
  SongList songs = GetChildSongs(index);
  if (!songs.isEmpty()) {
    const quint64 id = app_->album_cover_loader()->LoadImageAsync(
        cover_loader_options_, songs.first());
    pending_art_[id] = ItemAndCacheKey(item, cache_key);
    pending_cache_keys_.insert(cache_key);
  }

  return no_cover_icon_;
}

void LibraryModel::AlbumArtLoaded(quint64 id, const QImage& image) {
  ItemAndCacheKey item_and_cache_key = pending_art_.take(id);
  LibraryItem* item = item_and_cache_key.first;
  const QString& cache_key = item_and_cache_key.second;
  if (!item) return;

  pending_cache_keys_.remove(cache_key);

  // Insert this image in the cache.
  if (image.isNull()) {
    // Set the no_cover image so we don't continually try to load art.
    QPixmapCache::insert(cache_key, no_cover_icon_);
  } else {
    QPixmapCache::insert(cache_key, QPixmap::fromImage(image));
  }

  // If we have a valid cover not already in the disk cache
  std::unique_ptr<QIODevice> cached_img(icon_cache_->data(QUrl(cache_key)));
  if (!cached_img && !image.isNull()) {
    QNetworkCacheMetaData item_metadata;
    item_metadata.setSaveToDisk(true);
    item_metadata.setUrl(QUrl(cache_key));
    QIODevice* cache = icon_cache_->prepare(item_metadata);
    if (cache) {
      image.save(cache, "XPM");
      icon_cache_->insert(cache);
    }
  }

  const QModelIndex index = ItemToIndex(item);
  emit dataChanged(index, index);
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const {
  const LibraryItem* item = IndexToItem(index);

  // Handle a special case for returning album artwork instead of a generic CD
  // icon.
  // this is here instead of in the other data() function to let us use the
  // QModelIndex& version of GetChildSongs, which satisfies const-ness, instead
  // of the LibraryItem* version, which doesn't.
  if (use_pretty_covers_) {
    bool is_album_node = false;
    if (role == Qt::DecorationRole &&
        item->type == LibraryItem::Type_Container) {
      GroupBy container_type = group_by_[item->container_level];
      is_album_node = container_type == GroupBy_Album ||
                      container_type == GroupBy_YearAlbum ||
                      container_type == GroupBy_OriginalYearAlbum;
    }
    if (is_album_node) {
      // It has const behaviour some of the time - that's ok right?
      return const_cast<LibraryModel*>(this)->AlbumIcon(index);
    }
  }

  return data(item, role);
}

QVariant LibraryModel::data(const LibraryItem* item, int role) const {
  GroupBy container_type = item->type == LibraryItem::Type_Container
                               ? group_by_[item->container_level]
                               : GroupBy_None;

  switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      return item->DisplayText();

    case Qt::DecorationRole:
      switch (item->type) {
        case LibraryItem::Type_PlaylistContainer:
          return playlists_dir_icon_;
        case LibraryItem::Type_Container:
          switch (container_type) {
            case GroupBy_Album:
            case GroupBy_YearAlbum:
            case GroupBy_OriginalYearAlbum:
              return album_icon_;
            case GroupBy_Artist:
            case GroupBy_AlbumArtist:
              return artist_icon_;
            default:
              break;
          }
          break;
        case LibraryItem::Type_SmartPlaylist:
          return playlist_icon_;
        default:
          break;
      }
      break;

    case Role_Type:
      return item->type;

    case Role_IsDivider:
      return item->type == LibraryItem::Type_Divider;

    case Role_ContainerType:
      return container_type;

    case Role_Key:
      return item->key;

    case Role_Artist:
      return item->metadata.artist();

    case Role_Editable:
      if (!item->lazy_loaded) {
        const_cast<LibraryModel*>(this)
            ->LazyPopulate(const_cast<LibraryItem*>(item), true);
      }

      if (item->type == LibraryItem::Type_Container) {
        // if we have even one non editable item as a child, we ourselves
        // are not available for edit
        if (!item->children.isEmpty()) {
          for (LibraryItem* child : item->children) {
            if (!data(child, role).toBool()) {
              return false;
            }
          }
          return true;
        } else {
          return false;
        }
      } else if (item->type == LibraryItem::Type_Song) {
        return item->metadata.IsEditable();
      } else {
        return false;
      }

    case Role_SortText:
      return item->SortText();
  }
  return QVariant();
}

bool LibraryModel::HasCompilations(const LibraryQuery& query) {
  LibraryQuery q = query;
  q.AddCompilationRequirement(true);
  q.SetLimit(1);

  QMutexLocker l(backend_->db()->Mutex());
  if (!backend_->ExecQuery(&q)) return false;

  return q.Next();
}

LibraryModel::QueryResult LibraryModel::RunQuery(LibraryItem* parent) {
  QueryResult result;

  // Information about what we want the children to be
  int child_level = parent == root_ ? 0 : parent->container_level + 1;
  GroupBy child_type = child_level >= 3 ? GroupBy_None : group_by_[child_level];

  // Initialise the query.  child_type says what type of thing we want (artists,
  // songs, etc.)
  LibraryQuery q(query_options_);
  InitQuery(child_type, &q);

  // Walk up through the item's parents adding filters as necessary
  LibraryItem* p = parent;
  while (p && p->type == LibraryItem::Type_Container) {
    FilterQuery(group_by_[p->container_level], p, &q);
    p = p->parent;
  }

  // Artists GroupBy is special - we don't want compilation albums appearing
  if (IsArtistGroupBy(child_type)) {
    // Add the special Various artists node
    if (show_various_artists_ && HasCompilations(q)) {
      result.create_va = true;
    }

    // Don't show compilations again outside the Various artists node
    q.AddCompilationRequirement(false);
  }

  // Execute the query
  QMutexLocker l(backend_->db()->Mutex());
  if (!backend_->ExecQuery(&q)) return result;

  while (q.Next()) {
    result.rows << SqlRow(q);
  }
  return result;
}

void LibraryModel::PostQuery(LibraryItem* parent,
                             const LibraryModel::QueryResult& result,
                             bool signal) {
  // Information about what we want the children to be
  int child_level = parent == root_ ? 0 : parent->container_level + 1;
  GroupBy child_type = child_level >= 3 ? GroupBy_None : group_by_[child_level];

  if (result.create_va) {
    CreateCompilationArtistNode(signal, parent);
  }

  // Step through the results
  for (const SqlRow& row : result.rows) {
    // Create the item - it will get inserted into the model here
    LibraryItem* item = ItemFromQuery(child_type, signal, child_level == 0,
                                      parent, row, child_level);

    // Save a pointer to it for later
    if (child_type == GroupBy_None)
      song_nodes_[item->metadata.id()] = item;
    else
      container_nodes_[child_level][item->key] = item;
  }
}

void LibraryModel::LazyPopulate(LibraryItem* parent, bool signal) {
  if (parent->lazy_loaded) return;
  parent->lazy_loaded = true;

  QueryResult result = RunQuery(parent);
  PostQuery(parent, result, signal);
}

void LibraryModel::ResetAsync() {
  QFuture<LibraryModel::QueryResult> future =
      QtConcurrent::run(this, &LibraryModel::RunQuery, root_);
  NewClosure(future, this,
             SLOT(ResetAsyncQueryFinished(QFuture<LibraryModel::QueryResult>)),
             future);
}

void LibraryModel::ResetAsyncQueryFinished(
    QFuture<LibraryModel::QueryResult> future) {
  const struct QueryResult result = future.result();

  BeginReset();
  root_->lazy_loaded = true;

  PostQuery(root_, result, false);

  if (init_task_id_ != -1) {
    app_->task_manager()->SetTaskFinished(init_task_id_);
    init_task_id_ = -1;
  }

  endResetModel();
}

void LibraryModel::BeginReset() {
  beginResetModel();
  delete root_;
  song_nodes_.clear();
  container_nodes_[0].clear();
  container_nodes_[1].clear();
  container_nodes_[2].clear();
  divider_nodes_.clear();
  pending_art_.clear();
  smart_playlist_node_ = nullptr;

  root_ = new LibraryItem(this);
  root_->compilation_artist_node_ = nullptr;
  root_->lazy_loaded = false;

  // Smart playlists?
  if (show_smart_playlists_ && query_options_.filter().isEmpty())
    CreateSmartPlaylists();
}

void LibraryModel::Reset() {
  BeginReset();

  // Populate top level
  LazyPopulate(root_, false);

  endResetModel();
}

void LibraryModel::InitQuery(GroupBy type, LibraryQuery* q) {
  // Say what type of thing we want to get back from the database.
  switch (type) {
    case GroupBy_Artist:
      q->SetColumnSpec("DISTINCT artist");
      break;
    case GroupBy_Album:
      q->SetColumnSpec("DISTINCT album");
      break;
    case GroupBy_Composer:
      q->SetColumnSpec("DISTINCT composer");
      break;
    case GroupBy_Performer:
      q->SetColumnSpec("DISTINCT performer");
      break;
    case GroupBy_Disc:
      q->SetColumnSpec("DISTINCT disc");
      break;
    case GroupBy_Grouping:
      q->SetColumnSpec("DISTINCT grouping");
      break;
    case GroupBy_YearAlbum:
      q->SetColumnSpec("DISTINCT year, album, grouping");
      break;
    case GroupBy_OriginalYearAlbum:
      q->SetColumnSpec("DISTINCT year, originalyear, album, grouping");
      break;
    case GroupBy_Year:
      q->SetColumnSpec("DISTINCT year");
      break;
    case GroupBy_OriginalYear:
      q->SetColumnSpec("DISTINCT effective_originalyear");
      break;
    case GroupBy_Genre:
      q->SetColumnSpec("DISTINCT genre");
      break;
    case GroupBy_AlbumArtist:
      q->SetColumnSpec("DISTINCT effective_albumartist");
      break;
    case GroupBy_Bitrate:
      q->SetColumnSpec("DISTINCT bitrate");
      break;
    case GroupBy_None:
      q->SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);
      break;
    case GroupBy_FileType:
      q->SetColumnSpec("DISTINCT filetype");
      break;
  }
}

void LibraryModel::FilterQuery(GroupBy type, LibraryItem* item,
                               LibraryQuery* q) {
  // Say how we want the query to be filtered.  This is done once for each
  // parent going up the tree.

  switch (type) {
    case GroupBy_Artist:
      if (IsCompilationArtistNode(item))
        q->AddCompilationRequirement(true);
      else {
        // Don't duplicate compilations outside the Various artists node
        q->AddCompilationRequirement(false);
        q->AddWhere("artist", item->key);
      }
      break;
    case GroupBy_Album:
      q->AddWhere("album", item->key);
      break;
    case GroupBy_YearAlbum:
      q->AddWhere("year", item->metadata.year());
      q->AddWhere("album", item->metadata.album());
      q->AddWhere("grouping", item->metadata.grouping());
      break;
    case GroupBy_OriginalYearAlbum:
      q->AddWhere("year", item->metadata.year());
      q->AddWhere("originalyear", item->metadata.originalyear());
      q->AddWhere("album", item->metadata.album());
      q->AddWhere("grouping", item->metadata.grouping());
      break;

    case GroupBy_Year:
      q->AddWhere("year", item->key);
      break;
    case GroupBy_OriginalYear:
      q->AddWhere("effective_originalyear", item->key);
      break;
    case GroupBy_Composer:
      q->AddWhere("composer", item->key);
      break;
    case GroupBy_Performer:
      q->AddWhere("performer", item->key);
      break;
    case GroupBy_Disc:
      q->AddWhere("disc", item->key);
      break;
    case GroupBy_Grouping:
      q->AddWhere("grouping", item->key);
      break;
    case GroupBy_Genre:
      q->AddWhere("genre", item->key);
      break;
    case GroupBy_AlbumArtist:
      if (IsCompilationArtistNode(item))
        q->AddCompilationRequirement(true);
      else {
        // Don't duplicate compilations outside the Various artists node
        q->AddCompilationRequirement(false);
        q->AddWhere("effective_albumartist", item->key);
      }
      break;
    case GroupBy_FileType:
      q->AddWhere("filetype", item->metadata.filetype());
      break;
    case GroupBy_Bitrate:
      q->AddWhere("bitrate", item->key);
      break;
    case GroupBy_None:
      qLog(Error) << "Unknown GroupBy type" << type << "used in filter";
      break;
  }
}

LibraryItem* LibraryModel::InitItem(GroupBy type, bool signal,
                                    LibraryItem* parent, int container_level) {
  LibraryItem::Type item_type = type == GroupBy_None
                                    ? LibraryItem::Type_Song
                                    : LibraryItem::Type_Container;

  if (signal)
    beginInsertRows(ItemToIndex(parent), parent->children.count(),
                    parent->children.count());

  // Initialise the item depending on what type it's meant to be
  LibraryItem* item = new LibraryItem(item_type, parent);
  item->compilation_artist_node_ = nullptr;
  item->container_level = container_level;
  return item;
}

LibraryItem* LibraryModel::ItemFromQuery(GroupBy type, bool signal,
                                         bool create_divider,
                                         LibraryItem* parent, const SqlRow& row,
                                         int container_level) {
  LibraryItem* item = InitItem(type, signal, parent, container_level);
  int year = 0;
  int effective_originalyear = 0;
  int bitrate = 0;
  int disc = 0;

  switch (type) {
    case GroupBy_Artist:
      item->key = row.value(0).toString();
      item->display_text = TextOrUnknown(item->key);
      item->sort_text = SortTextForArtist(item->key);
      break;

    case GroupBy_YearAlbum:
      year = qMax(0, row.value(0).toInt());
      item->metadata.set_year(row.value(0).toInt());
      item->metadata.set_album(row.value(1).toString());
      item->metadata.set_grouping(row.value(2).toString());
      item->key = PrettyYearAlbum(year, item->metadata.album());
      item->sort_text = SortTextForNumber(year) + item->metadata.grouping() +
                        item->metadata.album();
      break;

    case GroupBy_OriginalYearAlbum:
      item->metadata.set_year(row.value(0).toInt());
      item->metadata.set_originalyear(row.value(1).toInt());
      item->metadata.set_album(row.value(2).toString());
      item->metadata.set_grouping(row.value(3).toString());
      effective_originalyear = qMax(0, item->metadata.effective_originalyear());
      item->key =
          PrettyYearAlbum(effective_originalyear, item->metadata.album());
      item->sort_text = SortTextForNumber(effective_originalyear) +
                        item->metadata.grouping() + item->metadata.album();
      break;

    case GroupBy_Year:
      year = qMax(0, row.value(0).toInt());
      item->key = QString::number(year);
      item->sort_text = SortTextForNumber(year) + " ";
      break;

    case GroupBy_OriginalYear:
      year = qMax(0, row.value(0).toInt());
      item->key = QString::number(year);
      item->sort_text = SortTextForNumber(year) + " ";
      break;

    case GroupBy_Composer:
    case GroupBy_Performer:
    case GroupBy_Grouping:
    case GroupBy_Genre:
    case GroupBy_Album:
    case GroupBy_AlbumArtist:
      item->key = row.value(0).toString();
      item->display_text = TextOrUnknown(item->key);
      item->sort_text = SortTextForArtist(item->key);
      break;

    case GroupBy_Disc:
      disc = row.value(0).toInt();
      item->key = QString::number(disc);
      item->sort_text = SortTextForNumber(disc);
      break;

    case GroupBy_FileType:
      item->metadata.set_filetype(Song::FileType(row.value(0).toInt()));
      item->key = item->metadata.TextForFiletype();
      break;

    case GroupBy_Bitrate:
      bitrate = qMax(0, row.value(0).toInt());
      item->key = QString::number(bitrate);
      item->sort_text = SortTextForNumber(bitrate) + " ";
      break;

    case GroupBy_None:
      item->metadata.InitFromQuery(row, true);
      item->key = item->metadata.title();
      item->display_text = item->metadata.TitleWithCompilationArtist();
      item->sort_text = SortTextForSong(item->metadata);
      break;
  }

  FinishItem(type, signal, create_divider, parent, item);
  return item;
}

LibraryItem* LibraryModel::ItemFromSong(GroupBy type, bool signal,
                                        bool create_divider,
                                        LibraryItem* parent, const Song& s,
                                        int container_level) {
  LibraryItem* item = InitItem(type, signal, parent, container_level);
  int year = 0;
  int originalyear = 0;
  int effective_originalyear = 0;
  int bitrate = 0;

  switch (type) {
    case GroupBy_Artist:
      item->key = s.artist();
      item->display_text = TextOrUnknown(item->key);
      item->sort_text = SortTextForArtist(item->key);
      break;

    case GroupBy_YearAlbum:
      year = qMax(0, s.year());
      item->metadata.set_year(year);
      item->metadata.set_album(s.album());
      item->key = PrettyYearAlbum(year, s.album());
      item->sort_text = SortTextForNumber(year) + s.grouping() + s.album();
      break;

    case GroupBy_OriginalYearAlbum:
      year = qMax(0, s.year());
      originalyear = qMax(0, s.originalyear());
      effective_originalyear = qMax(0, s.effective_originalyear());
      item->metadata.set_year(year);
      item->metadata.set_originalyear(originalyear);
      item->metadata.set_album(s.album());
      item->key = PrettyYearAlbum(effective_originalyear, s.album());
      item->sort_text =
          SortTextForNumber(effective_originalyear) + s.grouping() + s.album();
      break;

    case GroupBy_Year:
      year = qMax(0, s.year());
      item->key = QString::number(year);
      item->sort_text = SortTextForNumber(year) + " ";
      break;

    case GroupBy_OriginalYear:
      year = qMax(0, s.effective_originalyear());
      item->key = QString::number(year);
      item->sort_text = SortTextForNumber(year) + " ";
      break;

    case GroupBy_Composer:
      item->key = s.composer();
    case GroupBy_Performer:
      item->key = s.performer();
    case GroupBy_Grouping:
      item->key = s.grouping();
    case GroupBy_Genre:
      if (item->key.isNull()) item->key = s.genre();
    case GroupBy_Album:
      if (item->key.isNull()) item->key = s.album();
    case GroupBy_AlbumArtist:
      if (item->key.isNull()) item->key = s.effective_albumartist();
      item->display_text = TextOrUnknown(item->key);
      item->sort_text = SortTextForArtist(item->key);
      break;

    case GroupBy_Disc:
      item->key = QString::number(s.disc());
      item->sort_text = SortTextForNumber(s.disc());
      break;

    case GroupBy_FileType:
      item->metadata.set_filetype(s.filetype());
      item->key = s.TextForFiletype();
      break;

    case GroupBy_Bitrate:
      bitrate = qMax(0, s.bitrate());
      item->key = QString::number(bitrate);
      item->sort_text = SortTextForNumber(bitrate) + " ";
      break;

    case GroupBy_None:
      item->metadata = s;
      item->key = s.title();
      item->display_text = s.TitleWithCompilationArtist();
      item->sort_text = SortTextForSong(s);
      break;
  }

  FinishItem(type, signal, create_divider, parent, item);
  if (s.url().scheme() == "cdda") item->lazy_loaded = true;
  return item;
}

void LibraryModel::FinishItem(GroupBy type, bool signal, bool create_divider,
                              LibraryItem* parent, LibraryItem* item) {
  if (type == GroupBy_None) item->lazy_loaded = true;

  if (signal) endInsertRows();

  // Create the divider entry if we're supposed to
  if (create_divider && show_dividers_) {
    QString divider_key = DividerKey(type, item);
    item->sort_text.prepend(divider_key);

    if (!divider_key.isEmpty() && !divider_nodes_.contains(divider_key)) {
      if (signal)
        beginInsertRows(ItemToIndex(parent), parent->children.count(),
                        parent->children.count());

      LibraryItem* divider = new LibraryItem(LibraryItem::Type_Divider, root_);
      divider->key = divider_key;
      divider->display_text = DividerDisplayText(type, divider_key);
      divider->lazy_loaded = true;

      divider_nodes_[divider_key] = divider;

      if (signal) endInsertRows();
    }
  }
}

QString LibraryModel::TextOrUnknown(const QString& text) {
  if (text.isEmpty()) {
    return tr("Unknown");
  }
  return text;
}

QString LibraryModel::PrettyYearAlbum(int year, const QString& album) {
  if (year <= 0) return TextOrUnknown(album);
  return QString::number(year) + " - " + TextOrUnknown(album);
}

QString LibraryModel::SortText(QString text) {
  if (text.isEmpty()) {
    text = " unknown";
  } else {
    text = text.toLower();
  }
  text = text.remove(QRegExp("[^\\w ]"));

  return text;
}

QString LibraryModel::SortTextForArtist(QString artist) {
  artist = SortText(artist);

  if (artist.startsWith("the ")) {
    artist = artist.right(artist.length() - 4) + ", the";
  } else if (artist.startsWith("a ")) {
    artist = artist.right(artist.length() - 2) + ", a";
  } else if (artist.startsWith("an ")) {
    artist = artist.right(artist.length() - 3) + ", an";
  }

  return artist;
}

QString LibraryModel::SortTextForNumber(int number) {
  return QString("%1").arg(number, 4, 10, QChar('0'));
}

QString LibraryModel::SortTextForSong(const Song& song) {
  QString ret =
      QString::number(qMax(0, song.disc()) * 1000 + qMax(0, song.track()));
  ret.prepend(QString("0").repeated(6 - ret.length()));
  ret.append(song.url().toString());
  return ret;
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex& index) const {
  switch (IndexToItem(index)->type) {
    case LibraryItem::Type_Song:
    case LibraryItem::Type_Container:
    case LibraryItem::Type_SmartPlaylist:
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
    case LibraryItem::Type_Divider:
    case LibraryItem::Type_Root:
    case LibraryItem::Type_LoadingIndicator:
    default:
      return Qt::ItemIsEnabled;
  }
}

QStringList LibraryModel::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

QMimeData* LibraryModel::mimeData(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) return nullptr;

  // Special case: a smart playlist was dragged
  if (IndexToItem(indexes.first())->type == LibraryItem::Type_SmartPlaylist) {
    GeneratorPtr generator = CreateGenerator(indexes.first());
    if (!generator) return nullptr;

    GeneratorMimeData* data = new GeneratorMimeData(generator);
    data->setData(kSmartPlaylistsMimeType, QByteArray());
    data->name_for_new_playlist_ = this->data(indexes.first()).toString();
    return data;
  }

  SongMimeData* data = new SongMimeData;
  QList<QUrl> urls;
  QSet<int> song_ids;

  data->backend = backend_;

  for (const QModelIndex& index : indexes) {
    GetChildSongs(IndexToItem(index), &urls, &data->songs, &song_ids);
  }

  data->setUrls(urls);
  data->name_for_new_playlist_ =
      PlaylistManager::GetNameForNewPlaylist(data->songs);

  return data;
}

bool LibraryModel::CompareItems(const LibraryItem* a,
                                const LibraryItem* b) const {
  QVariant left(data(a, LibraryModel::Role_SortText));
  QVariant right(data(b, LibraryModel::Role_SortText));

  if (left.type() == QVariant::Int) return left.toInt() < right.toInt();
  return left.toString() < right.toString();
}

void LibraryModel::GetChildSongs(LibraryItem* item, QList<QUrl>* urls,
                                 SongList* songs, QSet<int>* song_ids) const {
  switch (item->type) {
    case LibraryItem::Type_Container: {
      const_cast<LibraryModel*>(this)->LazyPopulate(item);

      QList<LibraryItem*> children = item->children;
      qSort(children.begin(), children.end(),
            std::bind(&LibraryModel::CompareItems, this, _1, _2));

      for (LibraryItem* child : children)
        GetChildSongs(child, urls, songs, song_ids);
      break;
    }

    case LibraryItem::Type_Song:
      urls->append(item->metadata.url());
      if (!song_ids->contains(item->metadata.id())) {
        songs->append(item->metadata);
        song_ids->insert(item->metadata.id());
      }
      break;

    default:
      break;
  }
}

SongList LibraryModel::GetChildSongs(const QModelIndexList& indexes) const {
  QList<QUrl> dontcare;
  SongList ret;
  QSet<int> song_ids;

  for (const QModelIndex& index : indexes) {
    GetChildSongs(IndexToItem(index), &dontcare, &ret, &song_ids);
  }
  return ret;
}

SongList LibraryModel::GetChildSongs(const QModelIndex& index) const {
  return GetChildSongs(QModelIndexList() << index);
}

void LibraryModel::SetFilterAge(int age) {
  query_options_.set_max_age(age);
  ResetAsync();
}

void LibraryModel::SetFilterText(const QString& text) {
  query_options_.set_filter(text);
  ResetAsync();
}

void LibraryModel::SetFilterQueryMode(QueryOptions::QueryMode query_mode) {
  query_options_.set_query_mode(query_mode);
  ResetAsync();
}

bool LibraryModel::canFetchMore(const QModelIndex& parent) const {
  if (!parent.isValid()) return false;

  LibraryItem* item = IndexToItem(parent);
  return !item->lazy_loaded;
}

void LibraryModel::SetGroupBy(const Grouping& g) {
  group_by_ = g;

  ResetAsync();
  emit GroupingChanged(g);
}

const LibraryModel::GroupBy& LibraryModel::Grouping::operator[](int i) const {
  switch (i) {
    case 0:
      return first;
    case 1:
      return second;
    case 2:
      return third;
  }
  qLog(Error) << "LibraryModel::Grouping[] index out of range" << i;
  return first;
}

LibraryModel::GroupBy& LibraryModel::Grouping::operator[](int i) {
  switch (i) {
    case 0:
      return first;
    case 1:
      return second;
    case 2:
      return third;
  }
  qLog(Error) << "LibraryModel::Grouping[] index out of range" << i;
  return first;
}

void LibraryModel::CreateSmartPlaylists() {
  smart_playlist_node_ =
      new LibraryItem(LibraryItem::Type_PlaylistContainer, root_);
  smart_playlist_node_->container_level = 0;
  smart_playlist_node_->sort_text = "\0";
  smart_playlist_node_->key = tr("Smart playlists");
  smart_playlist_node_->lazy_loaded = true;

  QSettings s;
  s.beginGroup(kSmartPlaylistsSettingsGroup);
  int version = s.value(backend_->songs_table() + "_version", 0).toInt();

  // How many defaults do we have to write?
  int unwritten_defaults = 0;
  for (int i = version; i < default_smart_playlists_.count(); ++i) {
    unwritten_defaults += default_smart_playlists_[i].count();
  }

  // Save the defaults if there are any unwritten ones
  if (unwritten_defaults) {
    // How many items are stored already?
    int playlist_index = s.beginReadArray(backend_->songs_table());
    s.endArray();

    // Append the new ones
    s.beginWriteArray(backend_->songs_table(),
                      playlist_index + unwritten_defaults);
    for (; version < default_smart_playlists_.count(); ++version) {
      for (smart_playlists::GeneratorPtr gen :
           default_smart_playlists_[version]) {
        SaveGenerator(&s, playlist_index++, gen);
      }
    }
    s.endArray();
  }

  s.setValue(backend_->songs_table() + "_version", version);

  const int count = s.beginReadArray(backend_->songs_table());
  for (int i = 0; i < count; ++i) {
    s.setArrayIndex(i);
    ItemFromSmartPlaylist(s, false);
  }
}

void LibraryModel::ItemFromSmartPlaylist(const QSettings& s,
                                         bool notify) const {
  LibraryItem* item = new LibraryItem(LibraryItem::Type_SmartPlaylist,
                                      notify ? nullptr : smart_playlist_node_);
  item->display_text = tr(qPrintable(s.value("name").toString()));
  item->sort_text = item->display_text;
  item->key = s.value("type").toString();
  item->smart_playlist_data = s.value("data").toByteArray();
  item->lazy_loaded = true;

  if (notify) item->InsertNotify(smart_playlist_node_);
}

void LibraryModel::AddGenerator(GeneratorPtr gen) {
  QSettings s;
  s.beginGroup(kSmartPlaylistsSettingsGroup);

  // Count the existing items
  const int count = s.beginReadArray(backend_->songs_table());
  s.endArray();

  // Add this one to the end
  s.beginWriteArray(backend_->songs_table(), count + 1);
  SaveGenerator(&s, count, gen);

  // Add it to the model
  ItemFromSmartPlaylist(s, true);

  s.endArray();
}

void LibraryModel::UpdateGenerator(const QModelIndex& index, GeneratorPtr gen) {
  if (index.parent() != ItemToIndex(smart_playlist_node_)) return;
  LibraryItem* item = IndexToItem(index);
  if (!item) return;

  // Update the config
  QSettings s;
  s.beginGroup(kSmartPlaylistsSettingsGroup);

  // Count the existing items
  const int count = s.beginReadArray(backend_->songs_table());
  s.endArray();

  s.beginWriteArray(backend_->songs_table(), count);
  SaveGenerator(&s, index.row(), gen);

  // Update the text of the item
  item->display_text = gen->name();
  item->sort_text = item->display_text;
  item->key = gen->type();
  item->smart_playlist_data = gen->Save();
  item->ChangedNotify();
}

void LibraryModel::DeleteGenerator(const QModelIndex& index) {
  if (index.parent() != ItemToIndex(smart_playlist_node_)) return;

  // Remove the item from the tree
  smart_playlist_node_->DeleteNotify(index.row());

  QSettings s;
  s.beginGroup(kSmartPlaylistsSettingsGroup);

  // Rewrite all the items to the settings
  s.beginWriteArray(backend_->songs_table(),
                    smart_playlist_node_->children.count());
  int i = 0;
  for (LibraryItem* item : smart_playlist_node_->children) {
    s.setArrayIndex(i++);
    s.setValue("name", item->display_text);
    s.setValue("type", item->key);
    s.setValue("data", item->smart_playlist_data);
  }
  s.endArray();
}

void LibraryModel::SaveGenerator(QSettings* s, int i,
                                 GeneratorPtr generator) const {
  s->setArrayIndex(i);
  s->setValue("name", generator->name());
  s->setValue("type", generator->type());
  s->setValue("data", generator->Save());
}

GeneratorPtr LibraryModel::CreateGenerator(const QModelIndex& index) const {
  GeneratorPtr ret;

  const LibraryItem* item = IndexToItem(index);
  if (!item || item->type != LibraryItem::Type_SmartPlaylist) return ret;

  ret = Generator::Create(item->key);
  if (!ret) return ret;

  ret->set_name(item->display_text);
  ret->set_library(backend());
  ret->Load(item->smart_playlist_data);
  return ret;
}

void LibraryModel::TotalSongCountUpdatedSlot(int count) {
  total_song_count_ = count;
  emit TotalSongCountUpdated(count);
}

QDataStream& operator<<(QDataStream& s, const LibraryModel::Grouping& g) {
  s << quint32(g.first) << quint32(g.second) << quint32(g.third);
  return s;
}

QDataStream& operator>>(QDataStream& s, LibraryModel::Grouping& g) {
  quint32 buf;
  s >> buf;
  g.first = LibraryModel::GroupBy(buf);
  s >> buf;
  g.second = LibraryModel::GroupBy(buf);
  s >> buf;
  g.third = LibraryModel::GroupBy(buf);
  return s;
}
