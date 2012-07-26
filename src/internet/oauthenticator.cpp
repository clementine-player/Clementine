#include "oauthenticator.h"

#include <QDesktopServices>
#include <QStringList>
#include <QTcpSocket>
#include <QUrl>

#include <qjson/parser.h>

#include "core/closure.h"

namespace {

const char* kGoogleOAuthEndpoint = "https://accounts.google.com/o/oauth2/auth";
const char* kGoogleOAuthTokenEndpoint =
    "https://accounts.google.com/o/oauth2/token";

const char* kClientId = "679260893280.apps.googleusercontent.com";
const char* kClientSecret = "l3cWb8efUZsrBI4wmY3uKl6i";

}  // namespace

OAuthenticator::OAuthenticator(QObject* parent)
  : QObject(parent) {
}

void OAuthenticator::StartAuthorisation() {
  server_.listen(QHostAddress::LocalHost);
  const quint16 port = server_.serverPort();

  NewClosure(&server_, SIGNAL(newConnection()), this, SLOT(NewConnection()));

  QUrl url = QUrl(kGoogleOAuthEndpoint);
  url.addQueryItem("response_type", "code");
  url.addQueryItem("client_id", kClientId);
  url.addQueryItem("redirect_uri", QString("http://localhost:%1").arg(port));
  url.addQueryItem("scope", "https://www.googleapis.com/auth/drive.readonly");

  QDesktopServices::openUrl(url);
}

void OAuthenticator::NewConnection() {
  QTcpSocket* socket = server_.nextPendingConnection();
  server_.close();

  QByteArray buffer;

  NewClosure(socket, SIGNAL(readyRead()),
             this, SLOT(RedirectArrived(QTcpSocket*, QByteArray)), socket, buffer);

  // Everything is bon.
  socket->write("HTTP/1.0 200 OK\r\n");
  socket->flush();
}

void OAuthenticator::RedirectArrived(QTcpSocket* socket, QByteArray buffer) {
  buffer.append(socket->readAll());

  if (socket->atEnd() || buffer.endsWith("\r\n\r\n")) {
    socket->deleteLater();
    const QByteArray& code = ParseHttpRequest(buffer);
    qLog(Debug) << "Code:" << code;
    RequestAccessToken(code, socket->localPort());
  } else {
    NewClosure(socket, SIGNAL(readyReady()),
               this, SLOT(RedirectArrived(QTcpSocket*, QByteArray)), socket, buffer);
  }
}

QByteArray OAuthenticator::ParseHttpRequest(const QByteArray& request) const {
  QList<QByteArray> split = request.split('\r');
  const QByteArray& request_line = split[0];
  QByteArray path = request_line.split(' ')[1];
  QByteArray code = path.split('=')[1];

  return code;
}

void OAuthenticator::RequestAccessToken(const QByteArray& code, quint16 port) {
  typedef QPair<QString, QString> Param;
  QList<Param> parameters;
  parameters << Param("code", code)
             << Param("client_id", kClientId)
             << Param("client_secret", kClientSecret)
             << Param("grant_type", "authorization_code")
             // Even though we don't use this URI anymore, it must match the
             // original one.
             << Param("redirect_uri", QString("http://localhost:%1").arg(port));

  QStringList params;
  foreach (const Param& p, parameters) {
    params.append(QString("%1=%2").arg(p.first, QString(QUrl::toPercentEncoding(p.second))));
  }
  QString post_data = params.join("&");
  qLog(Debug) << post_data;

  QNetworkRequest request = QNetworkRequest(QUrl(kGoogleOAuthTokenEndpoint));
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");

  QNetworkReply* reply = network_.post(request, post_data.toUtf8());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchAccessTokenFinished(QNetworkReply*)), reply);
}

void OAuthenticator::FetchAccessTokenFinished(QNetworkReply* reply) {
  reply->deleteLater();

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
    qLog(Error) << "Failed to get access token"
                << reply->readAll();
    return;
  }

  QJson::Parser parser;
  bool ok = false;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Failed to parse oauth reply";
    return;
  }

  qLog(Debug) << result;

  access_token_ = result["access_token"].toString();
  refresh_token_ = result["refresh_token"].toString();

  emit AccessTokenAvailable(access_token_);
  emit RefreshTokenAvailable(refresh_token_);
}

void OAuthenticator::RefreshAuthorisation(const QString& refresh_token) {
  QUrl url = QUrl(kGoogleOAuthTokenEndpoint);

  typedef QPair<QString, QString> Param;
  QList<Param> parameters;
  parameters << Param("client_id", kClientId)
             << Param("client_secret", kClientSecret)
             << Param("grant_type", "refresh_token")
             << Param("refresh_token", refresh_token);
  QStringList params;
  foreach (const Param& p, parameters) {
    params.append(QString("%1=%2").arg(p.first, QString(QUrl::toPercentEncoding(p.second))));
  }
  QString post_data = params.join("&");
  qLog(Debug) << "Refresh post data:" << post_data;

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QNetworkReply* reply = network_.post(request, post_data.toUtf8());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RefreshAccessTokenFinished(QNetworkReply*)), reply);
}

void OAuthenticator::RefreshAccessTokenFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  bool ok = false;

  QVariantMap result = parser.parse(reply, &ok).toMap();
  QString access_token = result["access_token"].toString();
  emit AccessTokenAvailable(access_token);
}
