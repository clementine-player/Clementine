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

#include "jamendoservice.h"

#include <QDesktopServices>
#include <QFutureWatcher>
#include <QMenu>
#include <QNetworkReply>
#include <QSortFilterProxyModel>
#include <QtConcurrentRun>
#include <QXmlStreamReader>
#include "qtiocompressor.h"

#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/scopedtransaction.h"
#include "core/taskmanager.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "library/librarymodel.h"
#include "radio/radiomodel.h"
#include "radio/jamendodynamicplaylist.h"
#include "radio/jamendoplaylistitem.h"
#include "smartplaylists/generator.h"
#include "smartplaylists/querygenerator.h"
#include "ui/iconloader.h"

const char* JamendoService::kServiceName = "Jamendo";
const char* JamendoService::kDirectoryUrl =
    "http://img.jamendo.com/data/dbdump_artistalbumtrack.xml.gz";
const char* JamendoService::kMp3StreamUrl =
    "http://api.jamendo.com/get2/stream/track/redirect/?id=%1&streamencoding=mp31";
const char* JamendoService::kOggStreamUrl =
    "http://api.jamendo.com/get2/stream/track/redirect/?id=%1&streamencoding=ogg2";
const char* JamendoService::kAlbumCoverUrl =
    "http://api.jamendo.com/get2/image/album/redirect/?id=%1&imagesize=300";
const char* JamendoService::kHomepage = "http://www.jamendo.com/";
const char* JamendoService::kAlbumInfoUrl = "http://www.jamendo.com/album/%1";
const char* JamendoService::kDownloadAlbumUrl = "http://www.jamendo.com/download/album/%1";

const char* JamendoService::kSongsTable = "jamendo.songs";
const char* JamendoService::kFtsTable = "jamendo.songs_fts";
const char* JamendoService::kTrackIdsTable = "jamendo.track_ids";
const char* JamendoService::kTrackIdsColumn = "track_id";

const char* JamendoService::kSettingsGroup = "Jamendo";

const int JamendoService::kBatchSize = 10000;
const int JamendoService::kApproxDatabaseSize = 300000;

JamendoService::JamendoService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      network_(new NetworkAccessManager(this)),
      context_menu_(NULL),
      library_backend_(NULL),
      library_filter_(NULL),
      library_model_(NULL),
      library_sort_model_(new QSortFilterProxyModel(this)),
      load_database_task_id_(0),
      total_song_count_(0) {
  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(parent->db_thread());
  library_backend_->Init(parent->db_thread()->Worker(), kSongsTable,
                         QString::null, QString::null, kFtsTable);
  connect(library_backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(UpdateTotalSongCount(int)));

  using smart_playlists::Generator;
  using smart_playlists::GeneratorPtr;
  using smart_playlists::QueryGenerator;
  using smart_playlists::Search;
  using smart_playlists::SearchTerm;

  library_model_ = new LibraryModel(library_backend_, this);
  library_model_->set_show_various_artists(false);
  library_model_->set_show_smart_playlists(true);
  library_model_->set_default_smart_playlists(LibraryModel::DefaultGenerators()
    << (LibraryModel::GeneratorList()
      << GeneratorPtr(new JamendoDynamicPlaylist(tr("Jamendo Top Tracks of the Month"),
                      JamendoDynamicPlaylist::OrderBy_RatingMonth))
      << GeneratorPtr(new JamendoDynamicPlaylist(tr("Jamendo Top Tracks of the Week"),
                      JamendoDynamicPlaylist::OrderBy_RatingWeek))
      << GeneratorPtr(new JamendoDynamicPlaylist(tr("Jamendo Top Tracks"),
                      JamendoDynamicPlaylist::OrderBy_Rating))
      << GeneratorPtr(new JamendoDynamicPlaylist(tr("Jamendo Most Listened Tracks"),
                      JamendoDynamicPlaylist::OrderBy_Listened))
    )
    << (LibraryModel::GeneratorList()
      << GeneratorPtr(new QueryGenerator(tr("Dynamic random mix"), Search(
                      Search::Type_All, Search::TermList(),
                      Search::Sort_Random, SearchTerm::Field_Title), true))
    )
  );

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);
}

JamendoService::~JamendoService() {
}

QStandardItem* JamendoService::CreateRootItem() {
  QStandardItem* item = new QStandardItem(QIcon(":providers/jamendo.png"), kServiceName);
  item->setData(true, RadioModel::Role_CanLazyLoad);
  return item;
}

void JamendoService::LazyPopulate(QStandardItem* item) {
  switch (item->data(RadioModel::Role_Type).toInt()) {
    case RadioModel::Type_Service: {
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;
    }
    default:
      break;
  }
}

void JamendoService::UpdateTotalSongCount(int count) {
  total_song_count_ = count;
  if (total_song_count_ == 0 && !load_database_task_id_) {
    DownloadDirectory();
  }
}

