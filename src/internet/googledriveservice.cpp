#include "googledriveservice.h"

#include <QEventLoop>
#include <QScopedPointer>
#include <QSortFilterProxyModel>

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "googledriveclient.h"
#include "googledriveurlhandler.h"
#include "internetmodel.h"

const char* GoogleDriveService::kServiceName = "Google Drive";
const char* GoogleDriveService::kSettingsGroup = "GoogleDrive";

namespace {

static const char* kSongsTable = "google_drive_songs";
static const char* kFtsTable = "google_drive_songs_fts";

}


GoogleDriveService::GoogleDriveService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(NULL),
      client_(new google_drive::Client(this)),
      task_manager_(app->task_manager()),
      library_sort_model_(new QSortFilterProxyModel(this)) {
  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(app_->database(), kSongsTable,
                         QString::null, QString::null, kFtsTable);
  library_model_ = new LibraryModel(library_backend_, app_, this);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  app->player()->RegisterUrlHandler(new GoogleDriveUrlHandler(this, this));
  app->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_,
      tr("Google Drive"),
      "google_drive",
      QIcon(":/providers/googledrive.png"),
      true, app_, this));
}

QStandardItem* GoogleDriveService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/googledrive.png"), "Google Drive");
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void GoogleDriveService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      if (!client_->is_authenticated()) {
        Connect();
      }
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

void GoogleDriveService::Connect() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  google_drive::ConnectResponse* response =
      client_->Connect(s.value("refresh_token").toString());
  NewClosure(response, SIGNAL(Finished()),
             this, SLOT(ConnectFinished(google_drive::ConnectResponse*)),
             response);
}

void GoogleDriveService::ListFilesForMimeType(const QString& mime_type) {
  google_drive::ListFilesResponse* list_response = client_->ListFiles(
      QString("mimeType = '%1' and trashed = false").arg(mime_type));
  connect(list_response, SIGNAL(FilesFound(QList<google_drive::File>)),
          this, SLOT(FilesFound(QList<google_drive::File>)));
  NewClosure(list_response, SIGNAL(Finished()),
             this, SLOT(ListFilesFinished(google_drive::ListFilesResponse*)),
             list_response);
}

void GoogleDriveService::ConnectFinished(google_drive::ConnectResponse* response) {
  response->deleteLater();

  // Save the refresh token
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", response->refresh_token());

  // Find any music files
  ListFilesForMimeType("audio/mpeg");          // MP3/AAC
  ListFilesForMimeType("application/ogg");     // OGG
  ListFilesForMimeType("application/x-flac");  // FLAC
}

void GoogleDriveService::EnsureConnected() {
  if (client_->is_authenticated()) {
    return;
  }

  QEventLoop loop;
  connect(client_, SIGNAL(Authenticated()), &loop, SLOT(quit()));
  Connect();
  loop.exec();
}

void GoogleDriveService::FilesFound(const QList<google_drive::File>& files) {
  foreach (const google_drive::File& file, files) {
    MaybeAddFileToDatabase(file);
  }
}

void GoogleDriveService::ListFilesFinished(google_drive::ListFilesResponse* response) {
  response->deleteLater();
}

void GoogleDriveService::MaybeAddFileToDatabase(const google_drive::File& file) {
  QString url = QString("googledrive:%1").arg(file.id());
  Song song = library_backend_->GetSongByUrl(QUrl(url));
  // Song already in index.
  // TODO: Check etag and maybe update.
  if (song.is_valid()) {
    return;
  }

  const int task_id = task_manager_->StartTask(
      tr("Indexing %1").arg(file.title()));

  // Song not in index; tag and add.
  TagReaderClient::ReplyType* reply = app_->tag_reader_client()->ReadGoogleDrive(
      file.download_url(),
      file.title(),
      file.size(),
      file.mime_type(),
      client_->access_token());

  NewClosure(reply, SIGNAL(Finished(bool)),
             this, SLOT(ReadTagsFinished(TagReaderClient::ReplyType*,google_drive::File,QString,int)),
             reply, file, url, task_id);
}

void GoogleDriveService::ReadTagsFinished(TagReaderClient::ReplyType* reply,
                                          const google_drive::File& metadata,
                                          const QString& url,
                                          const int task_id) {
  reply->deleteLater();

  TaskManager::ScopedTask(task_id, task_manager_);

  const pb::tagreader::ReadGoogleDriveResponse& msg =
      reply->message().read_google_drive_response();
  if (!msg.has_metadata() || !msg.metadata().filesize()) {
    qLog(Debug) << "Failed to tag:" << metadata.title();
    return;
  }

  // Read the Song metadata from the message.
  Song song;
  song.InitFromProtobuf(msg.metadata());

  // Add some extra tags from the Google Drive metadata.
  song.set_etag(metadata.etag().remove('"'));
  song.set_mtime(metadata.modified_date().toTime_t());
  song.set_ctime(metadata.created_date().toTime_t());
  song.set_comment(metadata.description());
  song.set_directory_id(0);
  song.set_url(url);

  // Use the Google Drive title if we couldn't read tags from the file.
  if (song.title().isEmpty()) {
    song.set_title(metadata.title());
  }

  // Add the song to the database
  qLog(Debug) << "Adding song to db:" << song.title();
  library_backend_->AddOrUpdateSongs(SongList() << song);
}

QUrl GoogleDriveService::GetStreamingUrlFromSongId(const QString& id) {
  EnsureConnected();
  QScopedPointer<google_drive::GetFileResponse> response(client_->GetFile(id));

  QEventLoop loop;
  connect(response.data(), SIGNAL(Finished()), &loop, SLOT(quit()));
  loop.exec();

  QUrl url(response->file().download_url());
  url.setFragment(client_->access_token());
  return url;
}
