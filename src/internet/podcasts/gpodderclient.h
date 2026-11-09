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

#ifndef INTERNET_PODCASTS_GPODDERCLIENT_H_
#define INTERNET_PODCASTS_GPODDERCLIENT_H_

#include <QJsonDocument>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <functional>

#include "podcast.h"

class QJsonObject;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

// The parts of the gpodder.net API that Clementine uses: the directory
// (search and tags) and syncing subscriptions with a device.
// https://gpoddernet.readthedocs.io/en/latest/api/
//
// Each call takes a callback for its result and one for an error, and calls
// exactly one of them, unless the client is deleted first.
class GPodderClient : public QObject {
  Q_OBJECT

 public:
  static const char* kBaseUrl;

  enum class DeviceType { kDesktop, kLaptop };

  // What another device did with an episode.
  struct EpisodeAction {
    enum class Status { kNew, kPlayed, kDownloaded, kDeleted, kUnknown };

    QUrl podcast_url;
    QUrl url;
    Status status = Status::kUnknown;
  };

  // Changes made on other devices since the time given to GetDeviceUpdates.
  struct DeviceUpdates {
    QList<Podcast> add;
    QList<QUrl> remove;
    QList<EpisodeAction> updates;
  };

  using ErrorCallback = std::function<void(const QString& error)>;
  using DoneCallback = std::function<void()>;
  using PodcastsCallback = std::function<void(const QList<Podcast>& podcasts)>;
  using TagsCallback = std::function<void(const QStringList& tags)>;
  using DeviceUpdatesCallback =
      std::function<void(const DeviceUpdates& updates)>;

  explicit GPodderClient(QNetworkAccessManager* network,
                         QObject* parent = nullptr);

  // The account that the device calls are made for. The directory calls
  // don't need one.
  void SetCredentials(const QString& username, const QString& password);

  void Search(const QString& query, PodcastsCallback done,
              ErrorCallback failed);
  // The most used tags, most used first.
  void TopTags(int count, TagsCallback done, ErrorCallback failed);
  void PodcastsOfTag(const QString& tag, int count, PodcastsCallback done,
                     ErrorCallback failed);

  // Creates the device if it doesn't exist. This is also how Clementine
  // checks the username and password.
  void UpdateDevice(const QString& device_id, const QString& caption,
                    DeviceType type, DoneCallback done, ErrorCallback failed);
  // since is in seconds since the epoch; 0 gets everything.
  void GetDeviceUpdates(const QString& device_id, qint64 since,
                        DeviceUpdatesCallback done, ErrorCallback failed);
  void AddRemoveSubscriptions(const QString& device_id, const QList<QUrl>& add,
                              const QList<QUrl>& remove, DoneCallback done,
                              ErrorCallback failed);

  static Podcast ParsePodcast(const QJsonObject& object);

 private:
  using JsonCallback = std::function<void(const QJsonDocument& json)>;

  QUrl ApiUrl(const QString& path) const;
  QNetworkRequest Request(const QUrl& url) const;

  void Get(const QUrl& url, JsonCallback done, ErrorCallback failed);
  void Post(const QUrl& url, const QJsonDocument& body, JsonCallback done,
            ErrorCallback failed);
  // Calls done with the reply's JSON, or failed. An empty body is an empty
  // document.
  void HandleReply(QNetworkReply* reply, JsonCallback done,
                   ErrorCallback failed);

  static QList<Podcast> ParsePodcasts(const QJsonDocument& json);

  QNetworkAccessManager* network_;
  QString username_;
  QString password_;
};

#endif  // INTERNET_PODCASTS_GPODDERCLIENT_H_
