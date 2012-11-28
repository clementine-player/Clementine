#ifndef UBUNTUONESERVICE_H
#define UBUNTUONESERVICE_H

#include "internet/internetservice.h"

#include "core/tagreaderclient.h"

class LibraryBackend;
class LibraryModel;
class NetworkAccessManager;
class QNetworkReply;
class QSortFilterProxyModel;
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
  void ReadTagsFinished(
      TagReaderClient::ReplyType* reply, const QVariantMap& file, const QUrl& url);

 private:
  void Connect();
  void RequestFileList(const QString& path);
  void MaybeAddFileToDatabase(const QVariantMap& file);
  void ShowSettingsDialog();

 private:
  QByteArray GenerateAuthorisationHeader();

  QStandardItem* root_;
  NetworkAccessManager* network_;

  QString consumer_key_;
  QString consumer_secret_;
  QString token_;
  QString token_secret_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;
};

#endif  // UBUNTUONESERVICE_H
