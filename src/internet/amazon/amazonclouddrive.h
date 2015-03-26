#ifndef INTERNET_AMAZON_AMAZON_CLOUD_DRIVE_H_
#define INTERNET_AMAZON_AMAZON_CLOUD_DRIVE_H_

#include "internet/core/cloudfileservice.h"

#include <QDateTime>
#include <QString>
#include <QUrl>

class NetworkAccessManager;
class OAuthenticator;
class QNetworkReply;
class QNetworkRequest;

class AmazonCloudDrive : public CloudFileService {
  Q_OBJECT
 public:
  AmazonCloudDrive(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;

  QUrl GetStreamingUrlFromSongId(const QUrl& url);

  void ForgetCredentials();

 signals:
  void Connected();

 public slots:
  void Connect();

 private:
  void FetchEndpoint();
  void RequestChanges();
  void AddAuthorizationHeader(QNetworkRequest* request);

 private slots:
  void ConnectFinished(OAuthenticator*);
  void FetchEndpointFinished(QNetworkReply*);
  void RequestChangesFinished(QNetworkReply*);

 private:
  NetworkAccessManager* network_;
  QString access_token_;
  QDateTime expiry_time_;
  QString content_url_;
  QString metadata_url_;
};

#endif  // INTERNET_AMAZON_AMAZON_CLOUD_DRIVE_H_
