#ifndef BOXSERVICE_H
#define BOXSERVICE_H

#include "cloudfileservice.h"

#include <QDateTime>

class OAuthenticator;
class QNetworkReply;
class QNetworkRequest;

class BoxService : public CloudFileService {
  Q_OBJECT
 public:
  BoxService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  virtual bool has_credentials() const;
  QUrl GetStreamingUrlFromSongId(const QString& id);

 public slots:
  void Connect();
  void ForgetCredentials();

signals:
  void Connected();

 private slots:
  void ConnectFinished(OAuthenticator* oauth);
  void FetchUserInfoFinished(QNetworkReply* reply);
  void FetchFolderItemsFinished(QNetworkReply* reply, const int folder_id);
  void RedirectFollowed(QNetworkReply* reply, const Song& song,
                        const QString& mime_type);
  void InitialiseEventsFinished(QNetworkReply* reply);
  void FetchEventsFinished(QNetworkReply* reply);

 private:
  QString refresh_token() const;
  bool is_authenticated() const;
  void AddAuthorizationHeader(QNetworkRequest* request) const;
  void UpdateFiles();
  void FetchRecursiveFolderItems(const int folder_id, const int offset = 0);
  void UpdateFilesFromCursor(const QString& cursor);
  QNetworkReply* FetchContentUrlForFile(const QString& file_id);
  void InitialiseEventsCursor();
  void MaybeAddFileEntry(const QVariantMap& entry);
  void EnsureConnected();

  QString access_token_;
  QDateTime expiry_time_;
};

#endif  // BOXSERVICE_H
