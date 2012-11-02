#include "googledriveservice.h"

#include <QDesktopServices>
#include <QEventLoop>
#include <QMenu>
#include <QScopedPointer>
#include <QSortFilterProxyModel>

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "ui/albumcovermanager.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "playlist/playlist.h"
#include "ui/iconloader.h"
#include "googledriveclient.h"
#include "googledriveurlhandler.h"
#include "internetmodel.h"

const char* GoogleDriveService::kServiceName = "Google Drive";
const char* GoogleDriveService::kSettingsGroup = "GoogleDrive";

namespace {

static const char* kSongsTable = "google_drive_songs";
static const char* kFtsTable = "google_drive_songs_fts";
static const char* kDriveEditFileUrl = "https://docs.google.com/file/d/%1/edit";

}


GoogleDriveService::GoogleDriveService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(NULL),
      client_(new google_drive::Client(this)),
      task_manager_(app->task_manager()),
      library_sort_model_(new QSortFilterProxyModel(this)),
      playlist_manager_(app->playlist_manager()) {
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

GoogleDriveService::~GoogleDriveService() {
}

QStandardItem* GoogleDriveService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/googledrive.png"), "Google Drive");
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void GoogleDriveService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      if (refresh_token().isEmpty()) {
        ShowSettingsDialog();
      } else if (!client_->is_authenticated()) {
        Connect();
      }
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

QString GoogleDriveService::refresh_token() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  return s.value("refresh_token").toString();
}

void GoogleDriveService::Connect() {
  google_drive::ConnectResponse* response = client_->Connect(refresh_token());
  NewClosure(response, SIGNAL(Finished()),
             this, SLOT(ConnectFinished(google_drive::ConnectResponse*)),
             response);
}

void GoogleDriveService::ForgetCredentials() {
  client_->ForgetCredentials();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.remove("refresh_token");
  s.remove("user_email");
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

  if (!response->user_email().isEmpty()) {
    // We only fetch the user's email address the first time we authenticate.
    s.setValue("user_email", response->user_email());
  }

  emit Connected();

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
  song.set_url(QUrl(url));

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
  url.addQueryItem("access_token", client_->access_token());
  return url;
}

void GoogleDriveService::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_.reset(new QMenu);
    context_menu_->addActions(GetPlaylistActions());
    open_in_drive_action_ = context_menu_->addAction(
          QIcon(":/providers/googledrive.png"), tr("Open in Google Drive"),
          this, SLOT(OpenWithDrive()));
    context_menu_->addSeparator();
    context_menu_->addAction(
        IconLoader::Load("download"),
        tr("Cover Manager"),
        this,
        SLOT(ShowCoverManager()));
    context_menu_->addAction(IconLoader::Load("configure"),
                             tr("Configure..."),
                             this, SLOT(ShowSettingsDialog()));
  }

  // Only show some actions if there are real songs selected
  bool songs_selected = false;
  foreach (const QModelIndex& index, model()->selected_indexes()) {
    const int type = index.data(LibraryModel::Role_Type).toInt();
    if (type == LibraryItem::Type_Song ||
        type == LibraryItem::Type_Container) {
      songs_selected = true;
      break;
    }
  }

  open_in_drive_action_->setEnabled(songs_selected);

  context_menu_->popup(global_pos);
}

void GoogleDriveService::OpenWithDrive() {
  // Map indexes to the actual library model.
  QModelIndexList library_indexes;
  foreach (const QModelIndex& index, model()->selected_indexes()) {
    if (index.model() == library_sort_model_) {
      library_indexes << library_sort_model_->mapToSource(index);
    }
  }

  // Ask the library for the songs for these indexes.
  foreach (const Song& song, library_model_->GetChildSongs(library_indexes)) {
    QDesktopServices::openUrl(
          QUrl(QString(kDriveEditFileUrl).arg(song.url().path())));
  }
}

void GoogleDriveService::ShowSettingsDialog() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_GoogleDrive);
}

void GoogleDriveService::ShowCoverManager() {
  if (!cover_manager_) {
    cover_manager_.reset(new AlbumCoverManager(app_, library_backend_));
    cover_manager_->Init();
    connect(cover_manager_.get(), SIGNAL(AddToPlaylist(QMimeData*)), SLOT(AddToPlaylist(QMimeData*)));
  }

  cover_manager_->show();
}

void GoogleDriveService::AddToPlaylist(QMimeData* mime) {
  playlist_manager_->current()->dropMimeData(
      mime, Qt::CopyAction, -1, 0, QModelIndex());
}
