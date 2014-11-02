/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
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

#ifndef PODCASTS_GPODDERSYNC_H_
#define PODCASTS_GPODDERSYNC_H_

#include "podcastepisode.h"

#include <QDateTime>
#include <QObject>
#include <QScopedPointer>
#include <QSet>
#include <QSharedPointer>
#include <QUrl>

#include <ApiRequest.h>

class Application;
class Podcast;
class PodcastBackend;
class PodcastUrlLoader;
class PodcastUrlLoaderReply;

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class GPodderSync : public QObject {
  Q_OBJECT

 public:
  GPodderSync(Application* app, QObject* parent = nullptr);
  ~GPodderSync();

  static const char* kSettingsGroup;
  static const int kFlushUpdateQueueDelay;
  static const int kGetUpdatesInterval;

  static QString DefaultDeviceName();
  static QString DeviceId();

  bool is_logged_in() const;

  // Tries to login using the given username and password.  Also sets the
  // device name and type on gpodder.net.  You do NOT need to deleteLater()
  // the QNetworkReply returned from this function.
  // If login succeeds the username and password will be saved in QSettings.
  QNetworkReply* Login(const QString& username, const QString& password,
                       const QString& device_name);

  // Clears any saved username and password from QSettings.
  void Logout();

 public slots:
  void GetUpdatesNow();

 private slots:
  void ReloadSettings();
  void LoginFinished(QNetworkReply* reply, const QString& username,
                     const QString& password);

  void DeviceUpdatesFinished(mygpo::DeviceUpdatesPtr reply);
  void DeviceUpdatesFailed(mygpo::DeviceUpdatesPtr reply);

  void NewPodcastLoaded(PodcastUrlLoaderReply* reply, const QUrl& url,
                        const QList<mygpo::EpisodePtr>& actions);

  void ApplyActions(const QList<mygpo::EpisodePtr>& actions,
                    PodcastEpisodeList* episodes);

  void SubscriptionAdded(const Podcast& podcast);
  void SubscriptionRemoved(const Podcast& podcast);
  void FlushUpdateQueue();

  void AddRemoveFinished(mygpo::AddRemoveResultPtr reply,
                         const QList<QUrl>& affected_urls);
  void AddRemoveFailed(mygpo::AddRemoveResultPtr reply);

 private:
  void LoadQueue();
  void SaveQueue();

  void DoInitialSync();

 private:
  Application* app_;
  QNetworkAccessManager* network_;
  QScopedPointer<mygpo::ApiRequest> api_;

  PodcastBackend* backend_;
  PodcastUrlLoader* loader_;

  QString username_;
  QString password_;
  QDateTime last_successful_get_;
  QTimer* get_updates_timer_;

  QTimer* flush_queue_timer_;
  QSet<QUrl> queued_add_subscriptions_;
  QSet<QUrl> queued_remove_subscriptions_;
  bool flushing_queue_;
};

#endif  // PODCASTS_GPODDERSYNC_H_
