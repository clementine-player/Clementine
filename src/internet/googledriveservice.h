#ifndef GOOGLEDRIVESERVICE_H
#define GOOGLEDRIVESERVICE_H

#include "internetservice.h"

#include "core/network.h"

class QStandardItem;

class OAuthenticator;

class GoogleDriveService : public InternetService {
  Q_OBJECT
 public:
  GoogleDriveService(Application* app, InternetModel* parent);

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

 private slots:
  void AccessTokenAvailable(const QString& token);
  void ListFilesFinished(QNetworkReply* reply);

 private:
  void Connect();

  QStandardItem* root_;
  OAuthenticator* oauth_;

  QString access_token_;

  NetworkAccessManager network_;
};

#endif
