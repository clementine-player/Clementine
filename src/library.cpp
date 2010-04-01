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

#include "library.h"
#include "librarybackend.h"
#include "libraryitem.h"
#include "songmimedata.h"
#include "librarydirectorymodel.h"

#include <QStringList>
#include <QUrl>
#include <QMetaEnum>

#include <boost/bind.hpp>


Library::Library(EngineBase* engine, QObject* parent)
  : SimpleTreeModel<LibraryItem>(new LibraryItem(this), parent),
    engine_(engine),
    backend_factory_(new BackgroundThreadFactoryImplementation<LibraryBackendInterface, LibraryBackend>),
    watcher_factory_(new BackgroundThreadFactoryImplementation<LibraryWatcher, LibraryWatcher>),
    backend_(NULL),
    watcher_(NULL),
    dir_model_(new LibraryDirectoryModel(this)),
    waiting_for_threads_(2),
    artist_icon_(":artist.png"),
    album_icon_(":album.png"),
    no_cover_icon_(":nocover.png")
{
  root_->lazy_loaded = true;

  group_by_[0] = GroupBy_Artist;
  group_by_[1] = GroupBy_Album;
  group_by_[2] = GroupBy_None;
}

Library::~Library() {
  delete root_;
}

void Library::set_backend_factory(BackgroundThreadFactory<LibraryBackendInterface>* factory) {
  backend_factory_.reset(factory);
}

void Library::set_watcher_factory(BackgroundThreadFactory<LibraryWatcher>* factory) {
  watcher_factory_.reset(factory);
}

void Library::Init() {
  backend_ = backend_factory_->GetThread(this);
  watcher_ = watcher_factory_->GetThread(this);

  connect(backend_, SIGNAL(Initialised()), SLOT(BackendInitialised()));
  connect(watcher_, SIGNAL(Initialised()), SLOT(WatcherInitialised()));
}

void Library::StartThreads() {
  Q_ASSERT(waiting_for_threads_);
  Q_ASSERT(backend_);
  Q_ASSERT(watcher_);

  backend_->Start();

  watcher_->set_io_priority(BackgroundThreadBase::IOPRIO_CLASS_IDLE);
  watcher_->set_cpu_priority(QThread::IdlePriority);
  watcher_->Start();
}

