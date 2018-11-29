/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2014, Andreas <asfa194@gmail.com>
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

#include "albumcoverloader.h"

#include <QPainter>
#include <QDir>
#include <QCoreApplication>
#include <QUrl>
#include <QNetworkReply>

#include "config.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"
#include "internet/core/internetmodel.h"
#ifdef HAVE_SPOTIFY
#include "internet/spotify/spotifyservice.h"
#endif

AlbumCoverLoader::AlbumCoverLoader(QObject* parent)
    : QObject(parent),
      stop_requested_(false),
      next_id_(1),
      network_(new NetworkAccessManager(this)),
      connected_spotify_(false) {}

QString AlbumCoverLoader::ImageCacheDir() {
  return Utilities::GetConfigPath(Utilities::Path_AlbumCovers);
}

void AlbumCoverLoader::CancelTask(quint64 id) {
  QMutexLocker l(&mutex_);
  for (QQueue<Task>::iterator it = tasks_.begin(); it != tasks_.end(); ++it) {
    if (it->id == id) {
      tasks_.erase(it);
      break;
    }
  }
}

void AlbumCoverLoader::CancelTasks(const QSet<quint64>& ids) {
  QMutexLocker l(&mutex_);
  for (QQueue<Task>::iterator it = tasks_.begin(); it != tasks_.end();) {
    if (ids.contains(it->id)) {
      it = tasks_.erase(it);
    } else {
      ++it;
    }
  }
}

quint64 AlbumCoverLoader::LoadImageAsync(const AlbumCoverLoaderOptions& options,
                                         const QString& art_automatic,
                                         const QString& art_manual,
                                         const QString& song_filename,
                                         const QImage& embedded_image) {
  Task task;
  task.options = options;
  task.art_automatic = art_automatic;
  task.art_manual = art_manual;
  task.song_filename = song_filename;
  task.embedded_image = embedded_image;
  task.state = State_TryingManual;

  {
    QMutexLocker l(&mutex_);
    task.id = next_id_++;
    tasks_.enqueue(task);
  }

  metaObject()->invokeMethod(this, "ProcessTasks", Qt::QueuedConnection);

  return task.id;
}

void AlbumCoverLoader::ProcessTasks() {
  while (!stop_requested_) {
    // Get the next task
    Task task;
    {
      QMutexLocker l(&mutex_);
      if (tasks_.isEmpty()) return;
      task = tasks_.dequeue();
    }

    ProcessTask(&task);
  }
}

void AlbumCoverLoader::ProcessTask(Task* task) {
  TryLoadResult result = TryLoadImage(*task);
  if (result.started_async) {
    // The image is being loaded from a remote URL, we'll carry on later
    // when it's done
    return;
  }

  if (result.loaded_success) {
    QImage scaled = ScaleAndPad(task->options, result.image);
    emit ImageLoaded(task->id, scaled);
    emit ImageLoaded(task->id, scaled, result.image);
    return;
  }

  NextState(task);
}

void AlbumCoverLoader::NextState(Task* task) {
  if (task->state == State_TryingManual) {
    // Try the automatic one next
    task->state = State_TryingAuto;
    ProcessTask(task);
  } else {
    // Give up
    emit ImageLoaded(task->id, task->options.default_output_image_);
    emit ImageLoaded(task->id, task->options.default_output_image_,
                     task->options.default_output_image_);
  }
}

