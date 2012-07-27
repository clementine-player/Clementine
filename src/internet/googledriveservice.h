#ifndef GOOGLEDRIVESERVICE_H
#define GOOGLEDRIVESERVICE_H

#include "internetservice.h"

#include "core/network.h"

class QStandardItem;

class LibraryBackend;
class LibraryModel;
class OAuthenticator;
class QSortFilterProxyModel;

class GoogleDriveService : public InternetService {
  Q_OBJECT
 public:
  GoogleDriveService(Application* app, InternetModel* parent);

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

  QUrl GetStreamingUrlFromSongId(const QString& file_id);

 private slots:
  void AccessTokenAvailable(const QString& token);
  void RefreshTokenAvailable(const QString& token);
  void ListFilesFinished(QNetworkReply* reply);

 private:
  void Connect();
  void RefreshAuthorisation(const QString& refresh_token);
  void MaybeAddFileToDatabase(const QVariantMap& file);

  QStandardItem* root_;
  OAuthenticator* oauth_;

  QString access_token_;

  NetworkAccessManager network_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;

  int indexing_task_id_;
};

#endif
