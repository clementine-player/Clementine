/* This file is part of Clementine.
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2012, 2014, David Sansome <me@davidsansome.com>
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

#include "googledriveservice.h"

#include <QDesktopServices>
#include <QEventLoop>
#include <QMenu>
#include <QScopedPointer>
#include <QSortFilterProxyModel>
#include <QUrlQuery>

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "googledriveclient.h"
#include "googledriveurlhandler.h"
#include "internet/core/internetmodel.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "playlist/playlist.h"
#include "ui/albumcovermanager.h"
#include "ui/iconloader.h"

const char* GoogleDriveService::kServiceName = "Google Drive";
const char* GoogleDriveService::kSettingsGroup = "GoogleDrive";

namespace {

static const char* kDriveEditFileUrl = "https://docs.google.com/file/d/%1/edit";
static const char* kServiceId = "google_drive";

// Bumped whenever the requested OAuth scope changes in a way that requires
// existing users to reconnect (see MigrateLegacyCredentials). Version 2
// switched from the broad "drive.readonly" scope to "drive.file", which
// only grants access to items picked via the Google Picker.
static const int kScopeVersion = 2;

// Mime types the picker's file view is restricted to. Keep in sync with
// CloudFileService::IsSupportedMimeType.
QStringList PickableMimeTypes() {
  return QStringList() << "audio/ogg" << "audio/mpeg" << "audio/mp4"
                       << "audio/flac" << "audio/x-flac" << "application/ogg"
                       << "application/x-flac" << "audio/x-ms-wma";
}
}  // namespace

GoogleDriveService::GoogleDriveService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       IconLoader::Load("googledrive", IconLoader::Provider),
                       SettingsDialog::Page_GoogleDrive),
      client_(new google_drive::Client(this)),
      connect_in_progress_(false),
      scan_in_progress_(false),
      open_in_drive_action_(nullptr),
      update_action_(nullptr),
      full_rescan_action_(nullptr) {
  app->player()->RegisterUrlHandler(new GoogleDriveUrlHandler(this, this));

  MigrateLegacyCredentials();
  LoadPickedItems();
}

bool GoogleDriveService::has_credentials() const {
  return !refresh_token().isEmpty();
}

QString GoogleDriveService::refresh_token() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  return s.value("refresh_token").toString();
}

void GoogleDriveService::MigrateLegacyCredentials() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  if (!s.value("refresh_token").toString().isEmpty() &&
      s.value("scope_version").toInt() < kScopeVersion) {
    qLog(Info) << "Google Drive was linked with the old full-Drive access "
                 "scope; clearing stored credentials so the user can "
                 "reconnect using the new file picker.";
    s.remove("refresh_token");
    s.remove("user_email");
    s.remove("cursor");
  }
}

void GoogleDriveService::LoadPickedItems() {
  picked_items_.clear();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  const int count = s.beginReadArray("picked_items");
  for (int i = 0; i < count; ++i) {
    s.setArrayIndex(i);
    PickedItem item;
    item.id = s.value("id").toString();
    item.title = s.value("title").toString();
    item.resource_key = s.value("resource_key").toString();
    picked_items_ << item;
  }
  s.endArray();
}

void GoogleDriveService::SavePickedItems() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.beginWriteArray("picked_items", picked_items_.size());
  for (int i = 0; i < picked_items_.size(); ++i) {
    s.setArrayIndex(i);
    s.setValue("id", picked_items_[i].id);
    s.setValue("title", picked_items_[i].title);
    s.setValue("resource_key", picked_items_[i].resource_key);
  }
  s.endArray();
}

void GoogleDriveService::AddPickedItem(const QString& id, const QString& title,
                                       const QString& resource_key) {
  for (const PickedItem& item : picked_items_) {
    if (item.id == id) {
      // Already tracking this file (eg. the user re-picked it, or this is
      // a full rescan re-adding it).
      return;
    }
  }

  PickedItem item;
  item.id = id;
  item.title = title;
  item.resource_key = resource_key;
  picked_items_ << item;

  SavePickedItems();
  emit PickedItemsChanged();
}

void GoogleDriveService::Connect() {
  if (refresh_token().isEmpty()) {
    // Nothing to silently reconnect with - drive.file access can only ever
    // be granted via the combined OAuth+Picker flow in AddFiles(), so
    // there's no "just log in" step to fall back to here. Callers that
    // need to block until connected (EnsureConnected()) check for this
    // themselves rather than waiting on a signal that would never come.
    qLog(Debug) << "Google Drive: no stored credentials to reconnect with";
    return;
  }
  if (connect_in_progress_) {
    // Already connecting - EnsureConnected()'s nested event loop can let
    // this get called again reentrantly while that's still in flight. The
    // waiters - including any nested EnsureConnected() event loop - all get
    // unblocked by the in-flight call's own Authenticated() signal, so
    // there's nothing more to do here.
    qLog(Debug) << "Google Drive Connect() already in progress, ignoring";
    return;
  }
  connect_in_progress_ = true;

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
  s.remove("scope_version");
  s.remove("cursor");
  s.remove("picked_items");

  picked_items_.clear();
  emit PickedItemsChanged();
}

void GoogleDriveService::AddFiles() {
  google_drive::ConnectResponse* response =
      client_->AuthorizeAndPick(PickableMimeTypes());
  NewClosure(response, SIGNAL(Finished()), this,
             SLOT(AuthorizeAndPickFinished(google_drive::ConnectResponse*)),
             response);
}

void GoogleDriveService::AuthorizeAndPickFinished(
    google_drive::ConnectResponse* response) {
  response->deleteLater();

  qLog(Debug) << "AuthorizeAndPick finished: picked ids ="
             << response->picked_file_ids();

  // Save the refresh token, same as a plain ConnectFinished() would.
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", response->refresh_token());
  s.setValue("scope_version", kScopeVersion);
  if (!response->user_email().isEmpty()) {
    s.setValue("user_email", response->user_email());
  }

  emit Connected();

  // The redirect only gives us ids - fetch each one's metadata (see
  // AddPickedItemFinished).
  for (const QString& id : response->picked_file_ids()) {
    google_drive::GetFileResponse* file_response = client_->GetFile(id);
    NewClosure(
        file_response, SIGNAL(Finished()), this,
        SLOT(AddPickedItemFinished(google_drive::GetFileResponse*)),
        file_response);
  }

  // Also check for any changes since we last synced.
  CheckForUpdates();
}

void GoogleDriveService::AddPickedItemFinished(
    google_drive::GetFileResponse* response) {
  response->deleteLater();

  qLog(Debug) << "GetFile finished for picked item" << response->file_id()
             << ": had_error =" << response->had_error()
             << ", title =" << response->file().title()
             << ", mime type =" << response->file().mime_type();

  if (response->had_error()) {
    return;
  }

  // No resource key here: this is only ever reached for items the current
  // AuthorizeAndPick redirect (or a rescan of an already-tracked item, in
  // which case AddPickedItem() below is a no-op since it's already tracked
  // - see DoFullRescan()) returned, and that flow doesn't hand one back.
  AddPickedItem(response->file_id(), response->file().title(), QString());
  FilesFound(google_drive::FileList() << response->file());
}

void GoogleDriveService::RemoveItem(const QString& id) {
  for (int i = 0; i < picked_items_.size(); ++i) {
    if (picked_items_[i].id == id) {
      picked_items_.removeAt(i);
      break;
    }
  }

  SavePickedItems();
  emit PickedItemsChanged();

  // Simplest correct way to make sure the library no longer contains
  // anything from the removed file: reindex everything that's still picked
  // from scratch.
  DoFullRescan();
}

void GoogleDriveService::ListChanges(const QString& cursor) {
  if (scan_in_progress_) {
    qLog(Debug) << "Google Drive scan already in progress, ignoring";
    return;
  }
  scan_in_progress_ = true;

  const int task_id =
      task_manager_->StartTask(tr("Checking Google Drive for changes..."));

  google_drive::ListChangesResponse* changes_response =
      client_->ListChanges(cursor);
  connect(changes_response, SIGNAL(FilesFound(QList<google_drive::File>)),
          SLOT(FilesFound(QList<google_drive::File>)));
  connect(changes_response, SIGNAL(FilesDeleted(QList<QUrl>)),
          SLOT(FilesDeleted(QList<QUrl>)));
  NewClosure(
      changes_response, SIGNAL(Finished()), this,
      SLOT(ListChangesFinished(google_drive::ListChangesResponse*, int)),
      changes_response, task_id);
}

void GoogleDriveService::ListChangesFinished(
    google_drive::ListChangesResponse* changes_response, int task_id) {
  changes_response->deleteLater();
  task_manager_->SetTaskFinished(task_id);
  scan_in_progress_ = false;

  const QString cursor = changes_response->next_cursor();
  if (is_indexing()) {
    // Only save the cursor after all the songs have been indexed - that way if
    // Clementine is closed it'll resume next time.
    NewClosure(this, SIGNAL(AllIndexingTasksFinished()), this,
               SLOT(SaveCursor(QString)), cursor);
  } else {
    SaveCursor(cursor);
  }
}

void GoogleDriveService::SaveCursor(const QString& cursor) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("cursor", cursor);
}

void GoogleDriveService::ConnectFinished(
    google_drive::ConnectResponse* response) {
  response->deleteLater();
  connect_in_progress_ = false;

  // Save the refresh token
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", response->refresh_token());
  s.setValue("scope_version", kScopeVersion);

  if (!response->user_email().isEmpty()) {
    // We only fetch the user's email address the first time we authenticate.
    s.setValue("user_email", response->user_email());
  }

  emit Connected();

  // Find all the changes since the last check.
  CheckForUpdates();
}

void GoogleDriveService::EnsureConnected() {
  if (client_->is_authenticated()) {
    return;
  }
  if (refresh_token().isEmpty()) {
    // Nothing Connect() can do here (see its own early-return) - avoid
    // blocking forever below waiting for an Authenticated() that will never
    // come.
    qLog(Warning) << "Google Drive: cannot ensure connection, not linked yet";
    return;
  }

  QEventLoop loop;
  connect(client_, SIGNAL(Authenticated()), &loop, SLOT(quit()));
  Connect();
  loop.exec();
}

void GoogleDriveService::FilesFound(const QList<google_drive::File>& files) {
  qLog(Debug) << "FilesFound:" << files.size() << "file(s)";

  for (const google_drive::File& file : files) {
    qLog(Debug) << "  " << file.id() << file.title() << file.mime_type()
               << "supported =" << IsSupportedMimeType(file.mime_type());

    if (!IsSupportedMimeType(file.mime_type())) {
      continue;
    }

    QUrl url;
    url.setScheme("googledrive");
    url.setPath("/" + file.id());

    Song song;
    // Add some extra tags from the Google Drive metadata.
    song.set_etag(file.etag().remove('"'));
    song.set_mtime(file.modified_date().toSecsSinceEpoch());
    song.set_ctime(file.created_date().toSecsSinceEpoch());
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

  if (response->had_error()) {
    app_->AddError(tr("Could not find Google Drive file."));
    return QUrl();
  }

  return QUrl(response->file().download_url());
}

void GoogleDriveService::PopulateContextMenu() {
  context_menu_->addActions(GetPlaylistActions());
  open_in_drive_action_ = context_menu_->addAction(
      IconLoader::Load("googledrive", IconLoader::Provider),
      tr("Open in Google Drive"), this, SLOT(OpenWithDrive()));
  context_menu_->addSeparator();
  update_action_ = context_menu_->addAction(
      IconLoader::Load("view-refresh", IconLoader::Base),
      tr("Check for updates"), this, SLOT(CheckForUpdates()));
  full_rescan_action_ = context_menu_->addAction(
      IconLoader::Load("view-refresh", IconLoader::Base),
      tr("Do a full rescan..."), this, SLOT(FullRescanRequested()));
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                           tr("Cover Manager"), this, SLOT(ShowCoverManager()));
  context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base),
                           tr("Configure..."), this, SLOT(ShowConfig()));
}

void GoogleDriveService::UpdateContextMenu() {
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
  update_action_->setEnabled(!is_indexing());
  full_rescan_action_->setEnabled(!is_indexing());
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

void GoogleDriveService::DoFullRescan() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("cursor");

  library_backend_->DeleteAll();

  // The Changes API only reports changes going forward from a cursor, so it
  // can't backfill a picked item's existing content - re-fetch everything
  // we currently know about directly instead.
  for (const PickedItem& item : picked_items_) {
    google_drive::GetFileResponse* response =
        client_->GetFile(item.id, item.resource_key);
    NewClosure(
        response, SIGNAL(Finished()), this,
        SLOT(AddPickedItemFinished(google_drive::GetFileResponse*)),
        response);
  }

  // Establish a fresh cursor so future CheckForUpdates() calls only look at
  // changes from this point on.
  ListChanges(QString());
}

void GoogleDriveService::CheckForUpdates() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  ListChanges(s.value("cursor").toString());
}
