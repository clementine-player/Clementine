/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Martin Brodbeck <martin@brodbeck-online.de>
   Copyright 2013-2014, David Sansome <me@davidsansome.com>
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

#include "internet/core/cloudfileservice.h"

#include <QMenu>
#include <QSortFilterProxyModel>

#include "core/application.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "globalsearch/globalsearch.h"
#include "internet/core/cloudfilesearchprovider.h"
#include "internet/core/internetmodel.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "playlist/playlist.h"
#include "ui/iconloader.h"

CloudFileService::CloudFileService(Application* app, InternetModel* parent,
                                   const QString& service_name,
                                   const QString& service_id, const QIcon& icon,
                                   SettingsDialog::Page settings_page)
    : InternetService(service_name, app, parent, parent),
      root_(nullptr),
      network_(new NetworkAccessManager(this)),
      library_sort_model_(new QSortFilterProxyModel(this)),
      playlist_manager_(app->playlist_manager()),
      task_manager_(app->task_manager()),
      icon_(icon),
      settings_page_(settings_page),
      indexing_task_id_(-1),
      indexing_task_progress_(0),
      indexing_task_max_(0) {
  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());

  QString songs_table = service_id + "_songs";
  QString songs_fts_table = service_id + "_songs_fts";

  library_backend_->Init(app->database(), songs_table, QString::null,
                         QString::null, songs_fts_table);
  library_model_ = new LibraryModel(library_backend_, app_, this);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->setSortLocaleAware(true);
  library_sort_model_->sort(0);

  app->global_search()->AddProvider(
      new CloudFileSearchProvider(library_backend_, service_id, icon_, this));
}

QStandardItem* CloudFileService::CreateRootItem() {
  root_ = new QStandardItem(icon_, name());
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void CloudFileService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      if (!has_credentials()) {
        ShowSettingsDialog();
      } else {
        Connect();
      }
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

void CloudFileService::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_.reset(new QMenu);
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(IconLoader::Load("download", IconLoader::Base), 
                             tr("Cover Manager"), this, 
                             SLOT(ShowCoverManager()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base), 
                             tr("Configure..."), this, 
                             SLOT(ShowSettingsDialog()));
  }
  context_menu_->popup(global_pos);
}

void CloudFileService::ShowCoverManager() {
  if (!cover_manager_) {
    cover_manager_.reset(new AlbumCoverManager(app_, library_backend_));
    cover_manager_->Init();
    connect(cover_manager_.get(), SIGNAL(AddToPlaylist(QMimeData*)),
            SLOT(AddToPlaylist(QMimeData*)));
  }
  cover_manager_->show();
}

void CloudFileService::AddToPlaylist(QMimeData* mime) {
  playlist_manager_->current()->dropMimeData(mime, Qt::CopyAction, -1, 0,
                                             QModelIndex());
}

void CloudFileService::ShowSettingsDialog() {
  app_->OpenSettingsDialogAtPage(settings_page_);
}

bool CloudFileService::ShouldIndexFile(const QUrl& url,
                                       const QString& mime_type) const {
  if (!IsSupportedMimeType(mime_type)) {
    return false;
  }
  Song library_song = library_backend_->GetSongByUrl(url);
  if (library_song.is_valid()) {
    qLog(Debug) << "Already have:" << url;
    return false;
  }
  return true;
}

void CloudFileService::MaybeAddFileToDatabase(const Song& metadata,
                                              const QString& mime_type,
                                              const QUrl& download_url,
                                              const QString& authorisation) {
  if (!ShouldIndexFile(metadata.url(), mime_type)) {
    return;
  }

  if (indexing_task_id_ == -1) {
    indexing_task_id_ = task_manager_->StartTask(tr("Indexing %1").arg(name()));
    indexing_task_progress_ = 0;
    indexing_task_max_ = 0;
  }
  indexing_task_max_++;
  task_manager_->SetTaskProgress(indexing_task_id_, indexing_task_progress_,
                                 indexing_task_max_);

  TagReaderClient::ReplyType* reply = app_->tag_reader_client()->ReadCloudFile(
      download_url, metadata.title(), metadata.filesize(), mime_type,
      authorisation);
  pending_tagreader_replies_.append(reply);

  NewClosure(reply, SIGNAL(Finished(bool)), this,
             SLOT(ReadTagsFinished(TagReaderClient::ReplyType*, Song)), reply,
             metadata);
}

void CloudFileService::ReadTagsFinished(TagReaderClient::ReplyType* reply,
                                        const Song& metadata) {
  int index_reply;

  reply->deleteLater();

  if ((index_reply = pending_tagreader_replies_.indexOf(reply)) == -1) {
    qLog(Debug) << "Ignore the reply";
    return;
  }

  pending_tagreader_replies_.removeAt(index_reply);

  indexing_task_progress_++;
  if (indexing_task_progress_ == indexing_task_max_) {
    task_manager_->SetTaskFinished(indexing_task_id_);
    indexing_task_id_ = -1;
    emit AllIndexingTasksFinished();
  } else {
    task_manager_->SetTaskProgress(indexing_task_id_, indexing_task_progress_,
                                   indexing_task_max_);
  }

  const pb::tagreader::ReadCloudFileResponse& message =
      reply->message().read_cloud_file_response();
  if (!message.has_metadata() || !message.metadata().filesize()) {
    qLog(Debug) << "Failed to tag:" << metadata.url();
    return;
  }

  pb::tagreader::SongMetadata metadata_pb;
  metadata.ToProtobuf(&metadata_pb);
  metadata_pb.MergeFrom(message.metadata());

  Song song;
  song.InitFromProtobuf(metadata_pb);
  song.set_directory_id(0);

  qLog(Debug) << "Adding song to db:" << song.title();
  library_backend_->AddOrUpdateSongs(SongList() << song);
}

bool CloudFileService::IsSupportedMimeType(const QString& mime_type) const {
  return mime_type == "audio/ogg" || mime_type == "audio/mpeg" ||
         mime_type == "audio/mp4" || mime_type == "audio/flac" ||
         mime_type == "audio/x-flac" || mime_type == "application/ogg" ||
         mime_type == "application/x-flac" || mime_type == "audio/x-ms-wma";
}

QString CloudFileService::GuessMimeTypeForFile(const QString& filename) const {
  if (filename.endsWith(".mp3", Qt::CaseInsensitive)) {
    return "audio/mpeg";
  } else if (filename.endsWith(".m4a", Qt::CaseInsensitive) ||
             filename.endsWith(".m4b", Qt::CaseInsensitive)) {
    return "audio/mpeg";
  } else if (filename.endsWith(".ogg", Qt::CaseInsensitive) ||
             filename.endsWith(".opus", Qt::CaseInsensitive)) {
    return "application/ogg";
  } else if (filename.endsWith(".flac", Qt::CaseInsensitive)) {
    return "application/x-flac";
  } else if (filename.endsWith(".wma", Qt::CaseInsensitive)) {
    return "audio/x-ms-wma";
  }
  return QString::null;
}

void CloudFileService::AbortReadTagsReplies() {
  qLog(Debug) << "Aborting the read tags replies";
  pending_tagreader_replies_.clear();

  task_manager_->SetTaskFinished(indexing_task_id_);
  indexing_task_id_ = -1;
  emit AllIndexingTasksFinished();
}
