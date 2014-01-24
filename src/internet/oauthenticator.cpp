#include "oauthenticator.h"

#include <QDesktopServices>
#include <QStringList>
#include <QUrl>

#include <qjson/parser.h>

#include "core/closure.h"
#include "core/logging.h"
#include "internet/localredirectserver.h"

OAuthenticator::OAuthenticator(
    const QString& client_id,
    const QString& client_secret,
    RedirectStyle redirect,
    QObject* parent)
  : QObject(parent),
    client_id_(client_id),
    client_secret_(client_secret),
    redirect_style_(redirect) {
}

void OAuthenticator::StartAuthorisation(
    const QString& oauth_endpoint,
    const QString& token_endpoint,
    const QString& scope) {
  token_endpoint_ = QUrl(token_endpoint);
  LocalRedirectServer* server = new LocalRedirectServer(this);
  server->Listen();

  QUrl url = QUrl(oauth_endpoint);
  url.addQueryItem("response_type", "code");
  url.addQueryItem("client_id", client_id_);
  QUrl redirect_url;
  if (redirect_style_ == RedirectStyle::REMOTE) {
    const int port = server->url().port();
    redirect_url = QUrl(
        QString("https://clementine-data.appspot.com/skydrive?port=%1").arg(port));
  } else {
    redirect_url = server->url();
  }
  url.addQueryItem("redirect_uri", redirect_url.toString());
  url.addQueryItem("scope", scope);

  NewClosure(server, SIGNAL(Finished()),
             this, &OAuthenticator::RedirectArrived, server, redirect_url);

  QDesktopServices::openUrl(url);
}

void OAuthenticator::RedirectArrived(
    LocalRedirectServer* server, QUrl url) {
  server->deleteLater();
  QUrl request_url = server->request_url();
  qLog(Debug) << Q_FUNC_INFO << request_url;
  RequestAccessToken(
      request_url.queryItemValue("code").toUtf8(),
      url);
}

QByteArray OAuthenticator::ParseHttpRequest(const QByteArray& request) const {
  QList<QByteArray> split = request.split('\r');
  const QByteArray& request_line = split[0];
  QByteArray path = request_line.split(' ')[1];
  QByteArray code = path.split('=')[1];

  return code;
}

void OAuthenticator::RequestAccessToken(const QByteArray& code, const QUrl& url) {
  typedef QPair<QString, QString> Param;
  QList<Param> parameters;
  parameters << Param("code", code)
             << Param("client_id", client_id_)
             << Param("client_secret", client_secret_)
             << Param("grant_type", "authorization_code")
             // Even though we don't use this URI anymore, it must match the
             // original one.
             << Param("redirect_uri", url.toString());

  QStringList params;
  foreach (const Param& p, parameters) {
    params.append(QString("%1=%2").arg(p.first, QString(QUrl::toPercentEncoding(p.second))));
  }
  QString post_data = params.join("&");
  qLog(Debug) << post_data;

  QNetworkRequest request = QNetworkRequest(QUrl(token_endpoint_));
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
  SetExpiryTime(result["expires_in"].toInt());

  emit Finished();
}

void OAuthenticator::RefreshAuthorisation(
    const QString& token_endpoint,
    const QString& refresh_token) {
  refresh_token_ = refresh_token;

  QUrl url(token_endpoint);

  typedef QPair<QString, QString> Param;
  QList<Param> parameters;
  parameters << Param("client_id", client_id_)
             << Param("client_secret", client_secret_)
             << Param("grant_type", "refresh_token")
             << Param("refresh_token", refresh_token);
  QStringList params;
  foreach (const Param& p, parameters) {
    params.append(QString("%1=%2").arg(p.first, QString(QUrl::toPercentEncoding(p.second))));
  }
  QString post_data = params.join("&");

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QNetworkReply* reply = network_.post(request, post_data.toUtf8());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RefreshAccessTokenFinished(QNetworkReply*)), reply);
}

void OAuthenticator::SetExpiryTime(int expires_in_seconds) {
  // Set the expiry time with two minutes' grace.
  expiry_time_ = QDateTime::currentDateTime().addSecs(expires_in_seconds - 120);
  qLog(Debug) << "Current oauth access token expires at:" << expiry_time_;
}

void OAuthenticator::RefreshAccessTokenFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  bool ok = false;

  QVariantMap result = parser.parse(reply, &ok).toMap();
  access_token_ = result["access_token"].toString();
  if (result.contains("refresh_token")) {
    refresh_token_ = result["refresh_token"].toString();
  }
  SetExpiryTime(result["expires_in"].toInt());
  emit Finished();
}