AlbumCoverLoader::TryLoadResult AlbumCoverLoader::TryLoadImage(
    const Task& task) {
  // An image embedded in the song itself takes priority
  if (!task.embedded_image.isNull())
    return TryLoadResult(false, true,
                         ScaleAndPad(task.options, task.embedded_image));

  QString filename;
  switch (task.state) {
    case State_TryingAuto:
      filename = task.art_automatic;
      break;
    case State_TryingManual:
      filename = task.art_manual;
      break;
  }

  if (filename == Song::kManuallyUnsetCover)
    return TryLoadResult(false, true, task.options.default_output_image_);

  if (filename == Song::kEmbeddedCover && !task.song_filename.isEmpty()) {
    const QImage taglib_image =
        TagReaderClient::Instance()->LoadEmbeddedArtBlocking(
            task.song_filename);

    if (!taglib_image.isNull())
      return TryLoadResult(false, true,
                           ScaleAndPad(task.options, taglib_image));
  }

  if (filename.toLower().startsWith("http://") ||
      filename.toLower().startsWith("https://")) {
    QUrl url(filename);
    QNetworkReply* reply = network_->get(QNetworkRequest(url));
    NewClosure(reply, SIGNAL(finished()), this,
               SLOT(RemoteFetchFinished(QNetworkReply*)), reply);

    remote_tasks_.insert(reply, task);
    return TryLoadResult(true, false, QImage());
  }
#ifdef HAVE_SPOTIFY
  else if (filename.toLower().startsWith("spotify://image/")) {
    // HACK: we should add generic image URL handlers
    SpotifyService* spotify = InternetModel::Service<SpotifyService>();

    if (!connected_spotify_) {
      connect(spotify, SIGNAL(ImageLoaded(QString, QImage)),
              SLOT(SpotifyImageLoaded(QString, QImage)));
      connected_spotify_ = true;
    }

    QString id = QUrl(filename).path();
    if (id.startsWith('/')) {
      id.remove(0, 1);
    }
    remote_spotify_tasks_.insert(id, task);

    // Need to schedule this in the spotify service's thread
    QMetaObject::invokeMethod(spotify, "LoadImage", Qt::QueuedConnection,
                              Q_ARG(QString, id));
    return TryLoadResult(true, false, QImage());
  }
#endif

  QImage image(filename);
  return TryLoadResult(
      false, !image.isNull(),
      image.isNull() ? task.options.default_output_image_ : image);
}

#ifdef HAVE_SPOTIFY
void AlbumCoverLoader::SpotifyImageLoaded(const QString& id,
                                          const QImage& image) {
  if (!remote_spotify_tasks_.contains(id)) return;

  Task task = remote_spotify_tasks_.take(id);
  QImage scaled = ScaleAndPad(task.options, image);
  emit ImageLoaded(task.id, scaled);
  emit ImageLoaded(task.id, scaled, image);
}
#endif

void AlbumCoverLoader::RemoteFetchFinished(QNetworkReply* reply) {
  reply->deleteLater();

  Task task = remote_tasks_.take(reply);

  // Handle redirects.
  QVariant redirect =
      reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (redirect.isValid()) {
    if (++task.redirects > kMaxRedirects) {
      return;  // Give up.
    }
    QNetworkRequest request = reply->request();
    request.setUrl(redirect.toUrl());
    QNetworkReply* redirected_reply = network_->get(request);
    NewClosure(redirected_reply, SIGNAL(finished()), this,
               SLOT(RemoteFetchFinished(QNetworkReply*)), redirected_reply);

    remote_tasks_.insert(redirected_reply, task);
    return;
  }

  if (reply->error() == QNetworkReply::NoError) {
    // Try to load the image
    QImage image;
    if (image.load(reply, 0)) {
      QImage scaled = ScaleAndPad(task.options, image);
      emit ImageLoaded(task.id, scaled);
      emit ImageLoaded(task.id, scaled, image);
      return;
    }
  }

  NextState(&task);
}

QImage AlbumCoverLoader::ScaleAndPad(const AlbumCoverLoaderOptions& options,
                                     const QImage& image) {
  if (image.isNull()) return image;

  // Scale the image down
  QImage copy;
  if (options.scale_output_image_) {
    copy = image.scaled(QSize(options.desired_height_, options.desired_height_),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
  } else {
    copy = image;
  }

  if (!options.pad_output_image_) return copy;

  // Pad the image to height_ x height_
  QImage padded_image(options.desired_height_, options.desired_height_,
                      QImage::Format_ARGB32);
  padded_image.fill(0);

  QPainter p(&padded_image);
  p.drawImage((options.desired_height_ - copy.width()) / 2,
              (options.desired_height_ - copy.height()) / 2, copy);
  p.end();

  return padded_image;
}

QPixmap AlbumCoverLoader::TryLoadPixmap(const QString& automatic,
                                        const QString& manual,
                                        const QString& filename) {
  QPixmap ret;
  if (manual == Song::kManuallyUnsetCover) return ret;
  if (!manual.isEmpty()) ret.load(manual);
  if (ret.isNull()) {
    if (automatic == Song::kEmbeddedCover && !filename.isNull())
      ret = QPixmap::fromImage(
          TagReaderClient::Instance()->LoadEmbeddedArtBlocking(filename));
    else if (!automatic.isEmpty())
      ret.load(automatic);
  }
  return ret;
}

quint64 AlbumCoverLoader::LoadImageAsync(const AlbumCoverLoaderOptions& options,
                                         const Song& song) {
  return LoadImageAsync(options, song.art_automatic(), song.art_manual(),
                        song.url().toLocalFile(), song.image());
}
