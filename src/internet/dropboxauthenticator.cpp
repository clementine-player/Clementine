#include "dropboxauthenticator.h"

#include <time.h>

#include <qjson/parser.h>

#include <QDesktopServices>
#include <QStringList>
#include <QTcpSocket>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "internet/localredirectserver.h"

namespace {
static const char* kAppKey = "qh6ca27eclt9p2k";
static const char* kAppSecret = "pg7y68h5efap8r6";

// OAuth 1.0 endpoints
static const char* kRequestTokenEndpoint =
    "https://api.dropbox.com/1/oauth/request_token";
static const char* kAuthoriseEndpoint =
    "https://www.dropbox.com/1/oauth/authorize";
static const char* kAccessTokenEndpoint =
    "https://api.dropbox.com/1/oauth/access_token";

// Dropbox API endpoints
static const char* kAccountInfoEndpoint =
    "https://api.dropbox.com/1/account/info";

}  // namespace

DropboxAuthenticator::DropboxAuthenticator(QObject* parent)
    : QObject(parent), network_(new NetworkAccessManager(this)) {}

void DropboxAuthenticator::StartAuthorisation() {
  QUrl url(kRequestTokenEndpoint);
  QByteArray authorisation_header =
      GenerateAuthorisationHeader(QString::null, QString::null);
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", authorisation_header);

  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestTokenFinished(QNetworkReply*)), reply);
}

namespace {

// Parse a string like a=b&c=d into a map.
QMap<QString, QString> ParseParamList(const QString& params) {
  QMap<QString, QString> ret;
  QStringList components = params.split("&");
  for (const QString& component : components) {
    QStringList pairs = component.split("=");
    if (pairs.size() != 2) {
      continue;
    }
    ret[pairs[0]] = pairs[1];
  }
  return ret;
}
}

void DropboxAuthenticator::RequestTokenFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QString result = reply->readAll();
  QMap<QString, QString> params = ParseParamList(result);
  token_ = params["oauth_token"];
  secret_ = params["oauth_token_secret"];
  Authorise();
}

void DropboxAuthenticator::Authorise() {
  LocalRedirectServer* server = new LocalRedirectServer(this);
  server->Listen();

  NewClosure(server, SIGNAL(Finished()), this,
             SLOT(RedirectArrived(LocalRedirectServer*)), server);

  QUrl url(kAuthoriseEndpoint);
  url.addQueryItem("oauth_token", token_);
  url.addQueryItem("oauth_callback", server->url().toString());

  QDesktopServices::openUrl(url);
}

void DropboxAuthenticator::RedirectArrived(LocalRedirectServer* server) {
  server->deleteLater();
  QUrl request_url = server->request_url();
  qLog(Debug) << Q_FUNC_INFO << request_url;
  uid_ = request_url.queryItemValue("uid");
  RequestAccessToken();
}

void DropboxAuthenticator::RequestAccessToken() {
  QUrl url(kAccessTokenEndpoint);
  QNetworkRequest request(url);
  QByteArray authorisation_header =
      GenerateAuthorisationHeader(token_, secret_);
  request.setRawHeader("Authorization", authorisation_header);

  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestAccessTokenFinished(QNetworkReply*)), reply);
}

void DropboxAuthenticator::RequestAccessTokenFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QString result = QString::fromAscii(reply->readAll());
  qLog(Debug) << result;
  QMap<QString, QString> params = ParseParamList(result);
  access_token_ = params["oauth_token"];
  access_token_secret_ = params["oauth_token_secret"];
  qLog(Debug) << Q_FUNC_INFO << access_token_ << access_token_secret_;
  RequestAccountInformation();
}

QByteArray DropboxAuthenticator::GenerateAuthorisationHeader() {
  return GenerateAuthorisationHeader(access_token_, access_token_secret_);
}

QByteArray DropboxAuthenticator::GenerateAuthorisationHeader(
    const QString& token, const QString& token_secret) {
  typedef QPair<QString, QString> Param;
  QByteArray signature =
      QUrl::toPercentEncoding(QString("%1&%2").arg(kAppSecret, token_secret));
  QList<Param> params;
  params << Param("oauth_consumer_key", kAppKey)
         << Param("oauth_signature_method", "PLAINTEXT")
         << Param("oauth_timestamp", QString::number(time(nullptr)))
         << Param("oauth_nonce", QString::number(qrand()))
         << Param("oauth_signature", signature);
  if (!token.isNull()) {
    params << Param("oauth_token", token);
  }
  QStringList encoded_params;
  for (const Param& p : params) {
    encoded_params << QString("%1=\"%2\"").arg(p.first, p.second);
  }
  QString authorisation_header = QString("OAuth ") + encoded_params.join(", ");
  return authorisation_header.toUtf8();
}

void DropboxAuthenticator::RequestAccountInformation() {
  QUrl url(kAccountInfoEndpoint);
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  qLog(Debug) << Q_FUNC_INFO << url << request.rawHeader("Authorization");
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestAccountInformationFinished(QNetworkReply*)), reply);
}

void DropboxAuthenticator::RequestAccountInformationFinished(
    QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();
  name_ = response["display_name"].toString();
  emit Finished();
}
