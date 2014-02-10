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

static const char* kDriveEditFileUrl = "https://docs.google.com/file/d/%1/edit";
static const char* kServiceId = "google_drive";
}

GoogleDriveService::GoogleDriveService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       QIcon(":/providers/googledrive.png"),
                       SettingsDialog::Page_GoogleDrive),
      client_(new google_drive::Client(this)) {
  app->player()->RegisterUrlHandler(new GoogleDriveUrlHandler(this, this));
}

bool GoogleDriveService::has_credentials() const {
  return !refresh_token().isEmpty();
}

QString GoogleDriveService::refresh_token() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  return s.value("refresh_token").toString();
}

void GoogleDriveService::Connect() {
  google_drive::ConnectResponse* response = client_->Connect(refresh_token());
  NewClosure(response, SIGNAL(Finished()), this,
             SLOT(ConnectFinished(google_drive::ConnectResponse*)), response);
}

void GoogleDriveService::ForgetCredentials() {
  client_->ForgetCredentials();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.remove("refresh_token");
  s.remove("user_email");
}

void GoogleDriveService::ListChanges(const QString& cursor) {
  google_drive::ListChangesResponse* changes_response =
      client_->ListChanges(cursor);
  connect(changes_response, SIGNAL(FilesFound(QList<google_drive::File>)),
          SLOT(FilesFound(QList<google_drive::File>)));
  connect(changes_response, SIGNAL(FilesDeleted(QList<QUrl>)),
          SLOT(FilesDeleted(QList<QUrl>)));
  NewClosure(changes_response, SIGNAL(Finished()), this,
             SLOT(ListChangesFinished(google_drive::ListChangesResponse*)),
             changes_response);
}

void GoogleDriveService::ListChangesFinished(
    google_drive::ListChangesResponse* changes_response) {
  changes_response->deleteLater();
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("cursor", changes_response->next_cursor());
}

void GoogleDriveService::ConnectFinished(
    google_drive::ConnectResponse* response) {
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

  // Find all the changes since the last check.
  ListChanges(s.value("cursor").toString());
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
  for (const google_drive::File& file : files) {
    if (!IsSupportedMimeType(file.mime_type())) {
      continue;
    }

    QUrl url;
    url.setScheme("googledrive");
    url.setPath(file.id());

    Song song;
    // Add some extra tags from the Google Drive metadata.
    song.set_etag(file.etag().remove('"'));
    song.set_mtime(file.modified_date().toTime_t());
    song.set_ctime(file.created_date().toTime_t());
    song.set_comment(file.description());
    song.set_directory_id(0);
    song.set_url(QUrl(url));
    song.set_filesize(file.size());

    // Use the Google Drive title if we couldn't read tags from the file.
    if (song.title().isEmpty()) {
      song.set_title(file.title());
    }

    MaybeAddFileToDatabase(song, file.mime_type(), file.download_url(),
                           QString("Bearer %1").arg(client_->access_token()));
  }
}

void GoogleDriveService::FilesDeleted(const QList<QUrl>& files) {
  for (const QUrl& url : files) {
    Song song = library_backend_->GetSongByUrl(url);
    qLog(Debug) << "Deleting:" << url << song.title();
    if (song.is_valid()) {
      library_backend_->DeleteSongs(SongList() << song);
    }
  }
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
        QIcon(":/providers/googledrive.png"), tr("Open in Google Drive"), this,
        SLOT(OpenWithDrive()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download"), tr("Cover Manager"),
                             this, SLOT(ShowCoverManager()));
    context_menu_->addAction(IconLoader::Load("configure"), tr("Configure..."),
                             this, SLOT(ShowSettingsDialog()));
  }

  // Only show some actions if there are real songs selected
  bool songs_selected = false;
  for (const QModelIndex& index : model()->selected_indexes()) {
    const int type = index.data(LibraryModel::Role_Type).toInt();
    if (type == LibraryItem::Type_Song || type == LibraryItem::Type_Container) {
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
  for (const QModelIndex& index : model()->selected_indexes()) {
    if (index.model() == library_sort_model_) {
      library_indexes << library_sort_model_->mapToSource(index);
    }
  }

  // Ask the library for the songs for these indexes.
  for (const Song& song : library_model_->GetChildSongs(library_indexes)) {
    QDesktopServices::openUrl(
        QUrl(QString(kDriveEditFileUrl).arg(song.url().path())));
  }
}
