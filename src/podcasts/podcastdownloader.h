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

class Application;
class PodcastBackend;

class QNetworkAccessManager;

class PodcastDownloader : public QObject {
  Q_OBJECT

public:
  PodcastDownloader(Application* app, QObject* parent = 0);

  enum State {
    NotDownloading,
    Queued,
    Downloading,
    Finished
  };

  static const char* kSettingsGroup;

  QString DefaultDownloadDir() const;

public slots:
  // Adds the episode to the download queue
  void DownloadEpisode(const PodcastEpisode& episode);

signals:
  void ProgressChanged(const PodcastEpisode& episode,
                       PodcastDownloader::State state, int percent);

private slots:
  void ReloadSettings();

  void SubscriptionAdded(const Podcast& podcast);
  void EpisodesAdded(const QList<PodcastEpisode>& episodes);

  void ReplyReadyRead();
  void ReplyFinished();
  void ReplyDownloadProgress(qint64 received, qint64 total);

private:
  struct Task;

  void StartDownloading(Task* task);
  void NextTask();

  QString FilenameForEpisode(const PodcastEpisode& episode) const;
  QString SanitiseFilenameComponent(const QString& text) const;

private:
  Application* app_;
  PodcastBackend* backend_;
  QNetworkAccessManager* network_;

  QRegExp disallowed_filename_characters_;

  QString download_dir_;

  Task* current_task_;
  QQueue<Task*> queued_tasks_;

  time_t last_progress_signal_;
};

#endif // PODCASTDOWNLOADER_H
