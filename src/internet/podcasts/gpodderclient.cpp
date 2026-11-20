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

#include "gpodderclient.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

const char* GPodderClient::kBaseUrl = "https://gpodder.net";

namespace {

// For a name that goes into a URL's path.
QString PathPart(const QString& name) {
  return QString::fromLatin1(QUrl::toPercentEncoding(name));
}

QJsonArray UrlArray(const QList<QUrl>& urls) {
  QJsonArray ret;
  for (const QUrl& url : urls) {
    ret.append(url.toString());
  }
  return ret;
}

GPodderClient::EpisodeAction::Status ParseStatus(const QString& status) {
  using Status = GPodderClient::EpisodeAction::Status;
  const QString lower = status.toLower();
  if (lower == "new") return Status::kNew;
  if (lower == "play") return Status::kPlayed;
  if (lower == "download") return Status::kDownloaded;
  if (lower == "delete") return Status::kDeleted;
  return Status::kUnknown;
}

}  // namespace

GPodderClient::GPodderClient(QNetworkAccessManager* network, QObject* parent)
    : QObject(parent), network_(network) {}

void GPodderClient::SetCredentials(const QString& username,
                                   const QString& password) {
  username_ = username;
  password_ = password;
}

QUrl GPodderClient::ApiUrl(const QString& path) const {
  return QUrl(QString(kBaseUrl) + path);
}

QNetworkRequest GPodderClient::Request(const QUrl& url) const {
  QNetworkRequest request(url);
  if (!username_.isEmpty()) {
    request.setRawHeader(
        "Authorization",
        "Basic " + (username_ + ":" + password_).toUtf8().toBase64());
  }
  return request;
}

void GPodderClient::Get(const QUrl& url, JsonCallback done,
                        ErrorCallback failed) {
  HandleReply(network_->get(Request(url)), done, failed);
}

void GPodderClient::Post(const QUrl& url, const QJsonDocument& body,
                         JsonCallback done, ErrorCallback failed) {
  QNetworkRequest request = Request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  HandleReply(network_->post(request, body.toJson(QJsonDocument::Compact)),
              done, failed);
}

void GPodderClient::HandleReply(QNetworkReply* reply, JsonCallback done,
                                ErrorCallback failed) {
  connect(reply, &QNetworkReply::finished, this, [reply, done, failed]() {
    reply->deleteLater();

    const int status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError) {
      failed(reply->errorString());
      return;
    }
    if (status >= 400) {
      failed(tr("gpodder.net replied with HTTP status %1").arg(status));
      return;
    }

    const QByteArray data = reply->readAll();
    if (data.trimmed().isEmpty()) {
      done(QJsonDocument());
      return;
    }

    QJsonParseError error;
    const QJsonDocument json = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
      failed(
          tr("Couldn't read gpodder.net's reply: %1").arg(error.errorString()));
      return;
    }
    done(json);
  });
}

Podcast GPodderClient::ParsePodcast(const QJsonObject& object) {
  Podcast podcast;
  podcast.set_url(QUrl(object["url"].toString()));
  podcast.set_title(object["title"].toString());
  podcast.set_description(object["description"].toString());
  podcast.set_link(QUrl(object["website"].toString()));
  podcast.set_image_url_large(QUrl(object["logo_url"].toString()));

  podcast.set_extra("gpodder:subscribers", object["subscribers"].toInt());
  podcast.set_extra("gpodder:subscribers_last_week",
                    object["subscribers_last_week"].toInt());
  podcast.set_extra("gpodder:page", QUrl(object["mygpo_link"].toString()));
  return podcast;
}

QList<Podcast> GPodderClient::ParsePodcasts(const QJsonDocument& json) {
  QList<Podcast> ret;
  for (const QJsonValue& value : json.array()) {
    ret << ParsePodcast(value.toObject());
  }
  return ret;
}

void GPodderClient::Search(const QString& query, PodcastsCallback done,
                           ErrorCallback failed) {
  QUrl url = ApiUrl("/search.json");
  QUrlQuery url_query;
  url_query.addQueryItem("q", query);
  url.setQuery(url_query);

  Get(
      url, [done](const QJsonDocument& json) { done(ParsePodcasts(json)); },
      failed);
}

void GPodderClient::TopTags(int count, TagsCallback done,
                            ErrorCallback failed) {
  Get(
      ApiUrl(QString("/api/2/tags/%1.json").arg(count)),
      [done](const QJsonDocument& json) {
        QStringList tags;
        for (const QJsonValue& value : json.array()) {
          const QString tag = value.toObject()["tag"].toString();
          if (!tag.isEmpty()) tags << tag;
        }
        done(tags);
      },
      failed);
}

void GPodderClient::PodcastsOfTag(const QString& tag, int count,
                                  PodcastsCallback done, ErrorCallback failed) {
  Get(
      ApiUrl(QString("/api/2/tag/%1/%2.json")
                 .arg(PathPart(tag), QString::number(count))),
      [done](const QJsonDocument& json) { done(ParsePodcasts(json)); }, failed);
}

void GPodderClient::UpdateDevice(const QString& device_id,
                                 const QString& caption, DeviceType type,
                                 DoneCallback done, ErrorCallback failed) {
  QJsonObject body;
  body["caption"] = caption;
  body["type"] = type == DeviceType::kLaptop ? "laptop" : "desktop";

  Post(
      ApiUrl(QString("/api/2/devices/%1/%2.json")
                 .arg(PathPart(username_), PathPart(device_id))),
      QJsonDocument(body), [done](const QJsonDocument&) { done(); }, failed);
}

void GPodderClient::GetDeviceUpdates(const QString& device_id, qint64 since,
                                     DeviceUpdatesCallback done,
                                     ErrorCallback failed) {
  QUrl url = ApiUrl(QString("/api/2/updates/%1/%2.json")
                        .arg(PathPart(username_), PathPart(device_id)));
  QUrlQuery url_query;
  url_query.addQueryItem("since", QString::number(since));
  url.setQuery(url_query);

  Get(
      url,
      [done](const QJsonDocument& json) {
        const QJsonObject object = json.object();
        DeviceUpdates updates;
        for (const QJsonValue& value : object["add"].toArray()) {
          updates.add << ParsePodcast(value.toObject());
        }
        for (const QJsonValue& value : object["remove"].toArray()) {
          updates.remove << QUrl(value.toString());
        }
        for (const QJsonValue& value : object["updates"].toArray()) {
          const QJsonObject episode = value.toObject();
          EpisodeAction action;
          action.podcast_url = QUrl(episode["podcast_url"].toString());
          action.url = QUrl(episode["url"].toString());
          action.status = ParseStatus(episode["status"].toString());
          updates.updates << action;
        }
        done(updates);
      },
      failed);
}

void GPodderClient::AddRemoveSubscriptions(const QString& device_id,
                                           const QList<QUrl>& add,
                                           const QList<QUrl>& remove,
                                           DoneCallback done,
                                           ErrorCallback failed) {
  QJsonObject body;
  body["add"] = UrlArray(add);
  body["remove"] = UrlArray(remove);

  Post(
      ApiUrl(QString("/api/2/subscriptions/%1/%2.json")
                 .arg(PathPart(username_), PathPart(device_id))),
      QJsonDocument(body), [done](const QJsonDocument&) { done(); }, failed);
}
