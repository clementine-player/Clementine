#ifndef SUBSONICSERVICE_H
#define SUBSONICSERVICE_H

#include <QQueue>

#include "internet/internetmodel.h"
#include "internet/internetservice.h"

class QNetworkAccessManager;
class QNetworkReply;
class QSortFilterProxyModel;
class QXmlStreamReader;

class SubsonicUrlHandler;
class SubsonicLibraryScanner;

class SubsonicService : public InternetService {
  Q_OBJECT
  Q_ENUMS(LoginState)
  Q_ENUMS(ApiError)

 public:
  SubsonicService(Application* app, InternetModel* parent);
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
    LoginState_ConnectionRefused,
    LoginState_HostNotFound,
    LoginState_Timeout,
    LoginState_SslError,
    LoginState_IncompleteCredentials,
    LoginState_RedirectLimitExceeded,
    LoginState_RedirectNoUrl,
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

  enum Type { Type_Artist = InternetModel::TypeCount, Type_Album, Type_Track, };

  enum Role { Role_Id = InternetModel::RoleCount, };

  typedef QMap<QString, QString> RequestOptions;

  bool IsConfigured() const;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);
  void ShowContextMenu(const QPoint& global_pos);
  QWidget* HeaderWidget() const;
  void ReloadSettings();

  void Login();
  void Login(const QString& server, const QString& username,
             const QString& password, const bool& usesslv3);

  LoginState login_state() const { return login_state_; }

  // Subsonic API methods
  void Ping();

  QUrl BuildRequestUrl(const QString& view) const;
  // Scrubs the part of the path that we re-add in BuildRequestUrl().
  static QUrl ScrubUrl(const QUrl& url);
  // Convenience function to reduce QNetworkRequest/QSslConfiguration
  // boilerplate.
  QNetworkReply* Send(const QUrl& url);

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kApiVersion;
  static const char* kApiClientName;

  static const char* kSongsTable;
  static const char* kFtsTable;

  static const int kMaxRedirects;

signals:
  void LoginStateChanged(SubsonicService::LoginState newstate);

 private:
  void EnsureMenuCreated();
  // Update configured and working server state
  void UpdateServer(const QString& server);

  QNetworkAccessManager* network_;
  SubsonicUrlHandler* url_handler_;

  SubsonicLibraryScanner* scanner_;
  int load_database_task_id_;

  QMenu* context_menu_;
  QStandardItem* root_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  LibraryFilterWidget* library_filter_;
  QSortFilterProxyModel* library_sort_model_;
  int total_song_count_;

  // Configuration
  // The server that shows up in the GUI (use UpdateServer() to update)
  QString configured_server_;
  QString username_;
  QString password_;
  bool usesslv3_;

  LoginState login_state_;
  QString working_server_;  // The actual server, possibly post-redirect
  int redirect_count_;

 private slots:
  void UpdateTotalSongCount(int count);
  void ReloadDatabase();
  void ReloadDatabaseFinished();
  void OnLoginStateChanged(SubsonicService::LoginState newstate);
  void OnPingFinished(QNetworkReply* reply);

  void ShowConfig();
};

class SubsonicLibraryScanner : public QObject {
  Q_OBJECT

 public:
  SubsonicLibraryScanner(SubsonicService* service, QObject* parent = nullptr);
  ~SubsonicLibraryScanner();

  void Scan();
  const SongList& GetSongs() const { return songs_; }

  static const int kAlbumChunkSize;
  static const int kConcurrentRequests;

signals:
  void ScanFinished();

 private slots:
  // Step 1: use getAlbumList2 type=alphabeticalByName to list all albums
  void OnGetAlbumListFinished(QNetworkReply* reply, int offset);
  // Step 2: use getAlbum id=? to list all songs for each album
  void OnGetAlbumFinished(QNetworkReply* reply);

 private:
  void GetAlbumList(int offset);
  void GetAlbum(const QString& id);

  SubsonicService* service_;
  bool scanning_;
  QQueue<QString> album_queue_;
  QSet<QNetworkReply*> pending_requests_;
  SongList songs_;
};

#endif  // SUBSONICSERVICE_H
