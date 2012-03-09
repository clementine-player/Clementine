/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "podcastbackend.h"
#include "podcastupdater.h"
#include "podcasturlloader.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/timeconstants.h"

#include <QSettings>
#include <QTimer>

const char* PodcastUpdater::kSettingsGroup = "Podcasts";

PodcastUpdater::PodcastUpdater(Application* app, QObject* parent)
  : QObject(parent),
    app_(app),
    update_interval_secs_(0),
    update_timer_(new QTimer(this)),
    loader_(new PodcastUrlLoader(this)),
    pending_replies_(0)
{
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
  connect(update_timer_, SIGNAL(timeout()), SLOT(UpdateAllPodcastsNow()));
  connect(app_->podcast_backend(), SIGNAL(SubscriptionAdded(Podcast)),
          SLOT(UpdatePodcastNow(Podcast)));

  update_timer_->setSingleShot(true);

  ReloadSettings();
}

void PodcastUpdater::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  last_full_update_ = s.value("last_full_update").toDateTime();
  update_interval_secs_ = s.value("update_interval_secs").toInt();

  RestartTimer();
}

void PodcastUpdater::RestartTimer() {
  // Stop any existing timer
  update_timer_->stop();

  if (update_interval_secs_ > 0) {
    if (!last_full_update_.isValid()) {
      // Updates are enabled and we've never updated before.  Do it now.
      qLog(Info) << "Updating podcasts for the first time";
      UpdateAllPodcastsNow();
    } else {
      const QDateTime next_update = last_full_update_.addSecs(update_interval_secs_);
      const int secs_until_next_update = QDateTime::currentDateTime().secsTo(next_update);

      if (secs_until_next_update < 0) {
        qLog(Info) << "Updating podcasts" << (-secs_until_next_update) << "seconds late";
        UpdateAllPodcastsNow();
      } else {
        qLog(Info) << "Updating podcasts at" << next_update << "(in" << secs_until_next_update << "seconds)";
        update_timer_->start(secs_until_next_update * kMsecPerSec);
      }
    }
  }
}

void PodcastUpdater::UpdatePodcastNow(const Podcast& podcast) {
  PodcastUrlLoaderReply* reply = loader_->Load(podcast.url());
  NewClosure(reply, SIGNAL(Finished(bool)),
             this, SLOT(PodcastLoaded(PodcastUrlLoaderReply*,Podcast,bool)),
             reply, podcast, false);
}

void PodcastUpdater::UpdateAllPodcastsNow() {
  foreach (const Podcast& podcast, app_->podcast_backend()->GetAllSubscriptions()) {
    PodcastUrlLoaderReply* reply = loader_->Load(podcast.url());
    NewClosure(reply, SIGNAL(Finished(bool)),
               this, SLOT(PodcastLoaded(PodcastUrlLoaderReply*,Podcast,bool)),
               reply, podcast, true);

    pending_replies_ ++;
  }
}

void PodcastUpdater::PodcastLoaded(PodcastUrlLoaderReply* reply, const Podcast& podcast,
                                   bool one_of_many) {
  reply->deleteLater();

  if (one_of_many) {
    if (--pending_replies_ == 0) {
      RestartTimer();
    }
  }

  if (!reply->is_success()) {
    qLog(Warning) << "Error fetching podcast at" << podcast.url() << ":"
                  << reply->error_text();
    return;
  }

  if (reply->result_type() != PodcastUrlLoaderReply::Type_Podcast) {
    qLog(Warning) << "The URL" << podcast.url() << "no longer contains a podcast";
    return;
  }

  // Get the episode URLs we had for this podcast already.
  QSet<QUrl> existing_urls;
  foreach (const PodcastEpisode& episode,
           app_->podcast_backend()->GetEpisodes(podcast.database_id())) {
    existing_urls.insert(episode.url());
  }

  // Add any new episodes
  PodcastEpisodeList new_episodes;
  foreach (const Podcast& reply_podcast, reply->podcast_results()) {
    foreach (const PodcastEpisode& episode, reply_podcast.episodes()) {
      if (!existing_urls.contains(episode.url())) {
        PodcastEpisode episode_copy(episode);
        episode_copy.set_podcast_database_id(podcast.database_id());
        new_episodes.append(episode_copy);
      }
    }
  }

  app_->podcast_backend()->AddEpisodes(&new_episodes);
  qLog(Info) << "Added" << new_episodes.count() << "new episodes for" << podcast.url();
}
