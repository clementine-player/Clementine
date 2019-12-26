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

#include "gpoddersync.h"

#include <QCoreApplication>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QTimer>

#include "podcastbackend.h"
#include "podcasturlloader.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/timeconstants.h"
#include "core/utilities.h"

const char* GPodderSync::kSettingsGroup = "Podcasts";
const int GPodderSync::kFlushUpdateQueueDelay = 30 * kMsecPerSec;  // 30 seconds
const int GPodderSync::kGetUpdatesInterval =
    30 * 60 * kMsecPerSec;  // 30 minutes
const int GPodderSync::kRequestTimeout = 30 * kMsecPerSec;  // 30 seconds

GPodderSync::GPodderSync(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      network_(new NetworkAccessManager(kRequestTimeout, this)),
      backend_(app_->podcast_backend()),
      loader_(new PodcastUrlLoader(this)),
      get_updates_timer_(new QTimer(this)),
      flush_queue_timer_(new QTimer(this)),
      flushing_queue_(false) {
  ReloadSettings();
  LoadQueue();

  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
  connect(backend_, SIGNAL(SubscriptionAdded(Podcast)),
          SLOT(SubscriptionAdded(Podcast)));
  connect(backend_, SIGNAL(SubscriptionRemoved(Podcast)),
          SLOT(SubscriptionRemoved(Podcast)));

  get_updates_timer_->setInterval(kGetUpdatesInterval);
  connect(get_updates_timer_, SIGNAL(timeout()), SLOT(GetUpdatesNow()));

  flush_queue_timer_->setInterval(kFlushUpdateQueueDelay);
  flush_queue_timer_->setSingleShot(true);
  connect(flush_queue_timer_, SIGNAL(timeout()), SLOT(FlushUpdateQueue()));

  if (is_logged_in()) {
    GetUpdatesNow();
    flush_queue_timer_->start();
    get_updates_timer_->start();
  }
}

GPodderSync::~GPodderSync() {}

QString GPodderSync::DeviceId() {
  return QString("%1-%2")
      .arg(qApp->applicationName(), QHostInfo::localHostName())
      .toLower();
}

QString GPodderSync::DefaultDeviceName() {
  return tr("%1 on %2")
      .arg(qApp->applicationName(), QHostInfo::localHostName());
}

bool GPodderSync::is_logged_in() const {
  return !username_.isEmpty() && !password_.isEmpty() && api_;
}

void GPodderSync::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  username_ = s.value("gpodder_username").toString();
  password_ = s.value("gpodder_password").toString();
  last_successful_get_ = s.value("gpodder_last_get").toDateTime();

  if (!username_.isEmpty() && !password_.isEmpty()) {
    api_.reset(new mygpo::ApiRequest(username_, password_, network_));
  }
}

QNetworkReply* GPodderSync::Login(const QString& username,
                                  const QString& password,
                                  const QString& device_name) {
  api_.reset(new mygpo::ApiRequest(username, password, network_));

  QNetworkReply* reply = api_->renameDevice(
      username, DeviceId(), device_name,
      Utilities::IsLaptop() ? mygpo::Device::LAPTOP : mygpo::Device::DESKTOP);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(LoginFinished(QNetworkReply*, QString, QString)), reply,
             username, password);
  return reply;
}

void GPodderSync::LoginFinished(QNetworkReply* reply, const QString& username,
                                const QString& password) {
  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    username_ = username;
    password_ = password;

    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue("gpodder_username", username);
    s.setValue("gpodder_password", password);

    DoInitialSync();
  } else {
    api_.reset();
  }
}

void GPodderSync::Logout() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("gpodder_username");
  s.remove("gpodder_password");
  s.remove("gpodder_last_get");

  api_.reset();
}

