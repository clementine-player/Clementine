/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "plexauthenticator.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

#include "core/closure.h"
#include "core/logging.h"
#include "internet/core/localredirectserver.h"
#include "internet/plex/plexservice.h"

namespace {

const char* kPinsUrl = "https://plex.tv/api/v2/pins";
const char* kResourcesUrl = "https://plex.tv/api/v2/resources";
const char* kAuthUrl = "https://app.plex.tv/auth";

QNetworkRequest PlexTvRequest(const QUrl& url, const QString& token) {
  QNetworkRequest request(url);
  PlexService::AddClientHeaders(&request, token);
  return request;
}

}  // namespace

PlexAuthenticator::PlexAuthenticator(QObject* parent)
    : QObject(parent),
      network_(new QNetworkAccessManager(this)),
      redirect_server_(nullptr),
      pin_id_(0) {}

QUrl PlexAuthenticator::AuthUrl(const QString& client_id, const QString& code,
                                const QUrl& forward_url) {
  // app.plex.tv reads its parameters from a query string inside the fragment.
  QUrlQuery params;
  params.addQueryItem("clientID", client_id);
  params.addQueryItem("code", code);
  if (forward_url.isValid()) {
    params.addQueryItem(
        "forwardUrl",
        QString::fromLatin1(QUrl::toPercentEncoding(forward_url.toString())));
  }
  params.addQueryItem("context%5Bdevice%5D%5Bproduct%5D", "Clementine");

  QUrl url(kAuthUrl);
  url.setFragment("?" + params.query(QUrl::FullyEncoded), QUrl::TolerantMode);
  return url;
}

void PlexAuthenticator::StartSignIn() {
  pin_id_ = 0;

  QUrlQuery query;
  query.addQueryItem("strong", "true");
  QUrl url(kPinsUrl);
  url.setQuery(query);

  QNetworkRequest request = PlexTvRequest(url, QString());
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, &QNetworkReply::finished, this,
             &PlexAuthenticator::PinCreated, reply);
}

void PlexAuthenticator::PinCreated(QNetworkReply* reply) {
  reply->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    emit Failed(tr("Couldn't reach plex.tv: %1").arg(reply->errorString()));
    return;
  }

  const QJsonObject pin = QJsonDocument::fromJson(reply->readAll()).object();
  pin_id_ = pin["id"].toInteger();
  const QString code = pin["code"].toString();
  if (!pin_id_ || code.isEmpty()) {
    emit Failed(tr("plex.tv sent an unexpected response"));
    return;
  }

  if (redirect_server_) redirect_server_->deleteLater();
  redirect_server_ = new LocalRedirectServer(this);
  redirect_server_->Listen();
  connect(redirect_server_, SIGNAL(Finished()), SLOT(RedirectReceived()));

  emit AuthCodeReady(code);
  QDesktopServices::openUrl(
      AuthUrl(PlexService::ClientId(), code, redirect_server_->url()));
}

void PlexAuthenticator::RedirectReceived() { CheckPin(); }

void PlexAuthenticator::CheckPin() {
  if (!pin_id_) return;

  QNetworkReply* reply = network_->get(PlexTvRequest(
      QUrl(QString("%1/%2").arg(kPinsUrl).arg(pin_id_)), QString()));
  NewClosure(reply, &QNetworkReply::finished, this,
             &PlexAuthenticator::PinChecked, reply);
}

void PlexAuthenticator::PinChecked(QNetworkReply* reply) {
  reply->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    // A 404 means the PIN expired.
    emit Failed(tr("The sign-in request expired. Please try again."));
    pin_id_ = 0;
    return;
  }

  const QJsonObject pin = QJsonDocument::fromJson(reply->readAll()).object();
  const QString token = pin["authToken"].toString();
  if (token.isEmpty()) {
    emit Failed(
        tr("Plex hasn't confirmed the sign-in yet. Approve Clementine "
           "in your browser, then try again."));
    return;
  }

  pin_id_ = 0;
  emit Authenticated(token);
}

void PlexAuthenticator::FetchServers(const QString& user_token) {
  QUrlQuery query;
  query.addQueryItem("includeHttps", "1");
  query.addQueryItem("includeRelay", "1");
  QUrl url(kResourcesUrl);
  url.setQuery(query);

  QNetworkReply* reply = network_->get(PlexTvRequest(url, user_token));
  NewClosure(reply, &QNetworkReply::finished, this,
             &PlexAuthenticator::ResourcesFetched, reply);
}

void PlexAuthenticator::ResourcesFetched(QNetworkReply* reply) {
  reply->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    const int status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    emit Failed(status == 401
                    ? tr("Your Plex sign-in has expired. Please sign in again.")
                    : tr("Couldn't fetch your Plex servers: %1")
                          .arg(reply->errorString()));
    return;
  }

  emit ServersFound(
      PlexParser::ParseResources(QJsonDocument::fromJson(reply->readAll())));
}
