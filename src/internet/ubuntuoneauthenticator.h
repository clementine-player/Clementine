#ifndef UBUNTUONEAUTHENTICATOR_H
#define UBUNTUONEAUTHENTICATOR_H

#include <QObject>

class QNetworkReply;
class NetworkAccessManager;

class UbuntuOneAuthenticator : public QObject {
  Q_OBJECT
 public:
  explicit UbuntuOneAuthenticator(QObject* parent = 0);
  void StartAuthorisation(const QString& email, const QString& password);

 signals:
  void Finished();

 private slots:
  void AuthorisationFinished(QNetworkReply* reply);

 private:
  NetworkAccessManager* network_;
};

#endif   // UBUNTUONEAUTHENTICATOR_H
