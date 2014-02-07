#include "ubuntuoneauthenticator.h"

#include <time.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QHostInfo>
#include <QStringList>

#include <qjson/parser.h>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/timeconstants.h"

namespace {
static const char* kUbuntuOneEndpoint =
    "https://login.ubuntu.com/api/1.0/authentications";
static const char* kTokenNameTemplate = "Ubuntu One @ %1 [%2]";
static const char* kOAuthSSOFinishedEndpoint =
    "https://one.ubuntu.com/oauth/sso-finished-so-get-tokens/";
static const char* kOAuthHeaderPrefix = "OAuth realm=\"\", ";
}

UbuntuOneAuthenticator::UbuntuOneAuthenticator(QObject* parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)),
      success_(false) {}

void UbuntuOneAuthenticator::StartAuthorisation(const QString& email,
                                                const QString& password) {
  QUrl url(kUbuntuOneEndpoint);
  url.addQueryItem("ws.op", "authenticate");
  QString token_name = QString(kTokenNameTemplate).arg(
      QHostInfo::localHostName(), QCoreApplication::applicationName());
  url.addQueryItem("token_name", token_name);

  QByteArray authentication =
      QString(email + ":" + password).toAscii().toBase64();
  QString authorisation =
      QString("Basic %1").arg(QString::fromAscii(authentication));

  QNetworkRequest request(url);
  request.setRawHeader("Authorization", authorisation.toAscii());
  request.setRawHeader("Accept", "application/json");

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(AuthorisationFinished(QNetworkReply*)), reply);

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
    qLog(Error) << "Failed to authenticate to Ubuntu One:"
                << parser.errorString();
    emit Finished();
    return;
  }

  QVariantMap auth_info = json.toMap();
  consumer_key_ = auth_info["consumer_key"].toString();
  consumer_secret_ = auth_info["consumer_secret"].toString();
  token_ = auth_info["token"].toString();
  token_secret_ = auth_info["token_secret"].toString();

  CopySSOTokens();
}

QByteArray UbuntuOneAuthenticator::GenerateAuthorisationHeader(
    const QString& consumer_key, const QString& consumer_secret,
    const QString& token, const QString& token_secret) {
  typedef QPair<QString, QString> Param;
  QString timestamp = QString::number(time(nullptr));
  QList<Param> parameters;
  parameters << Param("oauth_nonce", QString::number(qrand()))
             << Param("oauth_timestamp", timestamp)
             << Param("oauth_version", "1.0")
             << Param("oauth_consumer_key", consumer_key)
             << Param("oauth_token", token)
             << Param("oauth_signature_method", "PLAINTEXT");
  qSort(parameters.begin(), parameters.end());
  QStringList encoded_params;
  foreach(const Param & p, parameters) {
    encoded_params << QString("%1=%2").arg(p.first, p.second);
  }

  QString signing_key = consumer_secret + "&" + token_secret;
  QByteArray signature = QUrl::toPercentEncoding(signing_key);

  // Construct authorisation header
  parameters << Param("oauth_signature", signature);
  QStringList header_params;
  foreach(const Param & p, parameters) {
    header_params << QString("%1=\"%2\"").arg(p.first, p.second);
  }
  QString authorisation_header = header_params.join(", ");
  authorisation_header.prepend(kOAuthHeaderPrefix);

  return authorisation_header.toAscii();
}

QByteArray UbuntuOneAuthenticator::GenerateAuthorisationHeader() {
  return GenerateAuthorisationHeader(consumer_key_, consumer_secret_, token_,
                                     token_secret_);
}

void UbuntuOneAuthenticator::CopySSOTokens() {
  QUrl url(kOAuthSSOFinishedEndpoint);
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  request.setRawHeader("Accept", "application/json");

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(CopySSOTokensFinished(QNetworkReply*)), reply);
}

void UbuntuOneAuthenticator::CopySSOTokensFinished(QNetworkReply* reply) {
  reply->deleteLater();
  qLog(Debug) << reply->readAll();
  success_ = true;
  emit Finished();
}
