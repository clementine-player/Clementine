#ifndef SUBSONICSERVICE_H
#define SUBSONICSERVICE_H

#include "internetmodel.h"
#include "internetservice.h"

#include <QQueue>

class QNetworkAccessManager;
class QXmlStreamReader;
class QSortFilterProxyModel;
class QNetworkReply;

class SubsonicUrlHandler;
class SubsonicLibraryScanner;

class SubsonicService : public InternetService
{
  Q_OBJECT
  Q_ENUMS(LoginState)
  Q_ENUMS(ApiError)

 public:
  SubsonicService(Application* app, InternetModel *parent);
  ~SubsonicService();

  enum LoginState {
    LoginState_Loggedin,
    LoginState_BadServer,
    LoginState_OutdatedClient,
    LoginState_OutdatedServer,
    LoginState_BadCredentials,
    LoginState_Unlicensed,
    LoginState_OtherError,
    LoginState_Unknown,
  };

  enum ApiError {
    ApiError_Generic = 0,
    ApiError_ParameterMissing = 10,
    ApiError_OutdatedClient = 20,
    ApiError_OutdatedServer = 30,
    ApiError_BadCredentials = 40,
    ApiError_Unauthorized = 50,
    ApiError_Unlicensed = 60,
    ApiError_NotFound = 70,
  };

  enum Type {
    Type_Artist = InternetModel::TypeCount,
    Type_Album,
    Type_Track,
  };

  enum Role {
    Role_Id = InternetModel::RoleCount,
  };

  typedef QMap<QString, QString> RequestOptions;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem *item);
  void ShowContextMenu(const QPoint &global_pos);
  QWidget* HeaderWidget() const;
  void ReloadSettings();

  void Login();
  void Login(const QString &server, const QString &username, const QString &password);
  LoginState login_state() const { return login_state_; }

  // Subsonic API methods
  void Ping();
  void GetIndexes();
  void GetMusicDirectory(const QString &id);

  QUrl BuildRequestUrl(const QString &view);
  // Convenience function to reduce QNetworkRequest/QSslConfiguration boilerplate
  QNetworkReply* Send(const QUrl &url);

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kApiVersion;
  static const char* kApiClientName;

  static const char* kSongsTable;
  static const char* kFtsTable;

 signals:
  void LoginStateChanged(SubsonicService::LoginState newstate);

 private:
  void EnsureMenuCreated();

  QNetworkAccessManager* network_;
  SubsonicUrlHandler* url_handler_;
  SubsonicLibraryScanner* scanner_;

  QMenu* context_menu_;
  QStandardItem* root_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  LibraryFilterWidget* library_filter_;
  QSortFilterProxyModel* library_sort_model_;

  // Configuration
  QString server_;
  QString username_;
  QString password_;

  LoginState login_state_;

 private slots:
  void onLoginStateChanged(SubsonicService::LoginState newstate);
  void onPingFinished(QNetworkReply* reply);
  void onSongsDiscovered(SongList songs);
};

class SubsonicLibraryScanner : public QObject {
  Q_OBJECT

 public:
  SubsonicLibraryScanner(SubsonicService* service, QObject* parent=0);
  ~SubsonicLibraryScanner();

  void Scan();

  static const int kAlbumChunkSize;
  static const int kSongListMinChunkSize;
  static const int kConcurrentRequests;

 signals:
  void SongsDiscovered(SongList);

 private slots:
  // Step 1: use getAlbumList2 type=alphabeticalByName to list all albums
  void onGetAlbumListFinished(QNetworkReply* reply, int offset);
  // Step 2: use getAlbum id=? to list all songs for each album
  void onGetAlbumFinished(QNetworkReply* reply);

 private:
  void GetAlbumList(int offset);
  void GetAlbum(QString id);

  SubsonicService* service_;
  QQueue<QString> album_queue_;
  SongList songlist_buffer_;
};

#endif // SUBSONICSERVICE_H
