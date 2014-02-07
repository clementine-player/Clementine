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

#ifndef PODCASTDOWNLOADER_H
#define PODCASTDOWNLOADER_H

#include "podcast.h"
#include "podcastepisode.h"

#include <QList>
#include <QObject>
#include <QQueue>
#include <QRegExp>
#include <QSet>

#ifdef Q_OS_WIN
#include <time.h>
#else
#include <sys/time.h>
#endif

class Application;
class PodcastBackend;

class QNetworkAccessManager;

class PodcastDownloader : public QObject {
  Q_OBJECT

 public:
  PodcastDownloader(Application* app, QObject* parent = 0);

  enum State { NotDownloading, Queued, Downloading, Finished };

  static const char* kSettingsGroup;
  static const int kAutoDeleteCheckIntervalMsec;

  QString DefaultDownloadDir() const;

 public slots:
  // Adds the episode to the download queue
  void DownloadEpisode(const PodcastEpisode& episode);

  // Deletes downloaded data for this episode
  void DeleteEpisode(const PodcastEpisode& episode);

signals:
  void ProgressChanged(const PodcastEpisode& episode,
                       PodcastDownloader::State state, int percent);

 private slots:
  void ReloadSettings();

  void SubscriptionAdded(const Podcast& podcast);
  void EpisodesAdded(const PodcastEpisodeList& episodes);

  void ReplyReadyRead();
  void ReplyFinished();
  void ReplyDownloadProgress(qint64 received, qint64 total);

  void AutoDelete();

 private:
  struct Task;

  void StartDownloading(Task* task);
  void NextTask();
  void FinishAndDelete(Task* task);

  QString FilenameForEpisode(const QString& directory,
                             const PodcastEpisode& episode) const;
  QString SanitiseFilenameComponent(const QString& text) const;

 private:
  Application* app_;
  PodcastBackend* backend_;
  QNetworkAccessManager* network_;

  QRegExp disallowed_filename_characters_;

  bool auto_download_;
  QString download_dir_;
  int delete_after_secs_;

  Task* current_task_;
  QQueue<Task*> queued_tasks_;
  QSet<int> downloading_episode_ids_;

  time_t last_progress_signal_;

  QTimer* auto_delete_timer_;
};

#endif  // PODCASTDOWNLOADER_H
