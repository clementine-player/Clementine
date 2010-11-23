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

#include <QFutureWatcher>
#include <QNetworkReply>
#include <QSortFilterProxyModel>
#include <QtConcurrentRun>
#include <QXmlStreamReader>
#include "qtiocompressor.h"

#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/taskmanager.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "library/librarymodel.h"
#include "radio/radiomodel.h"

const char* JamendoService::kServiceName = "Jamendo";
const char* JamendoService::kDirectoryUrl =
    "http://img.jamendo.com/data/dbdump_artistalbumtrack.xml.gz";
const char* JamendoService::kMp3StreamUrl =
    "http://api.jamendo.com/get2/stream/track/redirect/?id=%1&streamencoding=mp31";
const char* JamendoService::kOggStreamUrl =
    "http://api.jamendo.com/get2/stream/track/redirect/?id=%1&streamencoding=ogg2";
const char* JamendoService::kAlbumCoverUrl =
    "http://api.jamendo.com/get2/image/album/redirect/?id=%1&imagesize={100-600}";

const char* JamendoService::kSongsTable = "jamendo_songs";
const char* JamendoService::kFtsTable = "jamendo_songs_fts";

const char* JamendoService::kSettingsGroup = "Jamendo";

const int JamendoService::kBatchSize = 10000;
const int JamendoService::kApproxDatabaseSize = 300000;

JamendoService::JamendoService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      network_(new NetworkAccessManager(this)),
      root_(NULL),
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

  library_model_ = new LibraryModel(library_backend_, this);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);
}

JamendoService::~JamendoService() {
}

RadioItem* JamendoService::CreateRootItem(RadioItem* parent) {
  root_ = new RadioItem(this, RadioItem::Type_Service, kServiceName, parent);
  root_->icon = QIcon(":providers/jamendo.png");
  return root_;
}

void JamendoService::LazyPopulate(RadioItem* item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      library_model_->Init();
      model()->merged_model()->AddSubModel(
          model()->index(root_->row, 0, model()->ItemToIndex(item->parent)),
          library_sort_model_);
      break;
    default:
      break;
  }
  item->lazy_loaded = true;
}

void JamendoService::UpdateTotalSongCount(int count) {
  qDebug() << Q_FUNC_INFO << count;
  total_song_count_ = count;
  if (total_song_count_ == 0) {
    DownloadDirectory();
  }
}

void JamendoService::DownloadDirectory() {
  QNetworkRequest req = QNetworkRequest(QUrl(kDirectoryUrl));
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

  SongList songs;
  QXmlStreamReader reader(device);
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "artist") {
      songs << ReadArtist(&reader);
    }

    if (songs.count() >= kBatchSize) {
      // Add the songs to the database in batches
      library_backend_->AddOrUpdateSongs(songs);
      total_count += songs.count();
      songs.clear();

      // Update progress info
      model()->task_manager()->SetTaskProgress(
            load_database_task_id_, total_count, kApproxDatabaseSize);
    }
  }

  library_backend_->AddOrUpdateSongs(songs);

  connect(library_backend_, SIGNAL(SongsDiscovered(SongList)),
          library_model_, SLOT(SongsDiscovered(SongList)));
  library_model_->Reset();
}

SongList JamendoService::ReadArtist(QXmlStreamReader* reader) const {
  SongList ret;
  QString current_artist;
  QString current_album;

  while (!reader->atEnd()) {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      if (name == "name") {
        current_artist = reader->readElementText().trimmed();
      } else if (name == "album") {
        ret << ReadAlbum(current_artist, reader);
      }
    } else if (reader->isEndElement() && reader->name() == "artist") {
      break;
    }
  }

  return ret;
}

SongList JamendoService::ReadAlbum(
    const QString& artist, QXmlStreamReader* reader) const {
  SongList ret;
  QString current_album;
  QString cover;

  while (!reader->atEnd()) {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      if (reader->name() == "name") {
        current_album = reader->readElementText().trimmed();
      } else if (reader->name() == "id") {
        QString id = reader->readElementText();
        cover = QString(kAlbumCoverUrl).arg(id);
      } else if (reader->name() == "track") {
        ret << ReadTrack(artist, current_album, cover, reader);
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
                               QXmlStreamReader* reader) const {
  Song song;
  song.set_artist(artist);
  song.set_album(album);
  song.set_filetype(Song::Type_Stream);
  song.set_directory_id(0);
  song.set_mtime(0);
  song.set_ctime(0);
  song.set_filesize(0);

  while (!reader->atEnd()) {
    reader->readNext();
    if (reader->isStartElement()) {
      QStringRef name = reader->name();
      if (name == "name") {
        song.set_title(reader->readElementText().trimmed());
      } else if (name == "length") {
        QString length = reader->readElementText();
        // Some durations are 123.0 and some are 123.
        length = length.split('.')[0];
        song.set_length(length.toInt());
      } else if (name == "genre") {
        int genre_id = reader->readElementText().toInt();
        // In theory, genre 0 is "blues"; in practice it's invalid.
        if (genre_id != 0) {
          song.set_genre(genre_id);
        }
      } else if (name == "id") {
        QString id = reader->readElementText();
        QString ogg_url = QString(kOggStreamUrl).arg(id);
        song.set_filename(ogg_url);

        song.set_art_automatic(album_cover);
        song.set_valid(true);
      }
    } else if (reader->isEndElement() && reader->name() == "track") {
      break;
    }
  }
  return song;
}

void JamendoService::ParseDirectoryFinished() {
  QFutureWatcher<void>* watcher = static_cast<QFutureWatcher<void>*>(sender());

  model()->task_manager()->SetTaskFinished(load_database_task_id_);
  load_database_task_id_ = 0;
}

void JamendoService::EnsureMenuCreated() {
  if (library_filter_)
    return;

  library_filter_ = new LibraryFilterWidget(0);
  library_filter_->SetSettingsGroup(kSettingsGroup);
  library_filter_->SetLibraryModel(library_model_);
  library_filter_->SetFilterHint(tr("Search Jamendo"));
  library_filter_->SetAgeFilterEnabled(false);
}

QWidget* JamendoService::HeaderWidget() const {
  const_cast<JamendoService*>(this)->EnsureMenuCreated();
  return library_filter_;
}
