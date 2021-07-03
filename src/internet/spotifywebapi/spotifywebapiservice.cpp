/* This file is part of Clementine.
   Copyright 2021, Kenman Tsang <kentsangkm@pm.me>

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

#include "spotifywebapiservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtDebug>
#include <utility>

#include "3rdparty/qtiocompressor/qtiocompressor.h"
#include "core/application.h"
#include "core/network.h"
#include "core/timeconstants.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/spotifywebapisearchprovider.h"
#include "ui/iconloader.h"

namespace {
static constexpr const char* kServiceName = "SpotifyWebApi";
static constexpr const char* kGetAccessTokenUrl =
    "https://open.spotify.com/"
    "get_access_token?reason=transport&productType=web_player";
static constexpr const char* kSearchUrl =
    "https://api.spotify.com/v1/search?q=%1&type=track&limit=50";

template <typename... Args>
inline QJsonValue Get(QJsonValue obj, Args&&... args) {
  std::array<const char*, sizeof...(Args)> names = {
      std::forward<Args>(args)...};
  for (auto&& name : names) {
    Q_ASSERT(obj.isObject());
    obj = obj.toObject()[name];
  }
  return obj;
}

template <typename... Args>
inline QJsonValue Get(const QJsonDocument& obj, Args&&... args) {
  return Get(obj.object(), std::forward<Args>(args)...);
}

QString concat(const QJsonArray& array, const char* name) {
  QStringList ret;
  for (auto&& item : array) {
    ret << Get(item, name).toString();
  }
  return ret.join(", ");
}

}  // namespace

SpotifyWebApiService::SpotifyWebApiService(Application* app,
                                           InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      network_(new NetworkAccessManager{this}),
      token_expiration_ms_{0} {
  app_->global_search()->AddProvider(
      new SpotifyWebApiSearchProvider(app_, this));
}

SpotifyWebApiService::~SpotifyWebApiService() {}

QStandardItem* SpotifyWebApiService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("spotify", IconLoader::Provider),
                            kServiceName);
  return root_;
}

void SpotifyWebApiService::LazyPopulate(QStandardItem* item) {}

void SpotifyWebApiService::Search(int searchId, QString queryStr) {
  if (QDateTime::currentDateTime().toMSecsSinceEpoch() >=
      token_expiration_ms_) {
    QNetworkRequest request{QUrl{kGetAccessTokenUrl}};
    request.setRawHeader("Accept-Encoding", "gzip");

    QNetworkReply* reply = network_->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
      reply->deleteLater();

      OnTokenReady(ParseJsonReplyWithGzip(reply), searchId, queryStr);
    });
  } else {
    OnReadyToSearch(searchId, queryStr);
  }
}

void SpotifyWebApiService::OnTokenReady(const QJsonDocument& json, int searchId,
                                        QString queryStr) {
  if (!json.isEmpty()) {
    token_ = Get(json, "accessToken").toString();
    token_expiration_ms_ = static_cast<qint64>(
        Get(json, "accessTokenExpirationTimestampMs").toDouble());

    qLog(Debug) << "Spotify API Token:" << token_;

    OnReadyToSearch(searchId, queryStr);
  }
}

void SpotifyWebApiService::OnReadyToSearch(int searchId, QString queryStr) {
  qLog(Debug) << "Spotify API Searching: " << queryStr;

  QNetworkRequest request{
      QUrl{QString(kSearchUrl).arg(queryStr.toHtmlEscaped())}};

  request.setRawHeader("Accept-Encoding", "gzip");
  request.setRawHeader("Authorization", ("Bearer " + token_).toUtf8());

  QNetworkReply* reply = network_->get(request);
  connect(reply, &QNetworkReply::finished, [=] {
    reply->deleteLater();

    BuildResultList(ParseJsonReplyWithGzip(reply), searchId);
  });
}

void SpotifyWebApiService::BuildResultList(const QJsonDocument& json,
                                           int searchId) {
  QList<Song> result;

  for (auto&& item : Get(json, "tracks", "items").toArray()) {
    Song song;

    song.set_albumartist(
        concat(Get(item, "album", "artists").toArray(), "name"));
    song.set_album(Get(item, "album", "name").toString());
    song.set_artist(concat(Get(item, "artists").toArray(), "name"));
    song.set_disc(Get(item, "disc_number").toInt());
    song.set_length_nanosec(Get(item, "duration_ms").toInt() * kNsecPerMsec);
    song.set_title(Get(item, "name").toString());
    song.set_track(Get(item, "track_number").toInt());
    song.set_url(QUrl{Get(item, "uri").toString()});
    song.set_filetype(Song::Type_Stream);
    song.set_valid(true);
    song.set_directory_id(0);
    song.set_mtime(0);
    song.set_ctime(0);
    song.set_filesize(0);

    result += song;
  }

  emit SearchFinished(searchId, result);
}

QJsonDocument SpotifyWebApiService::ParseJsonReplyWithGzip(
    QNetworkReply* reply) {
  if (reply->error() != QNetworkReply::NoError) {
    app_->AddError(tr("%1 request failed:\n%2")
                       .arg(kServiceName)
                       .arg(reply->errorString()));
    return QJsonDocument();
  }

  QByteArray output;
  if (reply->hasRawHeader("content-encoding") &&
      reply->rawHeader("content-encoding") == "gzip") {
    QtIOCompressor gzip(reply);
    gzip.setStreamFormat(QtIOCompressor::GzipFormat);
    if (!gzip.open(QIODevice::ReadOnly)) {
      app_->AddError(tr("%1 failed to decode as gzip stream:\n%2")
                         .arg(kServiceName)
                         .arg(gzip.errorString()));
      return QJsonDocument();
    }
    output = gzip.readAll();
  } else {
    output = reply->readAll();
  }

  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(output, &error);
  if (error.error != QJsonParseError::NoError) {
    app_->AddError(tr("Failed to parse %1 response:\n%2")
                       .arg(kServiceName)
                       .arg(error.errorString()));
    return QJsonDocument();
  }

  return document;
}
