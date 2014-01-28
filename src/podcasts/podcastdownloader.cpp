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
#include "podcastdownloader.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/tagreaderclient.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "library/librarydirectorymodel.h"
#include "library/librarymodel.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>

const char* PodcastDownloader::kSettingsGroup = "Podcasts";
const int PodcastDownloader::kAutoDeleteCheckIntervalMsec = 15 * 60 * kMsecPerSec; // 15 minutes

struct PodcastDownloader::Task {
  Task() : file(NULL) {}
  ~Task() { delete file; }

  PodcastEpisode episode;
  QFile* file;
};

PodcastDownloader::PodcastDownloader(Application* app, QObject* parent)
  : QObject(parent),
    app_(app),
    backend_(app_->podcast_backend()),
    network_(new NetworkAccessManager(this)),
    disallowed_filename_characters_("[^a-zA-Z0-9_~ -]"),
    auto_download_(false),
    delete_after_secs_(0),
    current_task_(NULL),
    last_progress_signal_(0),
    auto_delete_timer_(new QTimer(this))
{
  connect(backend_, SIGNAL(EpisodesAdded(QList<PodcastEpisode>)),
          SLOT(EpisodesAdded(QList<PodcastEpisode>)));
  connect(backend_, SIGNAL(SubscriptionAdded(Podcast)),
          SLOT(SubscriptionAdded(Podcast)));
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
  connect(auto_delete_timer_, SIGNAL(timeout()), SLOT(AutoDelete()));

  auto_delete_timer_->setInterval(kAutoDeleteCheckIntervalMsec);
  auto_delete_timer_->start();

  ReloadSettings();
}

QString PodcastDownloader::DefaultDownloadDir() const {
  QString prefix = QDir::homePath();

  LibraryDirectoryModel* model = app_->library_model()->directory_model();
  if (model->rowCount() > 0) {
    // Download to the first library directory if there is one set
    prefix = model->index(0, 0).data().toString();
  }

  return prefix + "/Podcasts";
}

void PodcastDownloader::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  auto_download_ = s.value("auto_download", false).toBool();
  download_dir_ = s.value("download_dir", DefaultDownloadDir()).toString();
  delete_after_secs_ = s.value("delete_after", 0).toInt();
}

void PodcastDownloader::DownloadEpisode(const PodcastEpisode& episode) {
  if (downloading_episode_ids_.contains(episode.database_id()))
    return;
  downloading_episode_ids_.insert(episode.database_id());

  Task* task = new Task;
  task->episode = episode;

  if (current_task_) {
    // Add it to the queue
    queued_tasks_.enqueue(task);
    emit ProgressChanged(episode, Queued, 0);
  } else {
    // Start downloading now
    StartDownloading(task);
  }
}

void PodcastDownloader::DeleteEpisode(const PodcastEpisode& episode) {
  if (!episode.downloaded() || downloading_episode_ids_.contains(episode.database_id()))
    return;

  // Delete the local file
  if (!QFile::remove(episode.local_url().toLocalFile())) {
    qLog(Warning) << "The local file" << episode.local_url().toLocalFile()
                  << "could not be removed";
  }

  // Update the episode in the DB
  PodcastEpisode episode_copy(episode);
  episode_copy.set_downloaded(false);
  episode_copy.set_local_url(QUrl());
  backend_->UpdateEpisodes(PodcastEpisodeList() << episode_copy);
}

void PodcastDownloader::FinishAndDelete(Task* task) {
  Podcast podcast =
    backend_->GetSubscriptionById(task->episode.podcast_database_id());
  Song song = task->episode.ToSong(podcast);
  downloading_episode_ids_.remove(task->episode.database_id());
  emit ProgressChanged(task->episode, Finished, 0);
  // I didn't ecountered even a single podcast with a corect metadata
  TagReaderClient::Instance()->SaveFileBlocking(task->file->fileName(), song);
  delete task;

  NextTask();
}

QString PodcastDownloader::FilenameForEpisode(const QString& directory,
                                              const PodcastEpisode& episode) const {
  const QString file_extension = QFileInfo(episode.url().path()).suffix();
  int count = 0;
  
  // The file name contains the publication date and episode title
  QString base_filename = 
      episode.publication_date().date().toString(Qt::ISODate) + "-" +
      SanitiseFilenameComponent(episode.title());
  
  // Add numbers on to the end of the filename until we find one that doesn't
  // exist.
  forever {
    QString filename;
    
    if (count == 0) {
      filename = QString("%1/%2.%3").arg(
            directory, base_filename, file_extension);
    } else {
      filename = QString("%1/%2 (%3).%4").arg(
            directory, base_filename, QString::number(count), file_extension);
    }
    
    if (!QFile::exists(filename)) {
      return filename;
    }
    
    count ++;
  }
}

