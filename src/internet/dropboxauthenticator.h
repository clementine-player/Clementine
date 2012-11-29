#ifndef DROPBOXAUTHENTICATOR_H
#define DROPBOXAUTHENTICATOR_H

#include <QObject>
#include <QTcpServer>

class NetworkAccessManager;
class QNetworkReply;

class DropboxAuthenticator : public QObject {
  Q_OBJECT
 public:
  explicit DropboxAuthenticator(QObject* parent = 0);
  void StartAuthorisation(const QString& email);

 private slots:
  void RequestTokenFinished(QNetworkReply* reply);
  void RedirectArrived(QTcpSocket* socket, QByteArray buffer);
  void NewConnection();
  void RequestAccessTokenFinished(QNetworkReply* reply);

 private:
  void Authorise();
  void RequestAccessToken();

 private:
  NetworkAccessManager* network_;
  QTcpServer server_;

  QString token_;
  QString secret_;
  QString uid_;
};

#endif  // DROPBOXAUTHENTICATOR_H
