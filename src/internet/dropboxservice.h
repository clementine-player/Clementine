#ifndef DROPBOXSERVICE_H
#define DROPBOXSERVICE_H

#include "internet/cloudfileservice.h"

#include "core/tagreaderclient.h"

class DropboxAuthenticator;
class NetworkAccessManager;
class QNetworkReply;

class DropboxService : public CloudFileService {
  Q_OBJECT
 public:
  DropboxService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;

  QUrl GetStreamingUrlFromSongId(const QUrl& url);

signals:
  void Connected();

 public slots:
  void Connect();
  void AuthenticationFinished(DropboxAuthenticator* authenticator);

 private slots:
  void RequestFileListFinished(QNetworkReply* reply);
  void FetchContentUrlFinished(QNetworkReply* reply, const QVariantMap& file);
  void LongPollFinished(QNetworkReply* reply);
  void LongPollDelta();

 private:
  void RequestFileList();
  QByteArray GenerateAuthorisationHeader();
  QNetworkReply* FetchContentUrl(const QUrl& url);

 private:
  QString access_token_;
  QString access_token_secret_;

  NetworkAccessManager* network_;
};

#endif  // DROPBOXSERVICE_H
