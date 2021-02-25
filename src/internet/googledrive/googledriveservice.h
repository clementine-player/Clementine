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

#include "internet/core/cloudfileservice.h"

namespace google_drive {
class Client;
class ConnectResponse;
class File;
class ListFilesResponse;
class ListChangesResponse;
}  // namespace google_drive

class GoogleDriveService : public CloudFileService {
  Q_OBJECT

 public:
  GoogleDriveService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;

  google_drive::Client* client() const { return client_; }
  QString refresh_token() const;

  QUrl GetStreamingUrlFromSongId(const QString& file_id);

 public slots:
  void Connect();
  void ForgetCredentials();

 signals:
  void Connected();

 private slots:
  void ConnectFinished(google_drive::ConnectResponse* response);
  void FilesFound(const QList<google_drive::File>& files);
  void FilesDeleted(const QList<QUrl>& files);
  void ListChangesFinished(google_drive::ListChangesResponse* response);
  void SaveCursor(const QString& cursor);

  void OpenWithDrive();
  void DoFullRescan() override;
  void CheckForUpdates();

 private:
  void EnsureConnected();
  void RefreshAuthorisation(const QString& refresh_token);
  void ListChanges(const QString& cursor);

  void PopulateContextMenu() override;
  void UpdateContextMenu() override;

  google_drive::Client* client_;

  QAction* open_in_drive_action_;
  QAction* update_action_;
  QAction* full_rescan_action_;
};

#endif  // INTERNET_GOOGLEDRIVE_GOOGLEDRIVESERVICE_H_
