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

#ifndef INTERNET_GOOGLEDRIVE_GOOGLEDRIVESERVICE_H_
#define INTERNET_GOOGLEDRIVE_GOOGLEDRIVESERVICE_H_

#include "googledriveclient.h"
#include "internet/core/cloudfileservice.h"

namespace google_drive {
class Client;
class ConnectResponse;
class ListFilesResponse;
class ListChangesResponse;
}  // namespace google_drive

class GoogleDriveService : public CloudFileService {
  Q_OBJECT

 public:
  GoogleDriveService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  // One file the user has picked via the Picker.
  struct PickedItem {
    QString id;
    QString title;
    // Empty for most items - see google_drive::Client::GetFile.
    QString resource_key;
  };

  virtual bool has_credentials() const override;

  google_drive::Client* client() const { return client_; }
  QString refresh_token() const;

  const QList<PickedItem>& picked_items() const { return picked_items_; }

  QUrl GetStreamingUrlFromSongId(const QString& file_id);

 public slots:
  void Connect() override;
  void ForgetCredentials();

  // Performs Google's combined OAuth-consent + Picker flow (opens the
  // system browser) so the user can pick more files to index. Used both
  // for the very first connection and for adding more later - a plain
  // login without picking anything grants access to nothing at all under
  // drive.file scope, so there's no separate "just log in" action.
  void AddFiles();
  // Forgets a previously-picked file and removes its song.
  void RemoveItem(const QString& id);

 signals:
  void Connected();
  void PickedItemsChanged();

 private slots:
  void ConnectFinished(google_drive::ConnectResponse* response);
  void AuthorizeAndPickFinished(google_drive::ConnectResponse* response);
  void FilesFound(const QList<google_drive::File>& files);
  void FilesDeleted(const QList<QUrl>& files);
  void ListChangesFinished(google_drive::ListChangesResponse* response,
                           int task_id);
  void SaveCursor(const QString& cursor);

  void AddPickedItemFinished(google_drive::GetFileResponse* response);

  void OpenWithDrive();
  void DoFullRescan() override;
  void CheckForUpdates();

 private:
  void EnsureConnected();
  void RefreshAuthorisation(const QString& refresh_token);
  void ListChanges(const QString& cursor);
  void AddPickedItem(const QString& id, const QString& title,
                     const QString& resource_key);
  void LoadPickedItems();
  void SavePickedItems() const;
  void MigrateLegacyCredentials();

  void PopulateContextMenu() override;
  void UpdateContextMenu() override;

  google_drive::Client* client_;

  // Guards against a second, independent Connect() -> CheckForUpdates()
  // chain starting while one is already in flight - EnsureConnected()'s
  // nested QEventLoop keeps the UI responsive while it blocks, so eg.
  // expanding the Google Drive tree item for the first time while a
  // playback-triggered EnsureConnected() call is still waiting on OAuth can
  // otherwise re-enter Connect() and kick off a second parallel scan.
  bool connect_in_progress_;
  // Guards ListChanges() itself against concurrent invocation - eg. the
  // "Check for updates" context menu action isn't disabled until the later
  // tag-reading phase starts (see is_indexing()), so it can otherwise be
  // triggered again while an earlier scan is still just checking for
  // changes.
  bool scan_in_progress_;

  QList<PickedItem> picked_items_;

  QAction* open_in_drive_action_;
  QAction* update_action_;
  QAction* full_rescan_action_;
};

#endif  // INTERNET_GOOGLEDRIVE_GOOGLEDRIVESERVICE_H_
