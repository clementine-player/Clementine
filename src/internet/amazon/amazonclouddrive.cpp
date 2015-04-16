/* This file is part of Clementine.
   Copyright 2015, John Maguire <john.maguire@gmail.com>

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

#include "internet/amazon/amazonclouddrive.h"

#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/waitforsignal.h"
#include "internet/core/oauthenticator.h"
#include "internet/amazon/amazonurlhandler.h"
#include "library/librarybackend.h"
#include "ui/settingsdialog.h"

const char* AmazonCloudDrive::kServiceName = "Cloud Drive";
const char* AmazonCloudDrive::kSettingsGroup = "AmazonCloudDrive";

namespace {
static const char* kServiceId = "amazon_cloud_drive";
static const char* kClientId =
    "amzn1.application-oa2-client.2b1157a7dadc45c3888567882b3a9f05";
static const char* kClientSecret =
    "acfbf95340cc4c381dd43fb75b5e111882d7fd1b02a02f3013ab124baf8d1655";
static const char* kOAuthScope = "clouddrive:read";
static const char* kOAuthEndpoint = "https://www.amazon.com/ap/oa";
static const char* kOAuthTokenEndpoint = "https://api.amazon.com/auth/o2/token";

static const char* kEndpointEndpoint =
    "https://drive.amazonaws.com/drive/v1/account/endpoint";
static const char* kChangesEndpoint = "%1/changes";
static const char* kDownloadEndpoint = "%1/nodes/%2/content";
}  // namespace

AmazonCloudDrive::AmazonCloudDrive(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       QIcon(":/providers/amazonclouddrive.png"),
                       SettingsDialog::Page_AmazonCloudDrive),
      network_(new NetworkAccessManager) {
  app->player()->RegisterUrlHandler(new AmazonUrlHandler(this, this));
}

bool AmazonCloudDrive::has_credentials() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  return !s.value("refresh_token").toString().isEmpty();
}

QUrl AmazonCloudDrive::GetStreamingUrlFromSongId(const QUrl& url) {
  EnsureConnected();  // Access token must be up to date.
  QUrl download_url(
      QString(kDownloadEndpoint).arg(content_url_).arg(url.path()));
  download_url.setFragment(QString("Bearer %1").arg(access_token_));
  return download_url;
}

void AmazonCloudDrive::Connect() {
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret,
      // Amazon forbids arbitrary query parameters so REMOTE_WITH_STATE is
      // required.
      OAuthenticator::RedirectStyle::REMOTE_WITH_STATE, this);

  QSettings s;
  s.beginGroup(kSettingsGroup);
  QString refresh_token = s.value("refresh_token").toString();
  if (refresh_token.isEmpty()) {
    oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint, kOAuthScope);
  } else {
    oauth->RefreshAuthorisation(kOAuthTokenEndpoint, refresh_token);
  }

  NewClosure(oauth, SIGNAL(Finished()), this,
            SLOT(ConnectFinished(OAuthenticator*)), oauth);
}

void AmazonCloudDrive::EnsureConnected() {
  if (access_token_.isEmpty() ||
      QDateTime::currentDateTime().secsTo(expiry_time_) < 60) {
    Connect();
    WaitForSignal(this, SIGNAL(Connected()));
  }
}

void AmazonCloudDrive::ForgetCredentials() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("");
  access_token_ = QString();
  expiry_time_ = QDateTime();
}

void AmazonCloudDrive::ConnectFinished(OAuthenticator* oauth) {
  oauth->deleteLater();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", oauth->refresh_token());

  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();

  FetchEndpoint();
}

void AmazonCloudDrive::FetchEndpoint() {
  QUrl url(kEndpointEndpoint);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchEndpointFinished(QNetworkReply*)), reply);
}

void AmazonCloudDrive::FetchEndpointFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJsonObject json_response = QJsonDocument::fromJson(reply->readAll()).object();
  content_url_ = json_response["contentUrl"].toString();
  metadata_url_ = json_response["metadataUrl"].toString();
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QString checkpoint = s.value("checkpoint", "").toString();
  RequestChanges(checkpoint);

  // We wait until we know the endpoint URLs before emitting Connected();
  emit Connected();
}

void AmazonCloudDrive::RequestChanges(const QString& checkpoint) {
  EnsureConnected();
  QUrl url(QString(kChangesEndpoint).arg(metadata_url_));

  QJsonDocument data;
  QJsonObject object;
  object.insert("includePurged", QJsonValue("true"));
  if (!checkpoint.isEmpty()) {
    object.insert("checkpoint", checkpoint);
  }
  data.setObject(object);
  QByteArray json = data.toBinaryData();

  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->post(request, json);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestChangesFinished(QNetworkReply*)), reply);
}

void AmazonCloudDrive::RequestChangesFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QByteArray data = reply->readAll();
  QJsonObject json_response = QJsonDocument::fromBinaryData(data).object();

  QString checkpoint = json_response["checkpoint"].toString();
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("checkpoint", checkpoint);

  QJsonArray nodes = json_response["nodes"].toArray();
  for (const QJsonValue& n : nodes) {
    QJsonObject node = n.toObject();
    if (node["kind"].toString() == "FOLDER") {
      // Skip directories.
      continue;
    }
    QUrl url;
    url.setScheme("amazonclouddrive");
    url.setPath("/" + node["id"].toString());

    QString status = node["status"].toString();
    if (status == "PURGED") {
      // Remove no longer available files.
      Song song = library_backend_->GetSongByUrl(url);
      if (song.is_valid()) {
        library_backend_->DeleteSongs(SongList() << song);
      }
      continue;
    }
    if (status != "AVAILABLE") {
      // Ignore any other statuses.
      continue;
    }

    QJsonObject content_properties = node["contentProperties"].toObject();
    QString mime_type = content_properties["contentType"].toString();

    if (ShouldIndexFile(url, mime_type)) {
      QString node_id = node["id"].toString();
      QUrl content_url(
          QString(kDownloadEndpoint).arg(content_url_).arg(node_id));
      QString md5 = content_properties["md5"].toString();

      Song song;
      song.set_url(url);
      song.set_etag(md5);
      song.set_mtime(QDateTime::fromString(node["modifiedDate"].toString()).toTime_t());
      song.set_ctime(QDateTime::fromString(node["createdDate"].toString()).toTime_t());
      song.set_title(node["name"].toString());
      song.set_filesize(content_properties["size"].toInt());

      MaybeAddFileToDatabase(song, mime_type, content_url, QString("Bearer %1").arg(access_token_));
    }
  }

  // The API potentially returns a second JSON dictionary appended with a
  // newline at the end of the response with {"end": true} indicating that our
  // client is up to date with the latest changes.
  const int last_newline_index = data.lastIndexOf('\n');
  QByteArray last_line = data.mid(last_newline_index);
  QJsonObject end_json = QJsonDocument::fromJson(last_line).object();
  if (end_json.contains("end") && end_json["end"].toBool()) {
    return;
  } else {
    RequestChanges(checkpoint);
  }
}

void AmazonCloudDrive::AddAuthorizationHeader(QNetworkRequest* request) {
  request->setRawHeader("Authorization",
                        QString("Bearer %1").arg(access_token_).toUtf8());
}
