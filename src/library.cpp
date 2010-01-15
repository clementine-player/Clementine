#include "library.h"
#include "librarybackend.h"
#include "libraryitem.h"
#include "songmimedata.h"
#include "libraryconfig.h"

#include <QStringList>
#include <QUrl>

#include <boost/bind.hpp>


Library::Library(EngineBase* engine, QObject* parent)
  : SimpleTreeModel<LibraryItem>(new LibraryItem(this), parent),
    engine_(engine),
    backend_(new BackgroundThread<LibraryBackend>(this)),
    watcher_(new BackgroundThread<LibraryWatcher>(this)),
    artist_icon_(":artist.png"),
    album_icon_(":album.png"),
    config_(new LibraryConfig)
{
  root_->lazy_loaded = true;

  connect(backend_, SIGNAL(Initialised()), SLOT(BackendInitialised()));
  connect(watcher_, SIGNAL(Initialised()), SLOT(WatcherInitialised()));
  waiting_for_threads_ = 2;
}

Library::~Library() {
  delete root_;
  delete config_;
}

void Library::StartThreads() {
  Q_ASSERT(waiting_for_threads_);

  backend_->start();
  watcher_->start();
}

void Library::BackendInitialised() {
  connect(backend_->Worker().get(), SIGNAL(SongsDiscovered(SongList)), SLOT(SongsDiscovered(SongList)));
  connect(backend_->Worker().get(), SIGNAL(SongsDeleted(SongList)), SLOT(SongsDeleted(SongList)));
  connect(backend_->Worker().get(), SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(backend_->Worker().get(), SIGNAL(TotalSongCountUpdated(int)), SIGNAL(TotalSongCountUpdated(int)));

  config_->SetBackend(backend_->Worker());

  if (--waiting_for_threads_ == 0)
    Initialise();
}

void Library::WatcherInitialised() {
  watcher_->Worker()->SetEngine(engine_);
  if (--waiting_for_threads_ == 0)
    Initialise();
}

void Library::Initialise() {
  // The backend and watcher threads are finished initialising, now we can
  // connect them together and start everything off.
  watcher_->Worker()->SetBackend(backend_->Worker());

  connect(backend_->Worker().get(), SIGNAL(DirectoriesDiscovered(DirectoryList)),
          watcher_->Worker().get(), SLOT(AddDirectories(DirectoryList)));
  connect(watcher_->Worker().get(), SIGNAL(NewOrUpdatedSongs(SongList)),
          backend_->Worker().get(), SLOT(AddOrUpdateSongs(SongList)));
  connect(watcher_->Worker().get(), SIGNAL(SongsMTimeUpdated(SongList)),
          backend_->Worker().get(), SLOT(UpdateMTimesOnly(SongList)));
  connect(watcher_->Worker().get(), SIGNAL(SongsDeleted(SongList)),
          backend_->Worker().get(), SLOT(DeleteSongs(SongList)));

  // This will start the watcher checking for updates
  backend_->Worker()->LoadDirectoriesAsync();

  backend_->Worker()->UpdateTotalSongCountAsync();

  Reset();
}

void Library::SongsDiscovered(const SongList& songs) {
  foreach (const Song& song, songs) {
    if (!query_options_.Matches(song))
      continue;

    LibraryItem* artist = NULL;
    LibraryItem* album = NULL;

    if (song.is_compilation()) {
      if (compilation_artist_node_ == NULL)
        CreateCompilationArtistNode(true);
      artist = compilation_artist_node_;
    } else {
      if (artist_nodes_.contains(song.artist()))
        artist = artist_nodes_[song.artist()];
      else {
        artist = CreateArtistNode(true, song.artist());
      }
    }

    if (artist->lazy_loaded) {
      album = artist->ChildByKey(song.album());
      if (album == NULL)
        album = CreateAlbumNode(true, song.album(), artist, song.is_compilation());

      if (album->lazy_loaded)
        CreateSongNode(true, song, album);
    }
  }
}

LibraryItem* Library::CreateCompilationArtistNode(bool signal) {
  LibraryItem* parent = root_;
  if (signal)
    beginInsertRows(ItemToIndex(parent), parent->children.count(), parent->children.count());

  compilation_artist_node_ =
      new LibraryItem(LibraryItem::Type_CompilationArtist, "Various Artists", parent);
  compilation_artist_node_->sort_text = " various";

  if (signal)
    endInsertRows();

  return compilation_artist_node_;
}

LibraryItem* Library::CreateArtistNode(bool signal, const QString& name) {
  LibraryItem* parent = root_;
  if (signal)
    beginInsertRows(ItemToIndex(parent), parent->children.count(), parent->children.count());

  LibraryItem* ret = new LibraryItem(LibraryItem::Type_Artist, name, parent);
  ret->display_text = PrettyArtist(name);
  ret->sort_text = SortTextForArtist(name);

  artist_nodes_[name] = ret;

  if (signal)
    endInsertRows();

  if (!name.isEmpty()) {
    QChar divider_char = ret->sort_text[0];

    if (!divider_nodes_.contains(divider_char)) {
      if (signal)
        beginInsertRows(ItemToIndex(parent), parent->children.count(), parent->children.count());

      LibraryItem* divider =
          new LibraryItem(LibraryItem::Type_Divider, QString(divider_char), root_);
      divider->lazy_loaded = true;

      if (divider_char.isDigit())
        divider->display_text = "0-9";

      divider_nodes_[divider_char] = divider;

      if (signal)
        endInsertRows();
    }
  }

  return ret;
}

LibraryItem* Library::CreateAlbumNode(bool signal, const QString& name, LibraryItem* parent, bool compilation) {
  if (signal)
    beginInsertRows(ItemToIndex(parent), parent->children.count(), parent->children.count());

  LibraryItem* ret = new LibraryItem(
      compilation ? LibraryItem::Type_CompilationAlbum
                  : LibraryItem::Type_Album,
      name, parent);

  ret->display_text = PrettyAlbum(name);
  ret->sort_text = SortTextForAlbum(name);

  if (signal)
    endInsertRows();

  return ret;
}

LibraryItem* Library::CreateSongNode(bool signal, const Song& song, LibraryItem* parent) {
  if (signal)
    beginInsertRows(ItemToIndex(parent), parent->children.count(), parent->children.count());

  LibraryItem* ret = new LibraryItem(LibraryItem::Type_Song, song.title(), parent);
  ret->lazy_loaded = true;
  ret->display_text = song.PrettyTitleWithArtist();
  ret->song = song;

  song_nodes_[song.id()] = ret;

  if (signal)
    endInsertRows();

  return ret;
}

void Library::SongsDeleted(const SongList& songs) {
  // Delete song nodes
  foreach (const Song& song, songs) {
    if (song_nodes_.contains(song.id())) {
      LibraryItem* node = song_nodes_[song.id()];

      beginRemoveRows(ItemToIndex(node->parent), node->row, node->row);
      node->parent->Delete(node->row);
      song_nodes_.remove(song.id());
      endRemoveRows();
    }
  }

  // Delete now-empty album nodes
  foreach (const Song& song, songs) {
    LibraryItem* artist = NULL;

    if (song.is_compilation() && compilation_artist_node_->lazy_loaded)
      artist = compilation_artist_node_;
    else if (artist_nodes_.contains(song.artist()) &&
             artist_nodes_[song.artist()]->lazy_loaded)
      artist = artist_nodes_[song.artist()];

    if (artist == NULL)
      continue;

    LibraryItem* node = artist->ChildByKey(song.album());
    if (!node)
      continue;

    LazyPopulate(node);

    if (node->children.count() == 0) {
      beginRemoveRows(ItemToIndex(node->parent), node->row, node->row);
      node->parent->Delete(node->row);
      endRemoveRows();
    }
  }

  // Delete now-empty artist nodes
  foreach (const Song& song, songs) {
    // Was it a compilation?
    LibraryItem* node = NULL;
    if (song.is_compilation())
      node = compilation_artist_node_;
    else
      node = artist_nodes_.contains(song.artist()) ? artist_nodes_[song.artist()] : NULL;

    if (node) {
      LazyPopulate(node);

      if (node->children.count() == 0) {
        beginRemoveRows(ItemToIndex(node->parent), node->row, node->row);
        node->parent->Delete(node->row);

        if (song.is_compilation())
          compilation_artist_node_ = NULL;
        else {
          artist_nodes_.remove(song.artist());

          // Delete now-empty dividers
          QString sort_text(SortTextForArtist(song.artist()));
          if (!sort_text.isEmpty() && divider_nodes_.contains(sort_text[0])) {
            QChar c(sort_text[0]);
            bool found = false;
            foreach (LibraryItem* artist_node, artist_nodes_.values()) {
              if (artist_node->sort_text.startsWith(c)) {
                found = true;
                break;
              }
            }

            if (!found) {
              root_->Delete(divider_nodes_[c]->row);
              divider_nodes_.remove(c);
            }
          }
        }

        endRemoveRows();
      }
    }
  }
}

QVariant Library::data(const QModelIndex& index, int role) const {
  const LibraryItem* item = IndexToItem(index);

  return data(item, role);
}

QVariant Library::data(const LibraryItem* item, int role) const {
  switch (role) {
    case Qt::DisplayRole:
      return item->DisplayText();

    case Qt::DecorationRole:
      switch (item->type) {
        case LibraryItem::Type_Album:
        case LibraryItem::Type_CompilationAlbum:
          return album_icon_;
        case LibraryItem::Type_Artist:
        case LibraryItem::Type_CompilationArtist:
          return artist_icon_;
        default:
          break;
      }
      break;

    case Role_Type:
      return item->type;

    case Role_Key:
      return item->key;

    case Role_SortText:
      if (item->type == LibraryItem::Type_Song)
        return item->song.disc() * 1000 + item->song.track();
      return item->SortText();
  }
  return QVariant();
}

void Library::LazyPopulate(LibraryItem* item) {
  if (item->lazy_loaded)
    return;

  switch (item->type) {
    case LibraryItem::Type_CompilationArtist:
      foreach (const QString& album, backend_->Worker()->GetCompilationAlbums(query_options_))
        CreateAlbumNode(false, album, item, true);
      break;

    case LibraryItem::Type_CompilationAlbum:
      foreach (const Song& song, backend_->Worker()->GetCompilationSongs(query_options_, item->key))
        CreateSongNode(false, song, item);
      break;

    case LibraryItem::Type_Artist:
      foreach (const QString& album, backend_->Worker()->GetAlbumsByArtist(query_options_, item->key))
        CreateAlbumNode(false, album, item, false);
      break;

    case LibraryItem::Type_Album:
      foreach (const Song& song, backend_->Worker()->GetSongs(query_options_, item->parent->key, item->key))
        CreateSongNode(false, song, item);
      break;

    default:
      qWarning("Tried to LazyPopulate a bad item type");
      break;
  }

  item->lazy_loaded = true;
}

void Library::Reset() {
  delete root_;
  artist_nodes_.clear();
  song_nodes_.clear();
  divider_nodes_.clear();
  compilation_artist_node_ = NULL;

  root_ = new LibraryItem(this);
  root_->lazy_loaded = true;

  // Various artists?
  if (backend_->Worker()->HasCompilations(query_options_))
    CreateCompilationArtistNode(false);

  // Populate artists
  foreach (const QString& artist, backend_->Worker()->GetAllArtists(query_options_))
    CreateArtistNode(false, artist);

  reset();
}

QString Library::PrettyArtist(QString artist) const {
  if (artist.isEmpty()) {
    artist = "Unknown";
  }

  return artist;
}

QString Library::SortTextForArtist(QString artist) const {
  artist = SortTextForAlbum(artist);

  if (artist.startsWith("the ")) {
    artist = artist.right(artist.length() - 4) + ", the";
  }

  return artist;
}

QString Library::PrettyAlbum(QString album) const {
  if (album.isEmpty()) {
    album = "Unknown";
  }

  return album;
}

QString Library::SortTextForAlbum(QString album) const {
  if (album.isEmpty()) {
    album = " unknown";
  } else {
    album = album.toLower();
  }
  album = album.remove(QRegExp("[^\\w ]"));

  return album;
}

Qt::ItemFlags Library::flags(const QModelIndex& index) const {
  switch (IndexToItem(index)->type) {
  case LibraryItem::Type_Album:
  case LibraryItem::Type_Artist:
  case LibraryItem::Type_Song:
  case LibraryItem::Type_CompilationAlbum:
  case LibraryItem::Type_CompilationArtist:
    return Qt::ItemIsSelectable |
           Qt::ItemIsEnabled |
           Qt::ItemIsDragEnabled;
  case LibraryItem::Type_Divider:
  case LibraryItem::Type_Root:
  default:
    return Qt::ItemIsEnabled;
  }
}

QStringList Library::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

QMimeData* Library::mimeData(const QModelIndexList& indexes) const {
  SongMimeData* data = new SongMimeData;
  QList<QUrl> urls;

  foreach (const QModelIndex& index, indexes) {
    GetChildSongs(IndexToItem(index), &urls, &data->songs);
  }

  data->setUrls(urls);

  return data;
}

bool Library::CompareItems(const LibraryItem* a, const LibraryItem* b) const {
  QVariant left(data(a, Library::Role_SortText));
  QVariant right(data(b, Library::Role_SortText));

  if (left.type() == QVariant::Int)
    return left.toInt() < right.toInt();
  return left.toString() < right.toString();
}

void Library::GetChildSongs(LibraryItem* item, QList<QUrl>* urls, SongList* songs) const {
  switch (item->type) {
    case LibraryItem::Type_Album:
    case LibraryItem::Type_Artist:
    case LibraryItem::Type_CompilationAlbum:
    case LibraryItem::Type_CompilationArtist: {
      const_cast<Library*>(this)->LazyPopulate(item);

      QList<LibraryItem*> children = item->children;
      qSort(children.begin(), children.end(), boost::bind(
          &Library::CompareItems, this, _1, _2));

      foreach (LibraryItem* child, children)
        GetChildSongs(child, urls, songs);
      break;
    }

    case LibraryItem::Type_Song:
      urls->append(QUrl::fromLocalFile(item->song.filename()));
      songs->append(item->song);
      break;

    default:
      break;
  }
}

SongList Library::GetChildSongs(const QModelIndex& index) const {
  QList<QUrl> dontcare;
  SongList ret;

  if (!index.isValid())
    return SongList();

  GetChildSongs(IndexToItem(index), &dontcare, &ret);
  return ret;
}

void Library::ShowConfig() {
  config_->show();
}

void Library::SetFilterAge(int age) {
  query_options_.max_age = age;
  Reset();
}

void Library::SetFilterText(const QString& text) {
  query_options_.filter = text;
  Reset();
}

bool Library::canFetchMore(const QModelIndex &parent) const {
  if (!parent.isValid())
    return false;

  LibraryItem* item = IndexToItem(parent);
  return !item->lazy_loaded;
}
