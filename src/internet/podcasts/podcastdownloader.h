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

#ifndef INTERNET_PODCASTS_PODCASTDOWNLOADER_H_
#define INTERNET_PODCASTS_PODCASTDOWNLOADER_H_

#include <QFile>
#include <QList>
#include <QObject>
#include <QQueue>
#include <QRegExp>
#include <QSet>
#include <memory>

#include "core/network.h"
#include "podcast.h"
#include "podcastepisode.h"

#ifdef Q_OS_WIN
#include <time.h>
#else
#include <sys/time.h>
#endif

class Application;
class PodcastBackend;

class QNetworkAccessManager;

namespace PodcastDownload {
enum State { NotDownloading, Queued, Downloading, Finished };
}

class Task : public QObject {
  Q_OBJECT

 public:
  Task(PodcastEpisode episode, QFile* file, PodcastBackend* backend);
  PodcastEpisode episode() const;

 signals:
  void ProgressChanged(const PodcastEpisode& episode,
                       PodcastDownload::State state, int percent);
  void finished(Task* task);

 public slots:
  void finishedPublic();

 private slots:
  void reading();
  void downloadProgressInternal(qint64 received, qint64 total);
  void finishedInternal();

 private:
  std::unique_ptr<QFile> file_;
  PodcastEpisode episode_;
  QNetworkRequest req_;
  PodcastBackend* backend_;
  std::unique_ptr<NetworkAccessManager> network_;
  std::unique_ptr<RedirectFollower> repl;
};

class PodcastDownloader : public QObject {
  Q_OBJECT

 public:
  explicit PodcastDownloader(Application* app, QObject* parent = nullptr);

  static const char* kSettingsGroup;
  PodcastEpisodeList EpisodesDownloading(const PodcastEpisodeList& episodes);
  QString DefaultDownloadDir() const;

 public slots:
  // Adds the episode to the download queue
  void DownloadEpisode(const PodcastEpisode& episode);
  void cancelDownload(const PodcastEpisodeList& episodes);

 signals:
  void ProgressChanged(const PodcastEpisode& episode,
                       PodcastDownload::State state, int percent);

 private slots:
  void ReloadSettings();

  void SubscriptionAdded(const Podcast& podcast);
  void EpisodesAdded(const PodcastEpisodeList& episodes);

  void ReplyFinished(Task* task);

 private:
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

  QList<Task*> list_tasks_;
};

#endif  // INTERNET_PODCASTS_PODCASTDOWNLOADER_H_