void GPodderSync::GetUpdatesNow() {
  if (!is_logged_in()) return;

  qlonglong timestamp = 0;
  if (last_successful_get_.isValid()) {
    timestamp = last_successful_get_.toTime_t();
  }

  mygpo::DeviceUpdatesPtr reply(
      api_->deviceUpdates(username_, DeviceId(), timestamp));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(DeviceUpdatesFinished(mygpo::DeviceUpdatesPtr)), reply);
  NewClosure(reply, SIGNAL(parseError()), this,
             SLOT(DeviceUpdatesFailed(mygpo::DeviceUpdatesPtr)), reply);
  NewClosure(reply, SIGNAL(requestError(QNetworkReply::NetworkError)), this,
             SLOT(DeviceUpdatesFailed(mygpo::DeviceUpdatesPtr)), reply);
}

void GPodderSync::DeviceUpdatesFailed(mygpo::DeviceUpdatesPtr reply) {
  qLog(Warning) << "Failed to get gpodder.net device updates";
}

void GPodderSync::DeviceUpdatesFinished(mygpo::DeviceUpdatesPtr reply) {
  // Remember episode actions for each podcast, so when we add a new podcast
  // we can apply the actions immediately.
  QMap<QUrl, QList<mygpo::EpisodePtr>> episodes_by_podcast;
  for (mygpo::EpisodePtr episode : reply->updateList()) {
    episodes_by_podcast[episode->podcastUrl()].append(episode);
  }

  for (mygpo::PodcastPtr podcast : reply->addList()) {
    const QUrl url(podcast->url());

    // Are we subscribed to this podcast already?
    Podcast existing_podcast = backend_->GetSubscriptionByUrl(url);
    if (existing_podcast.is_valid()) {
      // Just apply actions to this existing podcast
      ApplyActions(episodes_by_podcast[url],
                   existing_podcast.mutable_episodes());
      backend_->UpdateEpisodes(existing_podcast.episodes());
      continue;
    }

    // Start loading the podcast.  Remember actions and apply them after we
    // have a list of the episodes.
    PodcastUrlLoaderReply* loader_reply = loader_->Load(url);
    NewClosure(loader_reply, SIGNAL(Finished(bool)), this,
               SLOT(NewPodcastLoaded(PodcastUrlLoaderReply*, QUrl,
                                     QList<mygpo::EpisodePtr>)),
               loader_reply, url, episodes_by_podcast[url]);
  }

  // Unsubscribe from podcasts that were removed.
  for (const QUrl& url : reply->removeList()) {
    backend_->Unsubscribe(backend_->GetSubscriptionByUrl(url));
  }

  last_successful_get_ = QDateTime::currentDateTime();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("gpodder_last_get", last_successful_get_);
}

void GPodderSync::NewPodcastLoaded(PodcastUrlLoaderReply* reply,
                                   const QUrl& url,
                                   const QList<mygpo::EpisodePtr>& actions) {
  reply->deleteLater();

  if (!reply->is_success()) {
    qLog(Warning) << "Error fetching podcast at" << url << ":"
                  << reply->error_text();
    return;
  }

  if (reply->result_type() != PodcastUrlLoaderReply::Type_Podcast) {
    qLog(Warning) << "The URL" << url << "no longer contains a podcast";
    return;
  }

  // Apply the actions to the episodes in the podcast.
  for (Podcast podcast : reply->podcast_results()) {
    ApplyActions(actions, podcast.mutable_episodes());

    // Add the subscription
    backend_->Subscribe(&podcast);
  }
}

void GPodderSync::ApplyActions(
    const QList<QSharedPointer<mygpo::Episode>>& actions,
    PodcastEpisodeList* episodes) {
  for (PodcastEpisodeList::iterator it = episodes->begin();
       it != episodes->end(); ++it) {
    // Find an action for this episode
    for (mygpo::EpisodePtr action : actions) {
      if (action->url() != it->url()) continue;

      switch (action->status()) {
        case mygpo::Episode::PLAY:
        case mygpo::Episode::DOWNLOAD:
          it->set_listened(true);
          break;

        default:
          break;
      }
      break;
    }
  }
}

void GPodderSync::SubscriptionAdded(const Podcast& podcast) {
  if (!is_logged_in()) return;

  const QUrl& url = podcast.url();

  queued_remove_subscriptions_.remove(url);
  queued_add_subscriptions_.insert(url);

  SaveQueue();
  flush_queue_timer_->start();
}

