/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof A. Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

Task::Task(PodcastEpisode episode, QFile* file, PodcastBackend* backend)
  : file_(file),
    episode_(episode),
    backend_(backend),
    network_(new NetworkAccessManager(this)) {
  req_ = QNetworkRequest(episode_.url());
  repl = new RedirectFollower(network_->get(req_));
  connect(repl, SIGNAL(readyRead()), SLOT(reading()));
  connect(repl, SIGNAL(finished()), SLOT(finished_()));
  connect(repl, SIGNAL(downloadProgress(qint64, qint64)),
          SLOT(downloadProgress_(qint64, qint64)));
  emit ProgressChanged(episode_, PodcastDownload::Downloading, 0);
}

Task::~Task() {
  delete repl;
  delete file_;
  delete network_;
}

PodcastEpisode Task::episode() {
  return episode_;
}

void Task::reading() {
  qint64 bytes = 0;
  forever {
    bytes = repl->bytesAvailable();
    if (bytes <= 0) break;

    file_->write(repl->reply()->read(bytes));
  }
}

void Task::finished_() {
  if (repl->error() != QNetworkReply::NoError) {
    qLog(Warning) << "Error downloading episode:" << repl->errorString();
    emit ProgressChanged(episode_, PodcastDownload::NotDownloading, 0);
    // Delete the file
    file_->remove();
    emit finished();
    return;
  }

  qLog(Info) << "Download of" << file_->fileName() << "finished";

  // Tell the database the episode has been updated.  Get it from the DB again
  // in case the listened field changed in the mean time.
  PodcastEpisode episode = episode_;
  episode.set_downloaded(true);
  episode.set_local_url(QUrl::fromLocalFile(file_->fileName()));
  backend_->UpdateEpisodes(PodcastEpisodeList() << episode);
  Podcast podcast =
     backend_->GetSubscriptionById(episode.podcast_database_id());
  Song song = episode_.ToSong(podcast);

  emit ProgressChanged(episode_, PodcastDownload::Finished, 0);
  emit finished();

  // I didn't ecountered even a single podcast with a corect metadata
  TagReaderClient::Instance()->SaveFileBlocking(file_->fileName(), song);
}

void Task::downloadProgress_(qint64 received, qint64 total) {
  if (total <= 0) {
    emit ProgressChanged(episode_, PodcastDownload::Downloading, 0);
  } else {
    emit ProgressChanged(episode_, PodcastDownload::Downloading,
                         static_cast<float>(received) / total * 100);
  }
}

PodcastDownloader::PodcastDownloader(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      backend_(app_->podcast_backend()),
      network_(new NetworkAccessManager(this)),
      disallowed_filename_characters_("[^a-zA-Z0-9_~ -]"),
      auto_download_(false) {
  connect(backend_, SIGNAL(EpisodesAdded(PodcastEpisodeList)),
          SLOT(EpisodesAdded(PodcastEpisodeList)));
  connect(backend_, SIGNAL(SubscriptionAdded(Podcast)),
          SLOT(SubscriptionAdded(Podcast)));
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));

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
      filename =
          QString("%1/%2.%3").arg(directory, base_filename, file_extension);
    } else {
      filename = QString("%1/%2 (%3).%4").arg(
          directory, base_filename, QString::number(count), file_extension);
    }

    if (!QFile::exists(filename)) {
      return filename;
    }

    count++;
  }
}

void PodcastDownloader::DownloadEpisode(const PodcastEpisode& episode) {
  QFile* file = nullptr;
  Task* task = nullptr;

  for ( Task* tas : list_tasks_ ) {
    if (tas->episode().database_id() == episode.database_id()) {
      return;
    }
  }

  Podcast podcast =
      backend_->GetSubscriptionById(episode.podcast_database_id());
  if (!podcast.is_valid()) {
    qLog(Warning) << "The podcast that contains episode" << task->episode().url()
                  << "doesn't exist any more";
    return;
  }
  const QString directory =
      download_dir_ + "/" + SanitiseFilenameComponent(podcast.title());
  const QString filepath = FilenameForEpisode(directory, episode);

  // Open the output file
  QDir().mkpath(directory);
  file = new QFile(filepath);
  if (!file->open(QIODevice::WriteOnly)) {
    qLog(Warning) << "Could not open the file" << filepath << "for writing";
    return;
  }

  task = new Task(episode, file, backend_);

  list_tasks_ << task;
  qLog(Info) << "Downloading" << task->episode().url() << "to" << filepath;
  connect(task, SIGNAL(finished()), SLOT(ReplyFinished()));
  connect(task, SIGNAL(ProgressChanged(const PodcastEpisode&,
                                       PodcastDownload::State, int)),
          SIGNAL(ProgressChanged(const PodcastEpisode&,
                                 PodcastDownload::State, int)));
}

void PodcastDownloader::ReplyFinished() {
  Task* task = qobject_cast<Task*>(sender());
  list_tasks_.removeAll(task);
}

QString PodcastDownloader::SanitiseFilenameComponent(const QString& text)
    const {
  return QString(text)
      .replace(disallowed_filename_characters_, " ")
      .simplified();
}

void PodcastDownloader::SubscriptionAdded(const Podcast& podcast) {
  EpisodesAdded(podcast.episodes());
}

void PodcastDownloader::EpisodesAdded(const PodcastEpisodeList& episodes) {
  if (auto_download_) {
    for (const PodcastEpisode& episode : episodes) {
      DownloadEpisode(episode);
    }
  }
}