void JamendoService::DownloadDirectory() {
  QNetworkRequest req = QNetworkRequest(QUrl(kDirectoryUrl));
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);

  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(DownloadDirectoryFinished()));
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                 SLOT(DownloadDirectoryProgress(qint64,qint64)));

  if (!load_database_task_id_) {
    load_database_task_id_ = model()->task_manager()->StartTask(
        tr("Downloading Jamendo catalogue"));
  }
}

void JamendoService::DownloadDirectoryProgress(qint64 received, qint64 total) {
  float progress = float(received) / total;
  model()->task_manager()->SetTaskProgress(load_database_task_id_,
                                           int(progress * 100), 100);
}

void JamendoService::DownloadDirectoryFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);

  model()->task_manager()->SetTaskFinished(load_database_task_id_);
  load_database_task_id_ = 0;

  // TODO: Not leak reply.
  QtIOCompressor* gzip = new QtIOCompressor(reply);
  gzip->setStreamFormat(QtIOCompressor::GzipFormat);
  if (!gzip->open(QIODevice::ReadOnly)) {
    qWarning() << "Jamendo library not in gzip format";
    delete gzip;
    return;
  }

  load_database_task_id_ = model()->task_manager()->StartTask(
      tr("Parsing Jamendo catalogue"));

  QFuture<void> future = QtConcurrent::run(
      this, &JamendoService::ParseDirectory, gzip);
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>();
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(ParseDirectoryFinished()));
}

void JamendoService::ParseDirectory(QIODevice* device) const {
  int total_count = 0;

  // Bit of a hack: don't update the model while we're parsing the xml
  disconnect(library_backend_, SIGNAL(SongsDiscovered(SongList)),
             library_model_, SLOT(SongsDiscovered(SongList)));
  disconnect(library_backend_, SIGNAL(TotalSongCountUpdated(int)),
             this, SLOT(UpdateTotalSongCount(int)));

  // Delete the database and recreate it.  This is faster than dropping tables
  // or removing rows.
  library_backend_->db()->RecreateAttachedDb("jamendo");

  TrackIdList track_ids;
  SongList songs;
  QXmlStreamReader reader(device);
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "artist") {
      songs << ReadArtist(&reader, &track_ids);
    }

    if (songs.count() >= kBatchSize) {
      // Add the songs to the database in batches
      library_backend_->AddOrUpdateSongs(songs);
      InsertTrackIds(track_ids);

      total_count += songs.count();
      songs.clear();
      track_ids.clear();

      // Update progress info
      model()->task_manager()->SetTaskProgress(
            load_database_task_id_, total_count, kApproxDatabaseSize);
    }
  }

  library_backend_->AddOrUpdateSongs(songs);
  InsertTrackIds(track_ids);

  connect(library_backend_, SIGNAL(SongsDiscovered(SongList)),
          library_model_, SLOT(SongsDiscovered(SongList)));
  connect(library_backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(UpdateTotalSongCount(int)));

  library_backend_->UpdateTotalSongCount();
}

void JamendoService::InsertTrackIds(const TrackIdList& ids) const {
  QMutexLocker l(library_backend_->db()->Mutex());
  QSqlDatabase db(library_backend_->db()->Connect());

  ScopedTransaction t(&db);

  QSqlQuery insert(QString("INSERT INTO %1 (%2) VALUES (:id)")
                   .arg(kTrackIdsTable, kTrackIdsColumn), db);

  foreach (int id, ids) {
    insert.bindValue(":id", id);
    if (!insert.exec()) {
      qWarning() << "Query failed" << insert.lastQuery();
    }
  }

  t.Commit();
}

SongList JamendoService::ReadArtist(QXmlStreamReader* reader,
                                    TrackIdList* track_ids) const {
  SongList ret;
  QString current_artist;

  while (!reader->atEnd()) {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      if (name == "name") {
        current_artist = reader->readElementText().trimmed();
      } else if (name == "album") {
        ret << ReadAlbum(current_artist, reader, track_ids);
      }
    } else if (reader->isEndElement() && reader->name() == "artist") {
      break;
    }
  }

  return ret;
}

SongList JamendoService::ReadAlbum(
    const QString& artist, QXmlStreamReader* reader, TrackIdList* track_ids) const {
  SongList ret;
  QString current_album;
  QString cover;
  int current_album_id = 0;

  while (!reader->atEnd()) {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      if (reader->name() == "name") {
        current_album = reader->readElementText().trimmed();
      } else if (reader->name() == "id") {
        QString id = reader->readElementText();
        cover = QString(kAlbumCoverUrl).arg(id);
        current_album_id = id.toInt();
      } else if (reader->name() == "track") {
        ret << ReadTrack(artist, current_album, cover, current_album_id,
                         reader, track_ids);
      }
    } else if (reader->isEndElement() && reader->name() == "album") {
      break;
    }
  }
  return ret;
}

