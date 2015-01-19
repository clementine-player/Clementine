/* This file is part of Clementine.
   Copyright 2014, Maltsev Vlad <shedwardx@gmail.com>
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
#include "vkmusiccache.h"

#include <qmath.h>
#include <QDir>

#include "core/application.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "vkservice.h"

VkMusicCache::VkMusicCache(Application* app, VkService* service)
    : QObject(service),
      app_(app),
      service_(service),
      current_song_index(0),
      is_downloading(false),
      is_aborted(false),
      task_id(0),
      file_(NULL),
      network_manager_(new QNetworkAccessManager),
      reply_(NULL) {}

QUrl VkMusicCache::Get(const QUrl& url) {
  QUrl result;
  if (InCache(url)) {
    QString cached_filename = CachedFilename(url);
    qLog(Info) << "Use cashed file" << cached_filename;
    result = QUrl::fromLocalFile(cached_filename);
  }
  return result;
}

void VkMusicCache::AddToCache(const QUrl& url, const QUrl& media_url,
                              bool force) {
  AddToQueue(CachedFilename(url), media_url);
  if (!force) {
    current_song_index = queue_.size();
  }
}

void VkMusicCache::BreakCurrentCaching() {
  if (current_song_index > 0) {
    // Current song in queue
    queue_.removeAt(current_song_index - 1);
  } else if (current_song_index == 0) {
    // Current song is downloading
    if (reply_) {
      reply_->abort();
      is_aborted = true;
    }
  }
}

/***
* Queue operations
*/

void VkMusicCache::AddToQueue(const QString& filename,
                              const QUrl& download_url) {
  DownloadItem item;
  item.filename = filename;
  item.url = download_url;
  queue_.push_back(item);
  DownloadNext();
}

/***
* Downloading
*/

void VkMusicCache::DownloadNext() {
  if (is_downloading || queue_.isEmpty()) {
    return;
  } else {
    current_download = queue_.first();
    queue_.pop_front();
    current_song_index--;

    // Check file path and file existance first
    if (QFile::exists(current_download.filename)) {
      qLog(Warning) << "Tried to overwrite already cached file"
                    << current_download.filename;
      return;
    }

    // Create temporarry file we download to.
    if (file_) {
      qLog(Warning) << "QFile" << file_->fileName() << "is not null";
      delete file_;
      file_ = NULL;
    }

    file_ = new QTemporaryFile;
    if (!file_->open(QFile::WriteOnly)) {
      qLog(Warning) << "Can not create temporary file" << file_->fileName()
                    << "Download right away to" << current_download.filename;
    }

    // Start downloading
    is_aborted = false;
    is_downloading = true;
    task_id = app_->task_manager()->StartTask(
        tr("Caching %1").arg(QFileInfo(current_download.filename).baseName()));
    reply_ = network_manager_->get(QNetworkRequest(current_download.url));
    connect(reply_, SIGNAL(finished()), SLOT(Downloaded()));
    connect(reply_, SIGNAL(readyRead()), SLOT(DownloadReadyToRead()));
    connect(reply_, SIGNAL(downloadProgress(qint64, qint64)),
            SLOT(DownloadProgress(qint64, qint64)));
    qLog(Info) << "Start cashing" << current_download.filename << "from"
               << current_download.url;
  }
}

void VkMusicCache::DownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
  if (bytesTotal) {
    int progress = qRound(100 * bytesReceived / bytesTotal);
    app_->task_manager()->SetTaskProgress(task_id, progress, 100);
  }
}

void VkMusicCache::DownloadReadyToRead() {
  if (file_) {
    file_->write(reply_->readAll());
  } else {
    qLog(Warning) << "Tried to write recived song to not created file";
  }
}

void VkMusicCache::Downloaded() {
  app_->task_manager()->SetTaskFinished(task_id);
  if (is_aborted || reply_->error()) {
    if (reply_->error()) {
      qLog(Info) << "Downloading failed" << reply_->errorString();
    }
  } else {
    DownloadReadyToRead();  // Save all recent recived data.

    QString path = service_->cacheDir();

    if (file_->size() > 0) {
      QDir(path).mkpath(QFileInfo(current_download.filename).path());
      if (file_->copy(current_download.filename)) {
        qLog(Info) << "Cached" << current_download.filename;
      } else {
        qLog(Error) << "Unable to save" << current_download.filename << ":"
                    << file_->errorString();
      }
    } else {
      qLog(Error) << "File" << current_download.filename << "is empty";
    }
  }

  delete file_;
  file_ = NULL;

  reply_->deleteLater();
  reply_ = NULL;

  is_downloading = false;
  DownloadNext();
}

/***
* Utils
*/

bool VkMusicCache::InCache(const QUrl& url) {
  return QFile::exists(CachedFilename(url));
}

QString VkMusicCache::CachedFilename(const QUrl& url) {
  QStringList args = url.path().split('/');

  QString cache_filename;
  if (args.size() == 4) {
    cache_filename = service_->cacheFilename();
    cache_filename.replace("%artist", args[2]);
    cache_filename.replace("%title", args[3]);
  } else {
    qLog(Warning) << "Song url with args" << args
                  << "does not contain artist and title"
                  << "use id as file name for cache.";
    cache_filename = args[1];
  }

  QString cache_dir = service_->cacheDir();
  if (cache_dir.isEmpty()) {
    qLog(Warning) << "Cache dir not defined";
    return "";
  }
  // TODO(Vk): Maybe use extenstion from link? Seems it's always mp3.
  return cache_dir + QDir::separator() + cache_filename + ".mp3";
}
