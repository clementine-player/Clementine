#ifndef GOOGLEDRIVESERVICE_H
#define GOOGLEDRIVESERVICE_H

#include "internetservice.h"

#include "core/network.h"
#include "core/tagreaderclient.h"

class QStandardItem;

class AlbumCoverManager;
class LibraryBackend;
class LibraryModel;
class PlaylistManager;
class TaskManager;
class QSortFilterProxyModel;

namespace google_drive {
  class Client;
  class ConnectResponse;
  class File;
  class ListFilesResponse;
}

class GoogleDriveService : public InternetService {
  Q_OBJECT
 public:
  GoogleDriveService(Application* app, InternetModel* parent);
  ~GoogleDriveService();

  static const char* kServiceName;
  static const char* kSettingsGroup;

  google_drive::Client* client() const { return client_; }
  QString refresh_token() const;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);
  void ShowContextMenu(const QPoint& global_pos);

  QUrl GetStreamingUrlFromSongId(const QString& file_id);

 public slots:
  void Connect();
  void ForgetCredentials();

 signals:
  void Connected();

 private slots:
  void ConnectFinished(google_drive::ConnectResponse* response);
  void FilesFound(const QList<google_drive::File>& files);
  void ListFilesFinished(google_drive::ListFilesResponse* response);
  void ReadTagsFinished(TagReaderClient::ReplyType* reply,
                        const google_drive::File& metadata,
                        const QString& url,
                        const int task_id);

  void OpenWithDrive();
  void ShowSettingsDialog();
  void ShowCoverManager();
  void AddToPlaylist(QMimeData* mime);

 private:
  void EnsureConnected();
  void RefreshAuthorisation(const QString& refresh_token);
  void MaybeAddFileToDatabase(const google_drive::File& file);
  void ListFilesForMimeType(const QString& mime_type);

  QStandardItem* root_;

  google_drive::Client* client_;

  NetworkAccessManager network_;
  TaskManager* task_manager_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;
  PlaylistManager* playlist_manager_;

  int indexing_task_id_;

  boost::scoped_ptr<QMenu> context_menu_;
  QAction* open_in_drive_action_;

  boost::scoped_ptr<AlbumCoverManager> cover_manager_;
};

#endif
