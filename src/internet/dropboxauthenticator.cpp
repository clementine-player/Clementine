#include "dropboxauthenticator.h"

#include <time.h>

#include <qjson/parser.h>

#include <QDesktopServices>
#include <QStringList>
#include <QTcpSocket>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

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
    : QObject(parent),
      network_(new NetworkAccessManager(this)) {
}

void DropboxAuthenticator::StartAuthorisation(const QString& email) {
  QUrl url(kRequestTokenEndpoint);
  typedef QPair<QString, QString> Param;

  QByteArray signature = QUrl::toPercentEncoding(QString(kAppSecret) + "&");
  QList<Param> params;
  params << Param("oauth_consumer_key", kAppKey)
         << Param("oauth_signature_method", "PLAINTEXT")
         << Param("oauth_timestamp", QString::number(time(NULL)))
         << Param("oauth_nonce", QString::number(qrand()))
         << Param("oauth_signature", signature);
  QStringList encoded_params;
  foreach (const Param& p, params) {
    encoded_params << QString("%1=\"%2\"").arg(p.first, p.second);
  }
  QString authorisation_header = QString("OAuth ") + encoded_params.join(", ");
  qLog(Debug) << authorisation_header;

  QNetworkRequest request(url);
  request.setRawHeader("Authorization", authorisation_header.toUtf8());

  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RequestTokenFinished(QNetworkReply*)), reply);
}

namespace {

// Parse a string like a=b&c=d into a map.
QMap<QString, QString> ParseParamList(const QString& params) {
  QMap<QString, QString> ret;
  QStringList components = params.split("&");
  foreach (const QString& component, components) {
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
  server_.listen(QHostAddress::LocalHost);
  const quint16 port = server_.serverPort();

  NewClosure(&server_, SIGNAL(newConnection()), this, SLOT(NewConnection()));

  QUrl url(kAuthoriseEndpoint);
  url.addQueryItem("oauth_token", token_);
  url.addQueryItem("oauth_callback", QString("http://localhost:%1").arg(port));

  QDesktopServices::openUrl(url);
}

void DropboxAuthenticator::NewConnection() {
  QTcpSocket* socket = server_.nextPendingConnection();
  server_.close();

  QByteArray buffer;
  NewClosure(socket, SIGNAL(readyRead()),
             this, SLOT(RedirectArrived(QTcpSocket*, QByteArray)), socket, buffer);
}

void DropboxAuthenticator::RedirectArrived(QTcpSocket* socket, QByteArray buffer) {
  buffer.append(socket->readAll());
  if (socket->atEnd() || buffer.endsWith("\r\n\r\n")) {
    socket->deleteLater();
    QList<QByteArray> split = buffer.split('\r');
    const QByteArray& request_line = split[0];
    QUrl url(QString::fromAscii(request_line.split(' ')[1]));
    uid_ = url.queryItemValue("uid");
    RequestAccessToken();
  }
}

void DropboxAuthenticator::RequestAccessToken() {
  QUrl url(kAccessTokenEndpoint);
  typedef QPair<QString, QString> Param;

  QByteArray signature = QUrl::toPercentEncoding(
      QString("%1&%2").arg(kAppSecret, secret_));
  QList<Param> params;
  params << Param("oauth_consumer_key", kAppKey)
         << Param("oauth_signature_method", "PLAINTEXT")
         << Param("oauth_timestamp", QString::number(time(NULL)))
         << Param("oauth_nonce", QString::number(qrand()))
         << Param("oauth_signature", signature)
         << Param("oauth_token", token_);
  QStringList encoded_params;
  foreach (const Param& p, params) {
    encoded_params << QString("%1=\"%2\"").arg(p.first, p.second);
  }
  QString authorisation_header = QString("OAuth ") + encoded_params.join(", ");
  qLog(Debug) << authorisation_header;
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", authorisation_header.toUtf8());

  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RequestAccessTokenFinished(QNetworkReply*)), reply);
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
  typedef QPair<QString, QString> Param;
  QByteArray signature = QUrl::toPercentEncoding(
      QString("%1&%2").arg(kAppSecret, access_token_secret_));
  QList<Param> params;
  params << Param("oauth_consumer_key", kAppKey)
         << Param("oauth_signature_method", "PLAINTEXT")
         << Param("oauth_timestamp", QString::number(time(NULL)))
         << Param("oauth_nonce", QString::number(qrand()))
         << Param("oauth_signature", signature)
         << Param("oauth_token", access_token_);
  QStringList encoded_params;
  foreach (const Param& p, params) {
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
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RequestAccountInformationFinished(QNetworkReply*)), reply);
}

void DropboxAuthenticator::RequestAccountInformationFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();
  name_ = response["display_name"].toString();
}
