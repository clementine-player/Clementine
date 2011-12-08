#ifndef SUBSONICSERVICE_H
#define SUBSONICSERVICE_H

#include "internetservice.h"

class QNetworkAccessManager;

class SubsonicService : public InternetService
{
  Q_OBJECT

 public:
  SubsonicService(InternetModel *parent);
  ~SubsonicService();

  enum LoginState {
    LoginState_Loggedin,
    LoginState_BadServer,
    LoginState_BadCredentials,
    LoginState_Unlicensed,
    LoginState_OtherError,
    LoginState_Unknown,
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

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kApiVersion;
  static const char* kApiClientName;

 signals:
  void LoginStateChanged(SubsonicService::LoginState newstate);

 protected:
  QModelIndex GetCurrentIndex();

 private:
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

  // TODO: Remove second argument, let caller call addQueryItem()
  QUrl BuildRequestUrl(const QString &view, const RequestOptions *options = 0);

  QModelIndex context_item_;
  QStandardItem* root_;
  QNetworkAccessManager* network_;

  // Configuration
  QString server_;
  QString username_;
  QString password_;

  LoginState login_state_;

 private slots:
  void onPingFinished();
};

#endif // SUBSONICSERVICE_H
