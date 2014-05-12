#ifndef SKYDRIVESERVICE_H
#define SKYDRIVESERVICE_H

#include "cloudfileservice.h"

#include <QDateTime>

class OAuthenticator;
class QNetworkRequest;
class QNetworkReply;

class SkydriveService : public CloudFileService {
  Q_OBJECT

 public:
  SkydriveService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;
  QUrl GetStreamingUrlFromSongId(const QString& song_id);

 public slots:
  virtual void Connect();
  void ForgetCredentials();

 private slots:
  void ConnectFinished(OAuthenticator* oauth);
  void FetchUserInfoFinished(QNetworkReply* reply);
  void ListFilesFinished(QNetworkReply* reply);

 signals:
  void Connected();

 private:
  QString refresh_token() const;
  void AddAuthorizationHeader(QNetworkRequest* request);
  void ListFiles(const QString& folder);
  void EnsureConnected();

  QString access_token_;
  QDateTime expiry_time_;
};

#endif  // SKYDRIVESERVICE_H
