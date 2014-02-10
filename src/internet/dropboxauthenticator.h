#ifndef DROPBOXAUTHENTICATOR_H
#define DROPBOXAUTHENTICATOR_H

#include <QObject>
#include <QTcpServer>

class LocalRedirectServer;
class NetworkAccessManager;
class QNetworkReply;

class DropboxAuthenticator : public QObject {
  Q_OBJECT
 public:
  explicit DropboxAuthenticator(QObject* parent = nullptr);
  void StartAuthorisation();

  const QString& access_token() const { return access_token_; }
  const QString& access_token_secret() const { return access_token_secret_; }
  const QString& uid() const { return uid_; }
  const QString& name() const { return name_; }

  static QByteArray GenerateAuthorisationHeader(const QString& token,
                                                const QString& secret);

signals:
  void Finished();

 private slots:
  void RequestTokenFinished(QNetworkReply* reply);
  void RedirectArrived(LocalRedirectServer* server);
  void RequestAccessTokenFinished(QNetworkReply* reply);
  void RequestAccountInformationFinished(QNetworkReply* reply);

 private:
  void Authorise();
  void RequestAccessToken();
  QByteArray GenerateAuthorisationHeader();
  void RequestAccountInformation();

 private:
  NetworkAccessManager* network_;
  QTcpServer server_;

  // Temporary access token used for first authentication flow.
  QString token_;
  QString secret_;

  // Permanent OAuth access tokens.
  QString access_token_;
  QString access_token_secret_;

  // User's Dropbox uid & name.
  QString uid_;
  QString name_;
};

#endif  // DROPBOXAUTHENTICATOR_H
