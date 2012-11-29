#ifndef DROPBOXAUTHENTICATOR_H
#define DROPBOXAUTHENTICATOR_H

#include <QObject>

class NetworkAccessManager;
class QNetworkReply;

class DropboxAuthenticator : public QObject {
  Q_OBJECT
 public:
  explicit DropboxAuthenticator(QObject* parent = 0);
  void StartAuthorisation(const QString& email);

 private slots:
  void RequestTokenFinished(QNetworkReply* reply);

 private:
  NetworkAccessManager* network_;
};

#endif  // DROPBOXAUTHENTICATOR_H
