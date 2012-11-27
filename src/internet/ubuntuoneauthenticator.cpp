#include "ubuntuoneauthenticator.h"

#include <QCoreApplication>
#include <QHostInfo>

#include <qjson/parser.h>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

namespace {
static const char* kUbuntuOneEndpoint =
    "https://login.ubuntu.com/api/1.0/authentications";
static const char* kTokenNameTemplate = "Ubuntu One @ %1 [%2]";
}

UbuntuOneAuthenticator::UbuntuOneAuthenticator(QObject* parent)
  : QObject(parent),
    network_(new NetworkAccessManager(this)) {
}

void UbuntuOneAuthenticator::StartAuthorisation(
    const QString& email,
    const QString& password) {
  QUrl url(kUbuntuOneEndpoint);
  url.addQueryItem("ws.op", "authenticate");
  QString token_name = QString(kTokenNameTemplate).arg(
      QHostInfo::localHostName(),
      QCoreApplication::applicationName());
  url.addQueryItem("token_name", token_name);

  QByteArray authentication = QString(email + ":" + password).toAscii().toBase64();
  QString authorisation =
      QString("Basic %1").arg(QString::fromAscii(authentication));

  QNetworkRequest request(url);
  request.setRawHeader("Authorization", authorisation.toAscii());
  request.setRawHeader("Accept", "application/json");

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(AuthorisationFinished(QNetworkReply*)), reply);

  qLog(Debug) << url;
  qLog(Debug) << authorisation;
}

void UbuntuOneAuthenticator::AuthorisationFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QByteArray data = reply->readAll();
  qLog(Debug) << data;

  QJson::Parser parser;
  bool ok = false;
  QVariant json = parser.parse(data, &ok);
  if (!ok) {
    qLog(Error) << "Failed to authenticate to Ubuntu One:" << parser.errorString();
    return;
  }

  QVariantMap auth_info = json.toMap();
  QString consumer_key = auth_info["consumer_key"].toString();
  QString consumer_secret = auth_info["consumer_secret"].toString();
  QString token = auth_info["token"].toString();
  QString token_secret = auth_info["token_secret"].toString();

  qLog(Debug)
      << consumer_key
      << consumer_secret
      << token
      << token_secret;
}
