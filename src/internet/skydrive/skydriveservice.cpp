/* This file is part of Clementine.
   Copyright 2012-2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#include "skydriveservice.h"

#include <memory>

#include <qjson/parser.h>

#include "core/application.h"
#include "core/player.h"
#include "core/waitforsignal.h"
#include "internet/core/oauthenticator.h"
#include "internet/skydrive/skydriveurlhandler.h"
#include "ui/iconloader.h"

namespace {

static const char* kServiceId = "skydrive";

static const char* kClientId = "0000000040111F16";
static const char* kClientSecret = "w2ClguSX0jG56cBl1CeUniypTBRjXt2Z";

static const char* kOAuthEndpoint =
    "https://login.live.com/oauth20_authorize.srf";
static const char* kOAuthTokenEndpoint =
    "https://login.live.com/oauth20_token.srf";
static const char* kOAuthScope = "wl.basic wl.skydrive wl.offline_access";

static const char* kLiveUserInfo = "https://apis.live.net/v5.0/me";
static const char* kSkydriveBase = "https://apis.live.net/v5.0/";

}  // namespace

const char* SkydriveService::kServiceName = "OneDrive";
const char* SkydriveService::kSettingsGroup = "Skydrive";

SkydriveService::SkydriveService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       IconLoader::Load("skydrive", IconLoader::provider),
                       SettingsDialog::Page_Skydrive) {
  app->player()->RegisterUrlHandler(new SkydriveUrlHandler(this, this));
}

bool SkydriveService::has_credentials() const {
  return !refresh_token().isEmpty();
}

QString SkydriveService::refresh_token() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  return s.value("refresh_token").toString();
}

void SkydriveService::Connect() {
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret, OAuthenticator::RedirectStyle::REMOTE, this);
  if (!refresh_token().isEmpty()) {
    oauth->RefreshAuthorisation(kOAuthTokenEndpoint, refresh_token());
  } else {
    oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint, kOAuthScope);
  }

  NewClosure(oauth, SIGNAL(Finished()), this,
             SLOT(ConnectFinished(OAuthenticator*)), oauth);
}

void SkydriveService::ConnectFinished(OAuthenticator* oauth) {
  oauth->deleteLater();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", oauth->refresh_token());

  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();

  QUrl url(kLiveUserInfo);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchUserInfoFinished(QNetworkReply*)), reply);
}

void SkydriveService::AddAuthorizationHeader(QNetworkRequest* request) {
  request->setRawHeader("Authorization",
                        QString("Bearer %1").arg(access_token_).toUtf8());
}

void SkydriveService::FetchUserInfoFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();

  QString name = response["name"].toString();
  if (!name.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("name", name);
  }

  emit Connected();

  ListFiles("me/skydrive");
}

void SkydriveService::ListFiles(const QString& folder) {
  QUrl url(QString(kSkydriveBase) + folder + "/files");
  url.addQueryItem("filter", "audio,folders");
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(ListFilesFinished(QNetworkReply*)), reply);
}

void SkydriveService::ListFilesFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();

  QVariantList files = response["data"].toList();
  for (const QVariant& f : files) {
    QVariantMap file = f.toMap();
    if (file["type"].toString() == "audio") {
      QString mime_type = GuessMimeTypeForFile(file["name"].toString());
      QUrl url;
      url.setScheme("skydrive");
      url.setPath(file["id"].toString());

      Song song;
      song.set_url(url);
      song.set_ctime(file["created_time"].toDateTime().toTime_t());
      song.set_mtime(file["updated_time"].toDateTime().toTime_t());
      song.set_comment(file["description"].toString());
      song.set_filesize(file["size"].toInt());
      song.set_title(file["name"].toString());

      QUrl download_url = file["source"].toUrl();
      // HTTPS appears to be broken somehow between Qt & Skydrive downloads.
      // Fortunately, just changing the scheme to HTTP works.
      download_url.setScheme("http");
      MaybeAddFileToDatabase(song, mime_type, download_url, QString::null);
    } else if (file["type"].toString() == "folder") {
      ListFiles(file["id"].toString());
    }
  }
}

QUrl SkydriveService::GetStreamingUrlFromSongId(const QString& file_id) {
  EnsureConnected();

  QUrl url(QString(kSkydriveBase) + file_id);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  std::unique_ptr<QNetworkReply> reply(network_->get(request));
  WaitForSignal(reply.get(), SIGNAL(finished()));

  QJson::Parser parser;
  QVariantMap response = parser.parse(reply.get()).toMap();
  return response["source"].toUrl();
}

void SkydriveService::EnsureConnected() {
  if (!access_token_.isEmpty()) {
    return;
  }

  Connect();
  WaitForSignal(this, SIGNAL(Connected()));
}

void SkydriveService::ForgetCredentials() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.remove("refresh_token");
  s.remove("name");
}
