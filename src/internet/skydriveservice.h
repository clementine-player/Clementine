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
  QUrl GetStreamingUrlFromSongId(const QString& song_id);

 protected:
  // CloudFileService
  virtual bool has_credentials() const;
  virtual void Connect();

 private slots:
  void ConnectFinished(OAuthenticator* oauth);
  void FetchUserInfoFinished(QNetworkReply* reply);
  void ListFilesFinished(QNetworkReply* reply);

signals:
  void Connected();

 private:
  void AddAuthorizationHeader(QNetworkRequest* request);
  void ListFiles(const QString& folder);
  void EnsureConnected();

  QString access_token_;
  QDateTime expiry_time_;
};

#endif  // SKYDRIVESERVICE_H