Song JamendoService::ReadTrack(const QString& artist,
                               const QString& album,
                               const QString& album_cover,
                               int album_id,
                               QXmlStreamReader* reader,
                               TrackIdList* track_ids) const {
  Song song;
  song.set_artist(artist);
  song.set_album(album);
  song.set_filetype(Song::Type_Stream);
  song.set_directory_id(0);
  song.set_mtime(0);
  song.set_ctime(0);
  song.set_filesize(0);

  // Shoehorn the album ID into the comment field
  song.set_comment(QString::number(album_id));

  while (!reader->atEnd()) {
    reader->readNext();
    if (reader->isStartElement()) {
      QStringRef name = reader->name();
      if (name == "name") {
        song.set_title(reader->readElementText().trimmed());
      } else if (name == "duration") {
        const int length = reader->readElementText().toFloat();
        song.set_length(length);
      } else if (name == "id3genre") {
        int genre_id = reader->readElementText().toInt();
        // In theory, genre 0 is "blues"; in practice it's invalid.
        if (genre_id != 0) {
          song.set_genre(genre_id);
        }
      } else if (name == "id") {
        QString id_text = reader->readElementText();
        int id = id_text.toInt();
        if (id == 0)
          continue;

        QString ogg_url = QString(kOggStreamUrl).arg(id_text);
        song.set_filename(ogg_url);
        song.set_art_automatic(album_cover);
        song.set_valid(true);

        // Rely on songs getting added in this exact order
        track_ids->append(id);
      }
    } else if (reader->isEndElement() && reader->name() == "track") {
      break;
    }
  }
  return song;
}

void JamendoService::ParseDirectoryFinished() {
  QFutureWatcher<void>* watcher = static_cast<QFutureWatcher<void>*>(sender());
  delete watcher;

  library_model_->Reset();

  model()->task_manager()->SetTaskFinished(load_database_task_id_);
  load_database_task_id_ = 0;
}

void JamendoService::EnsureMenuCreated() {
  if (library_filter_)
    return;

  context_menu_ = new QMenu;
  add_to_playlist_ = context_menu_->addAction(IconLoader::Load("media-playback-start"),
      tr("Append to current playlist"), this, SLOT(AddToPlaylist()));
  load_to_playlist_ = context_menu_->addAction(IconLoader::Load("media-playback-start"),
      tr("Replace current playlist"), this, SLOT(LoadToPlaylist()));
  open_in_new_playlist_ = context_menu_->addAction(IconLoader::Load("document-new"),
      tr("Open in new playlist"), this, SLOT(OpenInNewPlaylist()));
  context_menu_->addSeparator();
  album_info_ = context_menu_->addAction(IconLoader::Load("view-media-lyrics"),
      tr("Album info on jamendo.com..."), this, SLOT(AlbumInfo()));
  download_album_ = context_menu_->addAction(IconLoader::Load("download"),
      tr("Download this album..."), this, SLOT(DownloadAlbum()));
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("download"), tr("Open jamendo.com in browser"), this, SLOT(Homepage()));
  context_menu_->addAction(IconLoader::Load("view-refresh"), tr("Refresh catalogue"), this, SLOT(DownloadDirectory()));

  library_filter_ = new LibraryFilterWidget(0);
  library_filter_->SetSettingsGroup(kSettingsGroup);
  library_filter_->SetLibraryModel(library_model_);
  library_filter_->SetFilterHint(tr("Search Jamendo"));
  library_filter_->SetAgeFilterEnabled(false);
}

void JamendoService::ShowContextMenu(const QModelIndex& index, const QPoint& global_pos) {
  EnsureMenuCreated();

  if (index.model() == library_sort_model_) {
    context_item_ = index;
  } else {
    context_item_ = QModelIndex();
  }

  add_to_playlist_->setEnabled(context_item_.isValid());
  load_to_playlist_->setEnabled(context_item_.isValid());
  open_in_new_playlist_->setEnabled(context_item_.isValid());
  album_info_->setEnabled(context_item_.isValid());
  download_album_->setEnabled(context_item_.isValid());
  context_menu_->popup(global_pos);
}

QWidget* JamendoService::HeaderWidget() const {
  const_cast<JamendoService*>(this)->EnsureMenuCreated();
  return library_filter_;
}

void JamendoService::AddToPlaylist() {
  AddItemToPlaylist(context_item_, false);
}

void JamendoService::LoadToPlaylist() {
  AddItemToPlaylist(context_item_, true);
}

void JamendoService::OpenInNewPlaylist() {
  AddItemToPlaylist(context_item_, false, false, true);
}

void JamendoService::AlbumInfo() {
  SongList songs(library_model_->GetChildSongs(
      library_sort_model_->mapToSource(context_item_)));
  if (songs.isEmpty())
    return;

  // We put the album ID into the comment field
  int id = songs.first().comment().toInt();
  if (!id)
    return;

  QDesktopServices::openUrl(QUrl(QString(kAlbumInfoUrl).arg(id)));
}

void JamendoService::DownloadAlbum() {
  SongList songs(library_model_->GetChildSongs(
      library_sort_model_->mapToSource(context_item_)));
  if (songs.isEmpty())
    return;

  // We put the album ID into the comment field
  int id = songs.first().comment().toInt();
  if (!id)
    return;

  QDesktopServices::openUrl(QUrl(QString(kDownloadAlbumUrl).arg(id)));
}

void JamendoService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}
