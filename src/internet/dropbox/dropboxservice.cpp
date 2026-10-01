/* This file is part of Clementine.
   Copyright 2012-2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Martin Brodbeck <martin@brodbeck-online.de>
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

#include "dropboxservice.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrlQuery>

#include "core/application.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/utilities.h"
#include "core/waitforsignal.h"
#include "internet/core/oauthenticator.h"
#include "internet/dropbox/dropboxurlhandler.h"
#include "library/librarybackend.h"
#include "ui/iconloader.h"

using Utilities::ParseRFC822DateTime;

const char* DropboxService::kServiceName = "Dropbox";
const char* DropboxService::kSettingsGroup = "Dropbox";

namespace {

static const char* kServiceId = "dropbox";

static const char* kOAuthEndpoint = "https://www.dropbox.com/oauth2/authorize";
static const char* kOAuthClientId = "qh6ca27eclt9p2k";
static const char* kOAuthClientSecret = "pg7y68h5efap8r6";
static const char* kOAuthTokenEndpoint =
    "https://api.dropboxapi.com/oauth2/token";

static const char* kMediaEndpoint =
    "https://api.dropboxapi.com/2/files/get_temporary_link";
static const char* kListFolderEndpoint =
    "https://api.dropboxapi.com/2/files/list_folder";
static const char* kListFolderContinueEndpoint =
    "https://api.dropboxapi.com/2/files/list_folder/continue";
static const char* kLongPollEndpoint =
    "https://notify.dropboxapi.com/2/files/list_folder/longpoll";

}  // namespace

DropboxService::DropboxService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       IconLoader::Load("dropbox", IconLoader::Provider),
                       SettingsDialog::Page_Dropbox),
      network_(new NetworkAccessManager(this)) {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  // A long-lived token from before Dropbox's tokens started expiring, kept
  // until it's replaced by logging in again. It has no expiry time.
  access_token_ = settings.value("access_token2").toString();
  app->player()->RegisterUrlHandler(new DropboxUrlHandler(this, this));
}

bool DropboxService::has_credentials() const {
  return !refresh_token().isEmpty() || !access_token_.isEmpty();
}

QString DropboxService::refresh_token() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  return s.value("refresh_token").toString();
}

bool DropboxService::is_authenticated() const {
  return !access_token_.isEmpty() &&
         (!expiry_time_.isValid() ||
          QDateTime::currentDateTime().secsTo(expiry_time_) > 0);
}

void DropboxService::Connect() {
  if (!has_credentials()) {
    ShowConfig();
  } else if (is_authenticated()) {
    RequestFileList();
  } else {
    connect(RefreshAccessToken(), &OAuthenticator::Finished, this, [this] {
      if (is_authenticated()) RequestFileList();
    });
  }
}

void DropboxService::Login() {
  OAuthenticator* oauth = new OAuthenticator(
      kOAuthClientId, kOAuthClientSecret,
      OAuthenticator::RedirectStyle::REMOTE_WITH_STATE, this);
  connect(oauth, &OAuthenticator::Finished, this, [this, oauth] {
    AccessTokenFinished(oauth);
    if (is_authenticated()) RequestFileList();
  });
  // Dropbox's access tokens expire after a few hours: ask for a refresh
  // token too.
  QUrlQuery extra_params;
  extra_params.addQueryItem("token_access_type", "offline");
  oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint, QString(),
                            extra_params);
}

OAuthenticator* DropboxService::RefreshAccessToken() {
  OAuthenticator* oauth = new OAuthenticator(
      kOAuthClientId, kOAuthClientSecret,
      OAuthenticator::RedirectStyle::REMOTE_WITH_STATE, this);
  connect(oauth, &OAuthenticator::Finished, this,
          [this, oauth] { AccessTokenFinished(oauth); });
  oauth->RefreshAuthorisation(kOAuthTokenEndpoint, refresh_token());
  return oauth;
}

void DropboxService::AccessTokenFinished(OAuthenticator* oauth) {
  oauth->deleteLater();

  if (oauth->access_token().isEmpty()) {
    qLog(Error) << "Couldn't get a Dropbox access token";
    return;
  }

  access_token_ = oauth->access_token();
  expiry_time_ = oauth->expiry_time();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  // A refresh doesn't come with a new refresh token: keep the one we have.
  if (!oauth->refresh_token().isEmpty()) {
    s.setValue("refresh_token", oauth->refresh_token());
  }
  s.remove("access_token2");

  emit Connected();
}

void DropboxService::EnsureConnected() {
  if (is_authenticated() || refresh_token().isEmpty()) return;

  WaitForSignal(RefreshAccessToken(), SIGNAL(Finished()));
}

void DropboxService::ForgetCredentials() {
  access_token_.clear();
  expiry_time_ = QDateTime();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("refresh_token");
  s.remove("access_token2");
}

QByteArray DropboxService::GenerateAuthorisationHeader() {
  return QString("Bearer %1").arg(access_token_).toUtf8();
}

void DropboxService::RequestFileList() {
  if (!is_authenticated()) {
    // The access token has expired since the last listing: this is called
    // again once there's a new one.
    Connect();
    return;
  }

  QSettings s;
  s.beginGroup(kSettingsGroup);

  QString cursor = s.value("cursor", "").toString();

  if (cursor.isEmpty()) {
    QUrl url = QUrl(QString(kListFolderEndpoint));

    QJsonObject json;
    json.insert("path", "");
    json.insert("recursive", true);
    json.insert("include_deleted", true);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", GenerateAuthorisationHeader());
    request.setRawHeader("Content-Type", "application/json; charset=utf-8");

    QJsonDocument document(json);
    QNetworkReply* reply = network_->post(request, document.toJson());
    connect(reply, &QNetworkReply::finished,
            [=] { this->RequestFileListFinished(reply); });
  } else {
    QUrl url = QUrl(kListFolderContinueEndpoint);
    QJsonObject json;
    json.insert("cursor", cursor);
    QJsonDocument document(json);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", GenerateAuthorisationHeader());
    request.setRawHeader("Content-Type", "application/json; charset=utf-8");
    QNetworkReply* reply = network_->post(request, document.toJson());
    connect(reply, &QNetworkReply::finished,
            [=] { this->RequestFileListFinished(reply); });
  }
}

void DropboxService::RequestFileListFinished(QNetworkReply* reply) {
  reply->deleteLater();

  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() ==
          401 &&
      refresh_token().isEmpty()) {
    // An access token saved before we asked for refresh tokens, which has
    // expired, as all of them since 2021 have.
    ForgetCredentials();
    app_->AddError(
        tr("Your Dropbox login has expired. Log in again in "
           "Dropbox's settings."));
    return;
  }

  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) return;

  QJsonObject json_response = document.object();

  if (json_response.contains("reset") && json_response["reset"].toBool()) {
    qLog(Debug) << "Resetting Dropbox DB";
    library_backend_->DeleteAll();
  }

  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("cursor", json_response["cursor"].toString());

  QJsonArray contents = json_response["entries"].toArray();
  qLog(Debug) << "File list found:" << contents.size();
  for (const QJsonValue& c : contents) {
    QJsonObject item = c.toObject();
    QString path = item["path_lower"].toString();

    QUrl url;
    url.setScheme("dropbox");
    url.setPath(path);

    if (item[".tag"].toString() == "deleted") {
      qLog(Debug) << "Deleting:" << url;
      Song song = library_backend_->GetSongByUrl(url);
      if (song.is_valid()) {
        library_backend_->DeleteSongs(SongList() << song);
      }
      continue;
    }

    if (item[".tag"].toString() == "folder") {
      continue;
    }

    if (ShouldIndexFile(url, GuessMimeTypeForFile(url.toString()))) {
      QNetworkReply* reply = FetchContentUrl(url);
      connect(reply, &QNetworkReply::finished, [=] {
        this->FetchContentUrlFinished(reply, item.toVariantMap());
      });
    }
  }

  if (json_response.contains("has_more") &&
      json_response["has_more"].toBool()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("cursor", json_response["cursor"].toVariant());
    RequestFileList();
  } else {
    // Long-poll wait for changes.
    LongPollDelta();
  }
}

void DropboxService::LongPollDelta() {
  if (!has_credentials()) {
    // Might have been signed out by the user.
    return;
  }
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QUrl request_url = QUrl(QString(kLongPollEndpoint));
  QJsonObject json;
  json.insert("cursor", s.value("cursor").toString());
  json.insert("timeout", 30);
  QNetworkRequest request(request_url);
  request.setRawHeader("Content-Type", "application/json; charset=utf-8");
  QJsonDocument document(json);
  QNetworkReply* reply = network_->post(request, document.toJson());
  connect(reply, &QNetworkReply::finished,
          [=] { this->LongPollFinished(reply); });
}

void DropboxService::LongPollFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) return;

  QJsonObject json_response = document.object();
  if (json_response["changes"].toBool()) {
    // New changes, we should request deltas again.
    qLog(Debug) << "Detected new dropbox changes; fetching...";
    RequestFileList();
  } else {
    bool ok = false;
    int backoff_secs = json_response["backoff"].toString().toInt(&ok);
    backoff_secs = ok ? backoff_secs : 0;

    QTimer::singleShot(backoff_secs * 1000, this, SLOT(LongPollDelta()));
  }
}

QNetworkReply* DropboxService::FetchContentUrl(const QUrl& url) {
  QUrl request_url(kMediaEndpoint);
  QJsonObject json;
  json.insert("path", url.path());
  QJsonDocument document(json);
  QNetworkRequest request(request_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  request.setRawHeader("Content-Type", "application/json; charset=utf-8");
  return network_->post(request, document.toJson());
}

void DropboxService::FetchContentUrlFinished(QNetworkReply* reply,
                                             const QVariantMap& data) {
  reply->deleteLater();

  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) return;

  QJsonObject json_response = document.object();
  QFileInfo info(data["path_lower"].toString());

  QUrl url;
  url.setScheme("dropbox");
  url.setPath(data["path_lower"].toString());

  Song song;
  song.set_url(url);
  song.set_etag(data["rev"].toString());
  song.set_mtime(
      QDateTime::fromString(data["server_modified"].toString(), Qt::ISODate)
          .toSecsSinceEpoch());
  song.set_title(info.fileName());
  song.set_filesize(data["size"].toInt());
  song.set_ctime(0);

  MaybeAddFileToDatabase(
      song, GuessMimeTypeForFile(url.toString()),
      QUrl::fromEncoded(json_response["link"].toVariant().toByteArray()),
      QString());
}

QUrl DropboxService::GetStreamingUrlFromSongId(const QUrl& url) {
  EnsureConnected();
  QNetworkReply* reply = FetchContentUrl(url);
  WaitForSignal(reply, SIGNAL(finished()));

  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) return QUrl();

  QJsonObject json_response = document.object();
  return QUrl::fromEncoded(json_response["link"].toVariant().toByteArray());
}
