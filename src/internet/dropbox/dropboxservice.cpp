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
#include <QTimer>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

static const char* kMediaEndpoint = "https://api.dropbox.com/1/media/dropbox/";
static const char* kDeltaEndpoint = "https://api.dropbox.com/1/delta";
static const char* kLongPollEndpoint =
    "https://api-notify.dropbox.com/1/longpoll_delta";

}  // namespace

DropboxService::DropboxService(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       IconLoader::Load("dropbox", IconLoader::Provider),
                       SettingsDialog::Page_Dropbox),
      network_(new NetworkAccessManager(this)) {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  // OAuth2 version of dropbox auth token.
  access_token_ = settings.value("access_token2").toString();
  app->player()->RegisterUrlHandler(new DropboxUrlHandler(this, this));
}

bool DropboxService::has_credentials() const {
  return !access_token_.isEmpty();
}

void DropboxService::Connect() {
  if (has_credentials()) {
    RequestFileList();
  } else {
    ShowSettingsDialog();
  }
}

void DropboxService::AuthenticationFinished(OAuthenticator* authenticator) {
  authenticator->deleteLater();

  access_token_ = authenticator->access_token();

  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("access_token2", access_token_);

  emit Connected();

  RequestFileList();
}

QByteArray DropboxService::GenerateAuthorisationHeader() {
  return QString("Bearer %1").arg(access_token_).toUtf8();
}

void DropboxService::RequestFileList() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QUrl url = QUrl(QString(kDeltaEndpoint));
  QUrlQuery url_query;
  if (s.contains("cursor")) {
    url_query.addQueryItem("cursor", s.value("cursor").toString());
  }
  url.setQuery(url_query);
  QNetworkRequest request(url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());

  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestFileListFinished(QNetworkReply*)), reply);
}

void DropboxService::RequestFileListFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJsonDocument document = QJsonDocument::fromBinaryData(reply->readAll());
  QJsonObject json_response = document.object();

  if (json_response.contains("reset") && json_response["reset"].toBool()) {
    qLog(Debug) << "Resetting Dropbox DB";
    library_backend_->DeleteAll();
  }

  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("cursor", json_response["cursor"].toString());

  QJsonArray contents = json_response["entries"].toArray();
  qLog(Debug) << "Delta found:" << contents.size();
  for (const QJsonValue& c : contents) {
    QJsonArray item = c.toArray();
    QString path = item[0].toString();

    QUrl url;
    url.setScheme("dropbox");
    url.setPath(path);

    if (item[1].isNull()) {
      // Null metadata indicates path deleted.
      qLog(Debug) << "Deleting:" << url;
      Song song = library_backend_->GetSongByUrl(url);
      if (song.is_valid()) {
        library_backend_->DeleteSongs(SongList() << song);
      }
      continue;
    }

    QJsonObject metadata = item[1].toObject();
    if (metadata["is_dir"].toBool()) {
      continue;
    }

    // Workaround: Since Dropbox doesn't recognize Opus files and thus treats
    // them
    // as application/octet-stream, we overwrite the mime type here
    if (metadata["mime_type"].toString() == "application/octet-stream" &&
        url.toString().endsWith(".opus"))
      metadata["mime_type"] = GuessMimeTypeForFile(url.toString());

    if (ShouldIndexFile(url, metadata["mime_type"].toString())) {
      QNetworkReply* reply = FetchContentUrl(url);
      NewClosure(reply, SIGNAL(finished()), this,
                 SLOT(FetchContentUrlFinished(QNetworkReply*, QVariantMap)),
                 reply, metadata);
    }
  }

  if (json_response.contains("has_more") && json_response["has_more"].toBool()) {
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
  QUrlQuery url_query;
  if (s.contains("cursor")) {
    url_query.addQueryItem("cursor", s.value("cursor").toString());
  }
  request_url.setQuery(url_query);
  QNetworkRequest request(request_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(LongPollFinished(QNetworkReply*)), reply);
}

void DropboxService::LongPollFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJsonObject json_response = QJsonDocument::fromBinaryData(reply->readAll()).object();
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
  QUrl request_url = QUrl((QString(kMediaEndpoint)));
  request_url.setPath(request_url.path() + url.path().mid(1));
  QNetworkRequest request(request_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  return network_->post(request, QByteArray());
}

void DropboxService::FetchContentUrlFinished(QNetworkReply* reply,
                                             const QVariantMap& data) {
  reply->deleteLater();
  QJsonObject json_response = QJsonDocument::fromBinaryData(reply->readAll()).object();
  QFileInfo info(data["path"].toString());

  QUrl url;
  url.setScheme("dropbox");
  url.setPath(data["path"].toString());

  Song song;
  song.set_url(url);
  song.set_etag(data["rev"].toString());
  song.set_mtime(ParseRFC822DateTime(data["modified"].toString()).toTime_t());
  song.set_title(info.fileName());
  song.set_filesize(data["bytes"].toInt());
  song.set_ctime(0);

  MaybeAddFileToDatabase(song, data["mime_type"].toString(),
                         QUrl::fromEncoded(json_response["url"].toString().toLatin1()),
                         QString::null);
}

QUrl DropboxService::GetStreamingUrlFromSongId(const QUrl& url) {
  QNetworkReply* reply = FetchContentUrl(url);
  WaitForSignal(reply, SIGNAL(finished()));

  QJsonObject json_response = QJsonDocument::fromJson(reply->readAll()).object();
  return QUrl::fromEncoded(json_response["url"].toString().toLatin1());
}
