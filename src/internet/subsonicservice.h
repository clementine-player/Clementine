#ifndef SUBSONICSERVICE_H
#define SUBSONICSERVICE_H

#include "internetmodel.h"
#include "internetservice.h"

class QNetworkAccessManager;
class QXmlStreamReader;
class QSortFilterProxyModel;

class SubsonicUrlHandler;

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
  void ReloadSettings();

  void Login();
  void Login(const QString &server, const QString &username, const QString &password);
  LoginState login_state() const { return login_state_; }

  // Subsonic API methods
  void Ping();
  void GetIndexes();
  void GetMusicDirectory(const QString &id);

  QUrl BuildRequestUrl(const QString &view);

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kApiVersion;
  static const char* kApiClientName;

  static const char* kSongsTable;
  static const char* kFtsTable;

  static const int kChunkSize;

 signals:
  void LoginStateChanged(SubsonicService::LoginState newstate);

 protected:
  QModelIndex GetCurrentIndex();

 private:
  // Convenience function to reduce QNetworkRequest/QNetworkReply/connect boilerplate
  void Send(const QUrl &url, const char *slot);

  void ReadIndex(QXmlStreamReader *reader);
  void ReadArtist(QXmlStreamReader *reader);
  void ReadAlbum(QXmlStreamReader *reader);
  Song ReadTrack(QXmlStreamReader *reader);

  QModelIndex context_item_;
  QNetworkAccessManager* network_;
  SubsonicUrlHandler* url_handler_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  QSortFilterProxyModel* library_sort_model_;

  // Configuration
  QString server_;
  QString username_;
  QString password_;

  LoginState login_state_;

  int directory_count_;
  int processed_directory_count_;
  SongList new_songs_;

 private slots:
  void onLoginStateChanged(SubsonicService::LoginState newstate);
  void onPingFinished();
  void onGetIndexesFinished();
  void onGetMusicDirectoryFinished();
};

#endif // SUBSONICSERVICE_H
