#include "cloudfileservice.h"

#include <QMenu>
#include <QSortFilterProxyModel>

#include "core/application.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "internet/internetmodel.h"
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
      settings_page_(settings_page) {
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

  app->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_, service_name, service_id, icon_, true, app_, this));
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
    context_menu_->addAction(IconLoader::Load("download"), tr("Cover Manager"),
                             this, SLOT(ShowCoverManager()));
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

  const int task_id =
      task_manager_->StartTask(tr("Indexing %1").arg(metadata.title()));

  TagReaderClient::ReplyType* reply = app_->tag_reader_client()->ReadCloudFile(
      download_url, metadata.title(), metadata.filesize(), mime_type,
      authorisation);
  NewClosure(reply, SIGNAL(Finished(bool)), this,
             SLOT(ReadTagsFinished(TagReaderClient::ReplyType*, Song, int)),
             reply, metadata, task_id);
}

void CloudFileService::ReadTagsFinished(TagReaderClient::ReplyType* reply,
                                        const Song& metadata,
                                        const int task_id) {
  reply->deleteLater();
  TaskManager::ScopedTask(task_id, task_manager_);

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
  if (filename.endsWith(".mp3")) {
    return "audio/mpeg";
  } else if (filename.endsWith(".m4a")) {
    return "audio/mpeg";
  } else if (filename.endsWith(".ogg") || filename.endsWith(".opus")) {
    return "application/ogg";
  } else if (filename.endsWith(".flac")) {
    return "application/x-flac";
  } else if (filename.endsWith(".wma")) {
    return "audio/x-ms-wma";
  }
  return QString::null;
}
