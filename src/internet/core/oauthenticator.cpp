/* This file is part of Clementine.
   Copyright 2012-2014, John Maguire <john.maguire@gmail.com>
   Copyright 2012, 2014, David Sansome <me@davidsansome.com>
   Copyright 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "internet/core/oauthenticator.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSslError>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

#include "core/closure.h"
#include "core/logging.h"
#include "internet/core/localredirectserver.h"

const char* OAuthenticator::kRemoteURL =
    "https://clementine-data.appspot.com/skydrive";

OAuthenticator::OAuthenticator(const QString& client_id,
                               const QString& client_secret,
                               RedirectStyle redirect, QObject* parent)
    : QObject(parent),
      client_id_(client_id),
      client_secret_(client_secret),
      redirect_style_(redirect) {}

void OAuthenticator::StartAuthorisation(const QString& oauth_endpoint,
                                        const QString& token_endpoint,
                                        const QString& scope) {
  token_endpoint_ = QUrl(token_endpoint);
  LocalRedirectServer* server = new LocalRedirectServer(this);
  server->Listen();

  QUrl url = QUrl(oauth_endpoint);
  QUrlQuery url_query;
  url_query.addQueryItem("response_type", "code");
  url_query.addQueryItem("client_id", client_id_);
  QUrl redirect_url;
  QUrlQuery redirect_url_query;

  const QString port = QString::number(server->url().port());

  if (redirect_style_ == RedirectStyle::REMOTE) {
    redirect_url = QUrl(kRemoteURL);
    redirect_url_query.addQueryItem("port", port);
  } else if (redirect_style_ == RedirectStyle::REMOTE_WITH_STATE) {
    redirect_url = QUrl(kRemoteURL);
    url_query.addQueryItem("state", port);
  } else {
    redirect_url = server->url();
  }

  url_query.addQueryItem("redirect_uri", redirect_url.toString());
  if (!scope.isEmpty()) {  // Empty scope is valid for Dropbox.
    url_query.addQueryItem("scope", scope);
  }

  url.setQuery(url_query);
  redirect_url.setQuery(redirect_url_query);

  NewClosure(server, SIGNAL(Finished()), this, &OAuthenticator::RedirectArrived,
             server, redirect_url);

  QDesktopServices::openUrl(url);
}

void OAuthenticator::RedirectArrived(LocalRedirectServer* server, QUrl url) {
  server->deleteLater();
  QUrl request_url = server->request_url();
  RequestAccessToken(QUrlQuery(request_url).queryItemValue("code").toUtf8(),
                     url);
}

QByteArray OAuthenticator::ParseHttpRequest(const QByteArray& request) const {
  QList<QByteArray> split = request.split('\r');
  const QByteArray& request_line = split[0];
  QByteArray path = request_line.split(' ')[1];
  QByteArray code = path.split('=')[1];

  return code;
}

void OAuthenticator::RequestAccessToken(const QByteArray& code,
                                        const QUrl& url) {
  typedef QPair<QString, QString> Param;
  QList<Param> parameters;
  parameters << Param("code", code) << Param("client_id", client_id_)
             << Param("client_secret", client_secret_)
             << Param("grant_type", "authorization_code")
             // Even though we don't use this URI anymore, it must match the
             // original one.
             << Param("redirect_uri", url.toString());

  QStringList params;
  for (const Param& p : parameters) {
    params.append(QString("%1=%2").arg(
        p.first, QString(QUrl::toPercentEncoding(p.second))));
  }
  QString post_data = params.join("&");
  qLog(Debug) << post_data;

  QNetworkRequest request = QNetworkRequest(QUrl(token_endpoint_));
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");

  QNetworkReply* reply = network_.post(request, post_data.toUtf8());
  connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
          SLOT(SslErrors(QList<QSslError>)));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchAccessTokenFinished(QNetworkReply*)), reply);
}

void OAuthenticator::FetchAccessTokenFinished(QNetworkReply* reply) {
  reply->deleteLater();

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
    qLog(Error) << "Failed to get access token" << reply->readAll();
    return;
  }

  QJsonParseError error;
  QJsonDocument json_document =
      QJsonDocument::fromJson(reply->readAll(), &error);

  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Failed to parse oauth reply";
    return;
  }

  QJsonObject result = json_document.object();
  access_token_ = result["access_token"].toString();
  refresh_token_ = result["refresh_token"].toString();
  SetExpiryTime(result["expires_in"].toInt());

  emit Finished();
}

void OAuthenticator::RefreshAuthorisation(const QString& token_endpoint,
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
  for (const Param& p : parameters) {
    params.append(QString("%1=%2").arg(
        p.first, QString(QUrl::toPercentEncoding(p.second))));
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

  QJsonObject json_result = QJsonDocument::fromJson(reply->readAll()).object();

  access_token_ = json_result["access_token"].toString();
  if (json_result.contains("refresh_token")) {
    refresh_token_ = json_result["refresh_token"].toString();
  }
  SetExpiryTime(json_result["expires_in"].toInt());
  emit Finished();
}

void OAuthenticator::SslErrors(const QList<QSslError>& errors) {
  for (const QSslError& error : errors) {
    qLog(Debug) << error.errorString();
  }
}