void Library::BackendInitialised() {
  connect(backend_->Worker().get(), SIGNAL(SongsDiscovered(SongList)), SLOT(SongsDiscovered(SongList)));
  connect(backend_->Worker().get(), SIGNAL(SongsDeleted(SongList)), SLOT(SongsDeleted(SongList)));
  connect(backend_->Worker().get(), SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(backend_->Worker().get(), SIGNAL(TotalSongCountUpdated(int)), SIGNAL(TotalSongCountUpdated(int)));

  dir_model_->SetBackend(backend_->Worker());

  emit BackendReady(backend_->Worker());

  if (--waiting_for_threads_ == 0)
    Initialise();
}

void Library::WatcherInitialised() {
  connect(watcher_->Worker().get(), SIGNAL(ScanStarted()), SIGNAL(ScanStarted()));
  connect(watcher_->Worker().get(), SIGNAL(ScanFinished()), SIGNAL(ScanFinished()));

  watcher_->Worker()->SetEngine(engine_);

  if (--waiting_for_threads_ == 0)
    Initialise();
}

void Library::Initialise() {
  // The backend and watcher threads are finished initialising, now we can
  // connect them together and start everything off.
  watcher_->Worker()->SetBackend(backend_->Worker());

  connect(backend_->Worker().get(), SIGNAL(DirectoryDiscovered(Directory,SubdirectoryList)),
          watcher_->Worker().get(), SLOT(AddDirectory(Directory,SubdirectoryList)));
  connect(backend_->Worker().get(), SIGNAL(DirectoryDeleted(Directory)),
          watcher_->Worker().get(), SLOT(RemoveDirectory(Directory)));
  connect(watcher_->Worker().get(), SIGNAL(NewOrUpdatedSongs(SongList)),
          backend_->Worker().get(), SLOT(AddOrUpdateSongs(SongList)));
  connect(watcher_->Worker().get(), SIGNAL(SongsMTimeUpdated(SongList)),
          backend_->Worker().get(), SLOT(UpdateMTimesOnly(SongList)));
  connect(watcher_->Worker().get(), SIGNAL(SongsDeleted(SongList)),
          backend_->Worker().get(), SLOT(DeleteSongs(SongList)));
  connect(watcher_->Worker().get(), SIGNAL(SubdirsDiscovered(SubdirectoryList)),
          backend_->Worker().get(), SLOT(AddSubdirs(SubdirectoryList)));
  connect(watcher_->Worker().get(), SIGNAL(SubdirsMTimeUpdated(SubdirectoryList)),
          backend_->Worker().get(), SLOT(UpdateSubdirMTimes(SubdirectoryList)));

  // This will start the watcher checking for updates
  backend_->Worker()->LoadDirectoriesAsync();

  backend_->Worker()->UpdateTotalSongCountAsync();

  Reset();
}

void Library::SongsDiscovered(const SongList& songs) {
  foreach (const Song& song, songs) {
    // Sanity check to make sure we don't add songs that are outside the user's
    // filter
    if (!query_options_.Matches(song))
      continue;

    // Hey, we've already got that one!
    if (song_nodes_.contains(song.id()))
      continue;

    // Before we can add each song we need to make sure the required container
    // items already exist in the tree.  These depend on which "group by"
    // settings the user has on the library.  Eg. if the user grouped by
    // artist and album, we would need to make sure nodes for the song's artist
    // and album were already in the tree.

    // Find parent containers in the tree
    LibraryItem* container = root_;
    for (int i=0 ; i<3 ; ++i) {
      GroupBy type = group_by_[i];
      if (type == GroupBy_None) break;

      // Special case: if we're at the top level and the song is a compilation
      // and the top level is Artists, then we want the Various Artists node :(
      if (i == 0 && type == GroupBy_Artist && song.is_compilation()) {
        if (compilation_artist_node_ == NULL)
          CreateCompilationArtistNode(true, root_);
        container = compilation_artist_node_;
      } else {
        // Otherwise find the proper container at this level based on the
        // item's key
        QString key;
        switch (type) {
          case GroupBy_Album:    key = song.album(); break;
          case GroupBy_Artist:   key = song.artist(); break;
          case GroupBy_Composer: key = song.composer(); break;
          case GroupBy_Genre:    key = song.genre(); break;
          case GroupBy_Year:
            key = QString::number(qMax(0, song.year())); break;
          case GroupBy_YearAlbum:
            key = PrettyYearAlbum(qMax(0, song.year()), song.album()); break;
          case GroupBy_None: Q_ASSERT(0); break;
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
      if (!container->lazy_loaded)
        break;
    }

    if (!container->lazy_loaded)
      continue;

    // We've gone all the way down to the deepest level and everything was
    // already lazy loaded, so now we have to create the song in the container.
    song_nodes_[song.id()] =
        ItemFromSong(GroupBy_None, true, false, container, song, -1);
  }
}

LibraryItem* Library::CreateCompilationArtistNode(bool signal, LibraryItem* parent) {
  if (signal)
    beginInsertRows(ItemToIndex(parent), parent->children.count(), parent->children.count());

  compilation_artist_node_ =
      new LibraryItem(LibraryItem::Type_Container, parent);
  compilation_artist_node_->key = tr("Various Artists");
  compilation_artist_node_->sort_text = " various";
  compilation_artist_node_->container_level = parent->container_level + 1;

  if (signal)
    endInsertRows();

  return compilation_artist_node_;
}

QString Library::DividerKey(GroupBy type, LibraryItem* item) const {
  // Items which are to be grouped under the same divider must produce the
  // same divider key.  This will only get called for top-level items.

  if (item->sort_text.isEmpty())
    return QString();

  switch (type) {
  case GroupBy_Album:
  case GroupBy_Artist:
  case GroupBy_Composer:
  case GroupBy_Genre:
    if (item->sort_text[0].isDigit())
      return "0";
    if (item->sort_text[0] == ' ')
      return QString();
    return QString(item->sort_text[0]);

  case GroupBy_Year:
    return SortTextForYear(item->sort_text.toInt() / 10 * 10);

  case GroupBy_YearAlbum:
    return QString::number(item->metadata.year());

  case GroupBy_None:
  default:
    Q_ASSERT(0);
    return QString();
  }
}

QString Library::DividerDisplayText(GroupBy type, const QString& key) const {
  // Pretty display text for the dividers.

  switch (type) {
  case GroupBy_Album:
  case GroupBy_Artist:
  case GroupBy_Composer:
  case GroupBy_Genre:
    if (key == "0")
      return "0-9";
    // fallthrough

  case GroupBy_YearAlbum:
    return key;

  case GroupBy_Year:
    return QString::number(key.toInt()); // To remove leading 0s

  case GroupBy_None:
  default:
    Q_ASSERT(0);
    return QString();
  }
}

void Library::SongsDeleted(const SongList& songs) {
  // Delete the actual song nodes first, keeping track of each parent so we
  // might check to see if they're empty later.
  QSet<LibraryItem*> parents;
  foreach (const Song& song, songs) {
    if (song_nodes_.contains(song.id())) {
      LibraryItem* node = song_nodes_[song.id()];

      if (node->parent != root_)
        parents << node->parent;

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
    foreach (LibraryItem* node, parents) {
      parents.remove(node);
      if (node->children.count() != 0)
        continue;

      // Consider its parent for the next round
      if (node->parent != root_)
        parents << node->parent;

      // Maybe consider its divider node
      if (node->container_level == 0)
        divider_keys << DividerKey(group_by_[0], node);

      // Special case the Various Artists node
      if (node == compilation_artist_node_)
        compilation_artist_node_ = NULL;
      else
        container_nodes_[node->container_level].remove(node->key);

      // It was empty - delete it
      beginRemoveRows(ItemToIndex(node->parent), node->row, node->row);
      node->parent->Delete(node->row);
      endRemoveRows();
    }
  }

  // Delete empty dividers
  foreach (const QString& divider_key, divider_keys) {
    if (!divider_nodes_.contains(divider_key))
      continue;

    // Look to see if there are any other items still under this divider
    bool found = false;
    foreach (LibraryItem* node, container_nodes_[0].values()) {
      if (DividerKey(group_by_[0], node) == divider_key) {
        found = true;
        break;
      }
    }

    if (found)
      continue;

    // Remove the divider
    int row = divider_nodes_[divider_key]->row;
    beginRemoveRows(ItemToIndex(root_), row, row);
    root_->Delete(row);
    endRemoveRows();
    divider_nodes_.remove(divider_key);
  }
}

QVariant Library::data(const QModelIndex& index, int role) const {
  const LibraryItem* item = IndexToItem(index);

  return data(item, role);
}

QVariant Library::data(const LibraryItem* item, int role) const {
  GroupBy container_type =
      item->type == LibraryItem::Type_Container ?
      group_by_[item->container_level] : GroupBy_None;

  switch (role) {
    case Qt::DisplayRole:
      return item->DisplayText();

    case Qt::DecorationRole:
      switch (item->type)
        case LibraryItem::Type_Container:
          switch (container_type) {
            case GroupBy_Album:
              return album_icon_;
            case GroupBy_Artist:
              return artist_icon_;
            default:
              break;
          }
        default:
          break;
      break;

    case Role_Type:
      return item->type;

    case Role_ContainerType:
      return container_type;

    case Role_Key:
      return item->key;

    case Role_Artist:
      return item->metadata.artist();

    case Role_SortText:
      if (item->type == LibraryItem::Type_Song)
        return item->metadata.disc() * 1000 + item->metadata.track();
      return item->SortText();
  }
  return QVariant();
}

void Library::LazyPopulate(LibraryItem* parent, bool signal) {
  if (parent->lazy_loaded)
    return;
  parent->lazy_loaded = true;

  // Information about what we want the children to be
  int child_level = parent->container_level + 1;
  GroupBy child_type = child_level >= 3 ? GroupBy_None : group_by_[child_level];

  // Initialise the query.  child_type says what type of thing we want (artists,
  // songs, etc.)
  LibraryQuery q(query_options_);
  InitQuery(child_type, &q);

  // Top-level artists is special - we don't want compilation albums appearing
  if (child_level == 0 && child_type == GroupBy_Artist) {
    q.AddCompilationRequirement(false);
  }

  // Walk up through the item's parents adding filters as necessary
  LibraryItem* p = parent;
  while (p != root_) {
    FilterQuery(group_by_[p->container_level], p, &q);
    p = p->parent;
  }

  // Execute the query
  if (!backend_->Worker()->ExecQuery(&q))
    return;

  // Step through the results
  while (q.Next()) {
    // Create the item - it will get inserted into the model here
    LibraryItem* item =
        ItemFromQuery(child_type, signal, child_level == 0, parent, q, child_level);

    // Save a pointer to it for later
    if (child_type == GroupBy_None)
      song_nodes_[item->metadata.id()] = item;
    else
      container_nodes_[child_level][item->key] = item;
  }
}

void Library::Reset() {
  delete root_;
  song_nodes_.clear();
  container_nodes_[0].clear();
  container_nodes_[1].clear();
  container_nodes_[2].clear();
  divider_nodes_.clear();
  compilation_artist_node_ = NULL;

  root_ = new LibraryItem(this);
  root_->lazy_loaded = false;

  // Various artists?
  if (group_by_[0] == GroupBy_Artist &&
      backend_->Worker()->HasCompilations(query_options_))
    CreateCompilationArtistNode(false, root_);

  // Populate top level
  LazyPopulate(root_, false);

  reset();
}

void Library::InitQuery(GroupBy type, LibraryQuery* q) {
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
  case GroupBy_YearAlbum:
    q->SetColumnSpec("DISTINCT year, album");
    break;
  case GroupBy_Year:
    q->SetColumnSpec("DISTINCT year");
    break;
  case GroupBy_Genre:
    q->SetColumnSpec("DISTINCT genre");
    break;
  case GroupBy_None:
    q->SetColumnSpec("ROWID, " + QString(Song::kColumnSpec));
    break;
  }
}

void Library::FilterQuery(GroupBy type, LibraryItem* item, LibraryQuery* q) {
  // Say how we want the query to be filtered.  This is done once for each
  // parent going up the tree.

  switch (type) {
  case GroupBy_Artist:
    if (item == compilation_artist_node_)
      q->AddCompilationRequirement(true);
    else {
      if (item->container_level == 0) // Stupid hack
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
    break;
  case GroupBy_Year:
    q->AddWhere("year", item->key);
    break;
  case GroupBy_Composer:
    q->AddWhere("composer", item->key);
    break;
  case GroupBy_Genre:
    q->AddWhere("genre", item->key);
    break;
  case GroupBy_None:
    Q_ASSERT(0);
    break;
  }
}

LibraryItem* Library::InitItem(GroupBy type, bool signal, LibraryItem *parent,
                               int container_level) {
  LibraryItem::Type item_type =
      type == GroupBy_None ? LibraryItem::Type_Song :
      LibraryItem::Type_Container;

  if (signal)
    beginInsertRows(ItemToIndex(parent),
                    parent->children.count(),parent->children.count());

  // Initialise the item depending on what type it's meant to be
  LibraryItem* item = new LibraryItem(item_type, parent);
  item->container_level = container_level;
  return item;
}

LibraryItem* Library::ItemFromQuery(GroupBy type,
                                    bool signal, bool create_divider,
                                    LibraryItem* parent, const LibraryQuery& q,
                                    int container_level) {
  LibraryItem* item = InitItem(type, signal, parent, container_level);
  int year = 0;

  switch (type) {
  case GroupBy_Artist:
    item->key = q.Value(0).toString();
    item->display_text = TextOrUnknown(item->key);
    item->sort_text = SortTextForArtist(item->key);
    break;

  case GroupBy_YearAlbum:
    year = qMax(0, q.Value(0).toInt());
    item->metadata.set_year(year);
    item->metadata.set_album(q.Value(1).toString());
    item->key = PrettyYearAlbum(year, item->metadata.album());
    item->sort_text = SortTextForYear(year) + item->metadata.album();
    break;

  case GroupBy_Year:
    year = qMax(0, q.Value(0).toInt());
    item->key = QString::number(year);
    item->sort_text = SortTextForYear(year) + " ";
    break;

  case GroupBy_Composer:
  case GroupBy_Genre:
  case GroupBy_Album:
    item->key = q.Value(0).toString();
    item->display_text = TextOrUnknown(item->key);
    item->sort_text = SortText(item->key);
    break;

  case GroupBy_None:
    item->metadata.InitFromQuery(q);
    item->key = item->metadata.title();
    item->display_text = item->metadata.PrettyTitleWithArtist();
    break;
  }

  FinishItem(type, signal, create_divider, parent, item);
  return item;
}

LibraryItem* Library::ItemFromSong(GroupBy type,
                                   bool signal, bool create_divider,
                                   LibraryItem* parent, const Song& s,
                                   int container_level) {
  LibraryItem* item = InitItem(type, signal, parent, container_level);
  int year = 0;

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
    item->sort_text = SortTextForYear(year) + s.album();
    break;

  case GroupBy_Year:
    year = qMax(0, s.year());
    item->key = QString::number(year);
    item->sort_text = SortTextForYear(year) + " ";
    break;

  case GroupBy_Composer:                      item->key = s.composer();
  case GroupBy_Genre: if (item->key.isNull()) item->key = s.genre();
  case GroupBy_Album: if (item->key.isNull()) item->key = s.album();
    item->display_text = TextOrUnknown(item->key);
    item->sort_text = SortText(item->key);
    break;

  case GroupBy_None:
    item->metadata = s;
    item->key = s.title();
    item->display_text = s.PrettyTitleWithArtist();
    break;
  }

  FinishItem(type, signal, create_divider, parent, item);
  return item;
}

void Library::FinishItem(GroupBy type,
                         bool signal, bool create_divider,
                         LibraryItem *parent, LibraryItem *item) {
  if (type == GroupBy_None)
    item->lazy_loaded = true;

  if (signal)
    endInsertRows();

  // Create the divider entry if we're supposed to
  if (create_divider) {
    QString divider_key = DividerKey(type, item);

    if (!divider_key.isEmpty() && !divider_nodes_.contains(divider_key)) {
      if (signal)
        beginInsertRows(ItemToIndex(parent), parent->children.count(),
                        parent->children.count());

      LibraryItem* divider =
          new LibraryItem(LibraryItem::Type_Divider, root_);
      divider->key = divider_key;
      divider->display_text = DividerDisplayText(type, divider_key);
      divider->lazy_loaded = true;

      divider_nodes_[divider_key] = divider;

      if (signal)
        endInsertRows();
    }
  }
}

QString Library::TextOrUnknown(const QString& text) const {
  if (text.isEmpty()) {
    return tr("Unknown");
  }
  return text;
}

QString Library::PrettyYearAlbum(int year, const QString& album) const {
  return QString::number(year) + " - " + TextOrUnknown(album);
}

QString Library::SortText(QString text) const {
  if (text.isEmpty()) {
    text = " unknown";
  } else {
    text = text.toLower();
  }
  text = text.remove(QRegExp("[^\\w ]"));

  return text;
}

QString Library::SortTextForArtist(QString artist) const {
  artist = SortText(artist);

  if (artist.startsWith("the ")) {
    artist = artist.right(artist.length() - 4) + ", the";
  }

  return artist;
}

QString Library::SortTextForYear(int year) const {
  QString str = QString::number(year);
  return QString("0").repeated(qMax(0, 4 - str.length())) + str;
}

Qt::ItemFlags Library::flags(const QModelIndex& index) const {
  switch (IndexToItem(index)->type) {
  case LibraryItem::Type_Song:
  case LibraryItem::Type_Container:
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
    case LibraryItem::Type_Container: {
      const_cast<Library*>(this)->LazyPopulate(item);

      QList<LibraryItem*> children = item->children;
      qSort(children.begin(), children.end(), boost::bind(
          &Library::CompareItems, this, _1, _2));

      foreach (LibraryItem* child, children)
        GetChildSongs(child, urls, songs);
      break;
    }

    case LibraryItem::Type_Song:
      urls->append(QUrl::fromLocalFile(item->metadata.filename()));
      songs->append(item->metadata);
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

void Library::SetGroupBy(const Grouping& g) {
  group_by_ = g;

  if (!waiting_for_threads_)
    Reset();

  emit GroupingChanged(g);
}

const Library::GroupBy& Library::Grouping::operator [](int i) const {
  switch (i) {
    case 0: return first;
    case 1: return second;
    case 2: return third;
  }
  Q_ASSERT(0);
  return first;
}

Library::GroupBy& Library::Grouping::operator [](int i) {
  switch (i) {
    case 0: return first;
    case 1: return second;
    case 2: return third;
  }
  Q_ASSERT(0);
  return first;
}
