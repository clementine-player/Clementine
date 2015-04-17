/* This file is part of Clementine.
   Copyright 2014, Vlad Maltsev <shedwardx@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, Ivan Leontiev <leont.ivan@gmail.com>

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

#include "vkconnection.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "core/closure.h"
#include "core/logging.h"
#include "internet/core/localredirectserver.h"
#include "vreen/utils.h"

#include "vkservice.h"

static const QUrl kVkOAuthEndpoint("https://oauth.vk.com/authorize");
static const QUrl kVkOAuthTokenEndpoint("https://oauth.vk.com/access_token");
static const QUrl kApiUrl("https://api.vk.com/method/");
static const char* kScopeNames[] = {
    "notify", "friends",  "photos",        "audio",  "video",     "docs",
    "notes",  "pages",    "status",        "offers", "questions", "wall",
    "groups", "messages", "notifications", "stats",  "ads",       "offline"};

static const QString kAppID = "3421812";
static const QString kAppSecret = "cY7KMyX46Fq3nscZlbdo";
static const VkConnection::Scopes kScopes =
    VkConnection::Offline | VkConnection::Audio | VkConnection::Friends |
    VkConnection::Groups | VkConnection::Status;

static const char* kSettingsGroup = "Vk.com/oauth";

VkConnection::VkConnection(QObject* parent)
    : Connection(parent),
      state_(Vreen::Client::StateOffline),
      expires_in_(0),
      uid_(0) {
  loadToken();
}

VkConnection::~VkConnection() {}

void VkConnection::connectToHost(const QString& login,
                                 const QString& password) {
  Q_UNUSED(login)
  Q_UNUSED(password)
  if (hasAccount()) {
    setConnectionState(Vreen::Client::StateOnline);
  } else {
    setConnectionState(Vreen::Client::StateConnecting);
    requestAccessToken();
  }
}

void VkConnection::disconnectFromHost() {
  clear();
  setConnectionState(Vreen::Client::StateOffline);
}

void VkConnection::clear() {
  access_token_.clear();
  expires_in_ = 0;
  uid_ = 0;

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("access_token");
  s.remove("expires_in");
  s.remove("uid");
}

bool VkConnection::hasAccount() {
  return !access_token_.isNull() &&
         (expires_in_ >
          static_cast<time_t>(QDateTime::currentDateTime().toTime_t()));
}

QNetworkRequest VkConnection::makeRequest(const QString& method,
                                          const QVariantMap& args) {
  QUrl url = kApiUrl;
  url.setPath(url.path() + "/" + method);
  QUrlQuery url_query;
  for (auto it = args.constBegin(); it != args.constEnd(); ++it) {
    url_query.addQueryItem(QUrl::toPercentEncoding(it.key()),
                            QUrl::toPercentEncoding(it.value().toString()));
  }
  url_query.addQueryItem("access_token", access_token_);
  url.setQuery(url_query);
  return QNetworkRequest(url);
}

void VkConnection::decorateRequest(QNetworkRequest& request) {
  QUrl url = request.url();
  QUrlQuery url_query(url);
  url_query.addQueryItem("access_token", access_token_);
  url.setQuery(url_query);
  request.setUrl(url);
}

void VkConnection::requestAccessToken() {
  LocalRedirectServer* server = new LocalRedirectServer(this);
  server->Listen();

  QUrl url = kVkOAuthEndpoint;
  QUrlQuery url_query;
  url_query.addQueryItem("client_id", kAppID);
  url_query.addQueryItem("scope",
                   Vreen::flagsToStrList(kScopes, kScopeNames).join(","));
  url_query.addQueryItem("redirect_uri", server->url().toString());
  url_query.addQueryItem("response_type", "code");
  url.setQuery(url_query);

  qLog(Debug) << "Try to login to Vk.com" << url;

  NewClosure(server, SIGNAL(Finished()), this,
             SLOT(codeRecived(LocalRedirectServer*, QUrl)), server,
             server->url());
  QDesktopServices::openUrl(url);
}

void VkConnection::codeRecived(LocalRedirectServer* server, QUrl redirect_uri) {
  QUrlQuery url_query_server(server->request_url());
  if (url_query_server.hasQueryItem("code")) {
    code_ = url_query_server.queryItemValue("code").toUtf8();
    QUrl url = kVkOAuthTokenEndpoint;
    QUrlQuery url_query;
    url_query.addQueryItem("client_id", kAppID);
    url_query.addQueryItem("client_secret", kAppSecret);
    url_query.addQueryItem("code", QString::fromUtf8(code_));
    url_query.addQueryItem("redirect_uri", redirect_uri.toString());
    url.setQuery(url_query);
    qLog(Debug) << "Getting access token" << url;

    QNetworkRequest request(url);
    QNetworkReply* reply = network_.get(request);

    NewClosure(reply, SIGNAL(finished()), this,
               SLOT(accessTokenRecived(QNetworkReply*)), reply);
  } else {
    qLog(Error) << "Login failed" << server->request_url();
    clear();
    emit connectionStateChanged(Vreen::Client::StateOffline);
  }
}

void VkConnection::accessTokenRecived(QNetworkReply* reply) {
  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
    qLog(Error) << "Failed to get access token" << reply->readAll();
    emit setConnectionState(Vreen::Client::StateOffline);
    clear();
    return;
  }

  QByteArray reply_data = reply->readAll();
  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(reply_data, &error);
  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Failed to parse oauth reply" << reply_data;
    emit setConnectionState(Vreen::Client::StateOffline);
    clear();
    return;
  }

  QJsonObject json_result = document.object();
  qLog(Debug) << json_result;

  access_token_ = json_result["access_token"].toString().toLatin1();
  expires_in_ = json_result["expires_in"].toString().toUInt();
  uid_ = json_result["user_id"].toInt();

  if (expires_in_) {
    expires_in_ += QDateTime::currentDateTime().toTime_t();
  } else {
    expires_in_ += QDateTime::currentDateTime().addMonths(1).toTime_t();
  }
  qLog(Debug) << "Access token expires in" << expires_in_;

  saveToken();
  setConnectionState(Vreen::Client::StateOnline);

  reply->deleteLater();
}

void VkConnection::setConnectionState(Vreen::Client::State state) {
  if (state != state_) {
    state_ = state;
    emit connectionStateChanged(state);
  }
}

void VkConnection::saveToken() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("access_token", access_token_);
  s.setValue("expires_in", QVariant::fromValue(expires_in_));
  s.setValue("uid", uid_);
}

void VkConnection::loadToken() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  access_token_ = s.value("access_token").toByteArray();
  expires_in_ = s.value("expires_in").toUInt();
  uid_ = s.value("uid").toInt();
}
