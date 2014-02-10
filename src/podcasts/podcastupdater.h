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

#ifndef PODCASTS_PODCASTUPDATER_H_
#define PODCASTS_PODCASTUPDATER_H_

#include <QDateTime>
#include <QObject>

class Application;
class Podcast;
class PodcastUrlLoader;
class PodcastUrlLoaderReply;

class QTimer;

// Responsible for updating podcasts when they're first subscribed to, and
// then updating them at regular intervals afterwards.
class PodcastUpdater : public QObject {
  Q_OBJECT

 public:
  PodcastUpdater(Application* app, QObject* parent = nullptr);

  static const char* kSettingsGroup;

 public slots:
  void UpdateAllPodcastsNow();
  void UpdatePodcastNow(const Podcast& podcast);

 private slots:
  void ReloadSettings();

  void SubscriptionAdded(const Podcast& podcast);
  void PodcastLoaded(PodcastUrlLoaderReply* reply, const Podcast& podcast,
                     bool one_of_many);

 private:
  void RestartTimer();
  void SaveSettings();

 private:
  Application* app_;

  QDateTime last_full_update_;
  int update_interval_secs_;

  QTimer* update_timer_;
  PodcastUrlLoader* loader_;
  int pending_replies_;
};

#endif  // PODCASTS_PODCASTUPDATER_H_
