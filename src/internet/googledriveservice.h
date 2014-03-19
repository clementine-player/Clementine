#ifndef GOOGLEDRIVESERVICE_H
#define GOOGLEDRIVESERVICE_H

#include "cloudfileservice.h"

namespace google_drive {
class Client;
class ConnectResponse;
class File;
class ListFilesResponse;
class ListChangesResponse;
}

class GoogleDriveService : public CloudFileService {
  Q_OBJECT
 public:
  GoogleDriveService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;
  virtual void ShowContextMenu(const QPoint& global_pos);

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

  void OpenWithDrive();

 private:
  void EnsureConnected();
  void RefreshAuthorisation(const QString& refresh_token);
  void ListChanges(const QString& cursor);

  google_drive::Client* client_;

  QAction* open_in_drive_action_;
};

#endif
