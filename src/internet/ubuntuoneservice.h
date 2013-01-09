#ifndef UBUNTUONESERVICE_H
#define UBUNTUONESERVICE_H

#include "internet/cloudfileservice.h"

#include "core/tagreaderclient.h"

class QNetworkReply;
class UbuntuOneAuthenticator;

class UbuntuOneService : public CloudFileService {
  Q_OBJECT
 public:
  UbuntuOneService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  QUrl GetStreamingUrlFromSongId(const QString& song_id);

 private slots:
  void AuthenticationFinished(UbuntuOneAuthenticator* authenticator);
  void FileListRequestFinished(QNetworkReply* reply);
  void ShowCoverManager();
  void AddToPlaylist(QMimeData* mime);
  void VolumeListRequestFinished(QNetworkReply* reply);

 private:
  void Connect();
  QNetworkReply* SendRequest(const QUrl& url);
  void RequestVolumeList();
  void RequestFileList(const QString& path);
  bool has_credentials() const;

 private:
  QByteArray GenerateAuthorisationHeader();

  QString consumer_key_;
  QString consumer_secret_;
  QString token_;
  QString token_secret_;
};

#endif  // UBUNTUONESERVICE_H
