#ifndef UBUNTUONESERVICE_H
#define UBUNTUONESERVICE_H

#include "internet/internetservice.h"

class NetworkAccessManager;
class QNetworkReply;
class UbuntuOneAuthenticator;

class UbuntuOneService : public InternetService {
  Q_OBJECT
 public:
  UbuntuOneService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  // InternetService
  virtual QStandardItem* CreateRootItem();
  virtual void LazyPopulate(QStandardItem* parent);

  QUrl GetStreamingUrlFromSongId(const QString& song_id);

 private slots:
  void AuthenticationFinished(UbuntuOneAuthenticator* authenticator);
  void FileListRequestFinished(QNetworkReply* reply);

 private:
  void Connect();
  void RequestFileList();

 private:
  QByteArray GenerateAuthorisationHeader();

  QStandardItem* root_;
  NetworkAccessManager* network_;

  QString consumer_key_;
  QString consumer_secret_;
  QString token_;
  QString token_secret_;
};

#endif  // UBUNTUONESERVICE_H
