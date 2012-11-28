#ifndef UBUNTUONESERVICE_H
#define UBUNTUONESERVICE_H

#include "internet/internetservice.h"

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

class UbuntuOneService : public InternetService {
  Q_OBJECT
 public:
  UbuntuOneService(Application* app, InternetModel* parent);

  static const char* kServiceName;
  static const char* kSettingsGroup;

  // InternetService
  virtual QStandardItem* CreateRootItem();
  virtual void LazyPopulate(QStandardItem* parent);
  virtual void ShowContextMenu(const QPoint& global_pos);

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

  boost::scoped_ptr<QMenu> context_menu_;
  boost::scoped_ptr<AlbumCoverManager> cover_manager_;
  PlaylistManager* playlist_manager_;
};

#endif  // UBUNTUONESERVICE_H