void GPodderSync::SubscriptionRemoved(const Podcast& podcast) {
  if (!is_logged_in()) return;

  const QUrl& url = podcast.url();

  queued_remove_subscriptions_.insert(url);
  queued_add_subscriptions_.remove(url);

  SaveQueue();
  flush_queue_timer_->start();
}

namespace {
template <typename T>
void WriteContainer(const T& container, QSettings* s, const char* array_name,
                    const char* item_name) {
  s->beginWriteArray(array_name, container.count());
  int index = 0;
  for (const auto& item : container) {
    s->setArrayIndex(index++);
    s->setValue(item_name, item);
  }
  s->endArray();
}

template <typename T>
void ReadContainer(T* container, QSettings* s, const char* array_name,
                   const char* item_name) {
  container->clear();
  const int count = s->beginReadArray(array_name);
  for (int i = 0; i < count; ++i) {
    s->setArrayIndex(i);
    *container << s->value(item_name).value<typename T::value_type>();
  }
  s->endArray();
}
}  // namespace

void GPodderSync::SaveQueue() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  WriteContainer(queued_add_subscriptions_, &s,
                 "gpodder_queued_add_subscriptions", "url");
  WriteContainer(queued_remove_subscriptions_, &s,
                 "gpodder_queued_remove_subscriptions", "url");
}

void GPodderSync::LoadQueue() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  ReadContainer(&queued_add_subscriptions_, &s,
                "gpodder_queued_add_subscriptions", "url");
  ReadContainer(&queued_remove_subscriptions_, &s,
                "gpodder_queued_remove_subscriptions", "url");
}

void GPodderSync::FlushUpdateQueue() {
  if (!is_logged_in() || flushing_queue_) return;

  QSet<QUrl> all_urls =
      queued_add_subscriptions_ + queued_remove_subscriptions_;
  if (all_urls.isEmpty()) return;

  flushing_queue_ = true;
  mygpo::AddRemoveResultPtr reply(api_->addRemoveSubscriptions(
      username_, DeviceId(), queued_add_subscriptions_.toList(),
      queued_remove_subscriptions_.toList()));

  qLog(Info) << "Sending" << all_urls.count() << "changes to gpodder.net";

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(AddRemoveFinished(mygpo::AddRemoveResultPtr, QList<QUrl>)),
             reply, all_urls.toList());
  NewClosure(reply, SIGNAL(parseError()), this,
             SLOT(AddRemoveFailed(mygpo::AddRemoveResultPtr)), reply);
  NewClosure(reply, SIGNAL(requestError(QNetworkReply::NetworkError)), this,
             SLOT(AddRemoveFailed(mygpo::AddRemoveResultPtr)), reply);
}

void GPodderSync::AddRemoveFailed(mygpo::AddRemoveResultPtr reply) {
  flushing_queue_ = false;
  qLog(Warning) << "Failed to update gpodder.net subscriptions";
}

void GPodderSync::AddRemoveFinished(mygpo::AddRemoveResultPtr reply,
                                    const QList<QUrl>& affected_urls) {
  flushing_queue_ = false;

  // Remove the URLs from the queue.
  for (const QUrl& url : affected_urls) {
    queued_add_subscriptions_.remove(url);
    queued_remove_subscriptions_.remove(url);
  }

  SaveQueue();

  // Did more change in the mean time?
  if (!queued_add_subscriptions_.isEmpty() ||
      !queued_remove_subscriptions_.isEmpty()) {
    flush_queue_timer_->start();
  }
}

void GPodderSync::DoInitialSync() {
  // Get updates from the server
  GetUpdatesNow();
  get_updates_timer_->start();

  // Send our complete list of subscriptions
  queued_remove_subscriptions_.clear();
  queued_add_subscriptions_.clear();
  for (const Podcast& podcast : backend_->GetAllSubscriptions()) {
    queued_add_subscriptions_.insert(podcast.url());
  }

  SaveQueue();
  FlushUpdateQueue();
}
