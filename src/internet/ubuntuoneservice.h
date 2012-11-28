#ifndef UBUNTUONESERVICE_H
#define UBUNTUONESERVICE_H

#include "internet/cloudfileservice.h"

#include <QMenu>

#include "core/tagreaderclient.h"
#include "ui/albumcovermanager.h"

class LibraryBackend;
class LibraryModel;
class NetworkAccessManager;
class PlaylistManager;
class QNetworkReply;
class QSortFilterProxyModel;
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
  void ReadTagsFinished(
      TagReaderClient::ReplyType* reply, const QVariantMap& file, const QUrl& url);
  void ShowSettingsDialog();
  void ShowCoverManager();
  void AddToPlaylist(QMimeData* mime);

 private:
  void Connect();
  void RequestFileList(const QString& path);
  void MaybeAddFileToDatabase(const QVariantMap& file);
  bool has_credentials() const;

 private:
  QByteArray GenerateAuthorisationHeader();

  QString consumer_key_;
  QString consumer_secret_;
  QString token_;
  QString token_secret_;
};

#endif  // UBUNTUONESERVICE_H
