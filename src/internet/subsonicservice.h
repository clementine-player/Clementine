#ifndef SUBSONICSERVICE_H
#define SUBSONICSERVICE_H

#include "internetmodel.h"
#include "internetservice.h"

class QNetworkAccessManager;
class QXmlStreamReader;

class SubsonicUrlHandler;
class SubsonicHttpsUrlHandler;

class SubsonicService : public InternetService
{
  Q_OBJECT
  Q_ENUMS(LoginState)
  Q_ENUMS(ApiError)

 public:
  SubsonicService(InternetModel *parent);
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
  smart_playlists::GeneratorPtr CreateGenerator(QStandardItem* item);
  void ReloadSettings();

  void Login();
  void Login(const QString &server, const QString &username, const QString &password);
  LoginState login_state() const { return login_state_; }

  // Subsonic API methods
  void Ping();
  void GetIndexes();
  void GetMusicDirectory(const QString &id);

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kApiVersion;
  static const char* kApiClientName;

 signals:
  void LoginStateChanged(SubsonicService::LoginState newstate);

 protected:
  QModelIndex GetCurrentIndex();

 private:
  QUrl BuildRequestUrl(const QString &view);
  // Convenience function to reduce QNetworkRequest/QNetworkReply/connect boilerplate
  void Send(const QUrl &url, const char *slot);

  void ReadIndex(QXmlStreamReader *reader, QStandardItem *parent);
  void ReadArtist(QXmlStreamReader *reader, QStandardItem *parent);
  void ReadAlbum(QXmlStreamReader *reader, QStandardItem *parent);
  void ReadTrack(QXmlStreamReader *reader, QStandardItem *parent);

  QModelIndex context_item_;
  QStandardItem* root_;
  QNetworkAccessManager* network_;
  SubsonicUrlHandler* http_url_handler_;
  SubsonicHttpsUrlHandler* https_url_handler_;

  // Configuration
  QString server_;
  QString username_;
  QString password_;

  LoginState login_state_;

  QMap<QString, QStandardItem*> item_lookup_;

 private slots:
  void onLoginStateChanged(SubsonicService::LoginState newstate);
  void onPingFinished();
  void onGetIndexesFinished();
  void onGetMusicDirectoryFinished();
};

#endif // SUBSONICSERVICE_H