void PodcastDownloader::StartDownloading(Task* task) {
  current_task_ = task;

  // Need to get the name of the podcast to use in the directory name.
  Podcast podcast =
      backend_->GetSubscriptionById(task->episode.podcast_database_id());
  if (!podcast.is_valid()) {
    qLog(Warning) << "The podcast that contains episode" << task->episode.url()
                  << "doesn't exist any more";
    FinishAndDelete(task);
    return;
  }

  const QString directory = download_dir_ + "/" +
      SanitiseFilenameComponent(podcast.title());
  const QString filepath = FilenameForEpisode(directory, task->episode);

  // Open the output file
  QDir().mkpath(directory);
  task->file = new QFile(filepath);
  if (!task->file->open(QIODevice::WriteOnly)) {
    qLog(Warning) << "Could not open the file" << filepath << "for writing";
    FinishAndDelete(task);
    return;
  }

  qLog(Info) << "Downloading" << task->episode.url() << "to" << filepath;

  // Get the URL
  QNetworkRequest req(task->episode.url());
  RedirectFollower* reply = new RedirectFollower(network_->get(req));
  connect(reply, SIGNAL(readyRead()), SLOT(ReplyReadyRead()));
  connect(reply, SIGNAL(finished()), SLOT(ReplyFinished()));
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
          SLOT(ReplyDownloadProgress(qint64,qint64)));

  emit ProgressChanged(task->episode, Downloading, 0);
}

void PodcastDownloader::NextTask() {
  current_task_ = NULL;

  if (!queued_tasks_.isEmpty()) {
    StartDownloading(queued_tasks_.dequeue());
  }
}

void PodcastDownloader::ReplyReadyRead() {
  QNetworkReply* reply = qobject_cast<RedirectFollower*>(sender())->reply();
  if (!reply || !current_task_ || !current_task_->file)
    return;

  forever {
    const qint64 bytes = reply->bytesAvailable();
    if (bytes <= 0)
      break;

    current_task_->file->write(reply->read(bytes));
  }
}

void PodcastDownloader::ReplyDownloadProgress(qint64 received, qint64 total) {
  if (!current_task_ || !current_task_->file || total < 1024)
    return;

  const time_t current_time = QDateTime::currentDateTime().toTime_t();
  if (last_progress_signal_ == current_time)
    return;
  last_progress_signal_ = current_time;

  emit ProgressChanged(current_task_->episode, Downloading,
                       float(received) / total * 100);
}

void PodcastDownloader::ReplyFinished() {
  RedirectFollower* reply = qobject_cast<RedirectFollower*>(sender());
  if (!reply || !current_task_ || !current_task_->file)
    return;

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "Error downloading episode:" << reply->errorString();

    // Delete the file
    current_task_->file->remove();

    FinishAndDelete(current_task_);
    return;
  }

  qLog(Info) << "Download of" << current_task_->file->fileName() << "finished";

  // Tell the database the episode has been updated.  Get it from the DB again
  // in case the listened field changed in the mean time.
  PodcastEpisode episode = backend_->GetEpisodeById(current_task_->episode.database_id());
  episode.set_downloaded(true);
  episode.set_local_url(QUrl::fromLocalFile(current_task_->file->fileName()));
  backend_->UpdateEpisodes(PodcastEpisodeList() << episode);

  FinishAndDelete(current_task_);
}

QString PodcastDownloader::SanitiseFilenameComponent(const QString& text) const {
  return QString(text).replace(disallowed_filename_characters_, " ").simplified();
}

void PodcastDownloader::SubscriptionAdded(const Podcast& podcast) {
  EpisodesAdded(podcast.episodes());
}

void PodcastDownloader::EpisodesAdded(const QList<PodcastEpisode>& episodes) {
  if (auto_download_) {
    foreach (const PodcastEpisode& episode, episodes) {
      DownloadEpisode(episode);
    }
  }
}

void PodcastDownloader::AutoDelete() {
  if (delete_after_secs_ <= 0) {
    return;
  }

  QDateTime max_date = QDateTime::currentDateTime();
  max_date.addSecs(-delete_after_secs_);

  PodcastEpisodeList old_episodes = backend_->GetOldDownloadedEpisodes(max_date);
  if (old_episodes.isEmpty())
    return;

  qLog(Info) << "Deleting" << old_episodes.count()
             << "episodes because they were last listened to"
             << (delete_after_secs_ / kSecsPerDay)
             << "days ago";

  foreach (const PodcastEpisode& episode, old_episodes) {
    DeleteEpisode(episode);
  }
}
