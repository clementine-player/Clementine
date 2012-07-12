#ifndef OAUTHENTICATOR_H
#define OAUTHENTICATOR_H

#include <QObject>
#include <QTcpServer>

#include "core/network.h"

class QTcpSocket;

class OAuthenticator : public QObject {
  Q_OBJECT
 public:
  explicit OAuthenticator(QObject* parent = 0);
  void StartAuthorisation();

 signals:
  void AccessTokenAvailable(QString token);

 private slots:
  void NewConnection();
  void RedirectArrived(QTcpSocket* socket, QByteArray buffer);
  void FetchAccessTokenFinished(QNetworkReply* reply);
  void ListFilesResponse(QNetworkReply* reply);

 private:
  QByteArray ParseHttpRequest(const QByteArray& request) const;
  void RequestAccessToken(const QByteArray& code, quint16 port);
  void ListFiles(const QString& access_token);

  QTcpServer server_;
  NetworkAccessManager network_;

  QString access_token_;
  QString refresh_token_;
};

#endif
