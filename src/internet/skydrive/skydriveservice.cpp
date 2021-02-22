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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <memory>

#include "core/application.h"
#include "core/player.h"
#include "core/waitforsignal.h"
#include "internet/core/oauthenticator.h"
#include "internet/skydrive/skydriveurlhandler.h"
#include "ui/iconloader.h"

namespace {

// Keep legacy name since it needs to match the database table name.
static const char* kServiceId = "skydrive";

static const char* kClientId = "905def38-34d2-4e32-8ba7-c37bcc329047";
static const char* kClientSecret = "";

// https://docs.microsoft.com/en-us/azure/active-directory/develop/v2-oauth2-auth-code-flow
static const char* kOAuthEndpoint =
    "https://login.microsoftonline.com/common/oauth2/v2.0/authorize";
static const char* kOAuthTokenEndpoint =
    "https://login.microsoftonline.com/common/oauth2/v2.0/token";
static const char* kOAuthScope =
    "User.Read Files.Read Files.Read.All offline_access";

// MS Graph API
// https://docs.microsoft.com/en-us/graph/api/overview?view=graph-rest-1.0
static const char* kGraphUserInfo = "https://graph.microsoft.com/v1.0/me";
static const char* kDriveBase = "https://graph.microsoft.com/v1.0/me/drive/";

}  // namespace

const char* SkydriveService::kServiceName = "OneDrive";
const char* SkydriveService::kSettingsGroup = "OneDrive";

SkydriveService::SkydriveService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       IconLoader::Load("skydrive", IconLoader::Provider),
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
      kClientId, kClientSecret, OAuthenticator::RedirectStyle::LOCALHOST, this);
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

  FetchUserInfo();
}

void SkydriveService::FetchUserInfo() {
  QUrl url(kGraphUserInfo);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchUserInfoFinished(QNetworkReply*)), reply);
}

QByteArray SkydriveService::GetAuthHeader() const {
  return QString("Bearer %1").arg(access_token_).toUtf8();
}

void SkydriveService::AddAuthorizationHeader(QNetworkRequest* request) {
  request->setRawHeader("Authorization", GetAuthHeader());
}

void SkydriveService::FetchUserInfoFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) return;

  QJsonObject json_response = document.object();

  QString name = json_response["displayName"].toString();
  if (!name.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("name", name);
  }

  emit Connected();

  ListFiles("root");
}

void SkydriveService::ListFiles(const QString& folder) {
  QUrl url(QString(kDriveBase) + folder + "/children");
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);

  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(ListFilesFinished(QNetworkReply*)), reply);
}

void SkydriveService::ListFilesFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) return;

  QJsonObject json_response = document.object();

  QJsonArray items = json_response["value"].toArray();
  for (const QJsonValue& f : items) {
    QJsonObject item = f.toObject();

    const QString id = item["id"].toString();
    const QString name = item["name"].toString();

    if (item.contains("folder")) {
      ListFiles(QString("items/%1").arg(id));
    } else if (item.contains("file")) {
      // The response provides a mime type, but it doesn't know about some
      // types that we care about.
      QString mime_type = GuessMimeTypeForFile(name);
      QUrl url;
      url.setScheme(GetScheme());
      url.setPath("/" + id);

      Song song;
      song.set_url(url);
      song.set_ctime(
          QDateTime::fromString(item["createdDateTime"].toString(), Qt::ISODate)
              .toTime_t());
      song.set_mtime(QDateTime::fromString(
                         item["lastModifiedDateTime"].toString(), Qt::ISODate)
                         .toTime_t());
      song.set_comment(item["description"].toString());
      song.set_filesize(item["size"].toInt());
      song.set_title(name);

      QUrl download_url = ItemUrl(id, "content");
      MaybeAddFileToDatabase(song, mime_type, download_url,
                             QString("Bearer %1").arg(access_token_));
    } else {
      qLog(Debug) << "Unknown item type for" << name;
    }
  }
}

QUrl SkydriveService::ItemUrl(const QString& id, const QString& path) {
  return QUrl(QString(kDriveBase) + "items/" + id + "/" + path);
}

QUrl SkydriveService::GetStreamingUrlFromSongId(const QString& file_id) {
  EnsureConnected();

  return ItemUrl(file_id, "content");
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
