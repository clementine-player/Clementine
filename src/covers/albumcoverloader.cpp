/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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
#include "config.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"
#include "internet/internetmodel.h"

#include <QPainter>
#include <QDir>
#include <QCoreApplication>
#include <QUrl>
#include <QNetworkReply>

#ifdef HAVE_SPOTIFY
# include "internet/spotifyservice.h"
#endif


AlbumCoverLoader::AlbumCoverLoader(QObject* parent)
  : QObject(parent),
    stop_requested_(false),
    height_(120),
    scale_(true),
    padding_(true),
    next_id_(0),
    network_(new NetworkAccessManager(this)),
    connected_spotify_(false)
{
}

QString AlbumCoverLoader::ImageCacheDir() {
  return Utilities::GetConfigPath(Utilities::Path_AlbumCovers);
}

void AlbumCoverLoader::Clear() {
  QMutexLocker l(&mutex_);
  tasks_.clear();
}

quint64 AlbumCoverLoader::LoadImageAsync(const QString& art_automatic,
                                         const QString& art_manual,
                                         const QString& song_filename,
                                         const QImage& embedded_image) {
  Task task;
  task.art_automatic = art_automatic;
  task.art_manual = art_manual;
  task.song_filename = song_filename;
  task.embedded_image = embedded_image;
  task.state = State_TryingManual;

  {
    QMutexLocker l(&mutex_);
    task.id = next_id_ ++;
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
      if (tasks_.isEmpty())
        return;
      task = tasks_.dequeue();
    }

    ProcessTask(&task);
  }
}

void AlbumCoverLoader::ProcessTask(Task *task) {
  TryLoadResult result = TryLoadImage(*task);
  if (result.started_async) {
    // The image is being loaded from a remote URL, we'll carry on later
    // when it's done
    return;
  }

  if (result.loaded_success) {
    QImage scaled = ScaleAndPad(result.image);
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
    emit ImageLoaded(task->id, default_);
    emit ImageLoaded(task->id, default_, default_);
  }
}

AlbumCoverLoader::TryLoadResult AlbumCoverLoader::TryLoadImage(
    const Task& task) {
  // An image embedded in the song itself takes priority
  if (!task.embedded_image.isNull())
    return TryLoadResult(false, true, ScaleAndPad(task.embedded_image));

  QString filename;
  switch (task.state) {
    case State_TryingAuto:   filename = task.art_automatic; break;
    case State_TryingManual: filename = task.art_manual;    break;
  }

  if (filename == Song::kManuallyUnsetCover)
    return TryLoadResult(false, true, default_);

  if (filename == Song::kEmbeddedCover && !task.song_filename.isEmpty()) {
    QImage taglib_image = Song::LoadEmbeddedArt(task.song_filename);
    if (!taglib_image.isNull())
      return TryLoadResult(false, true, ScaleAndPad(taglib_image));
  }

  if (filename.toLower().startsWith("http://")) {
    QNetworkReply* reply = network_->get(QNetworkRequest(filename));
    connect(reply, SIGNAL(finished()), SLOT(RemoteFetchFinished()));

    remote_tasks_.insert(reply, task);
    return TryLoadResult(true, false, QImage());
  } else if (filename.toLower().startsWith("spotify://image/")) {
    // HACK: we should add generic image URL handlers
    #ifdef HAVE_SPOTIFY
      SpotifyService* spotify = InternetModel::Service<SpotifyService>();

      if (!connected_spotify_) {
        connect(spotify, SIGNAL(ImageLoaded(QUrl,QImage)),
                SLOT(SpotifyImageLoaded(QUrl,QImage)));
        connected_spotify_ = true;
      }

      QUrl url = QUrl(filename);
      remote_spotify_tasks_.insert(url, task);

      // Need to schedule this in the spotify service's thread
      QMetaObject::invokeMethod(spotify, "LoadImage", Qt::QueuedConnection,
                                Q_ARG(QUrl, url));
      return TryLoadResult(true, false, QImage());
    #else
      return TryLoadResult(false, false, QImage());
    #endif
  }

  QImage image(filename);
  return TryLoadResult(false, !image.isNull(), image.isNull() ? default_ : image);
}

void AlbumCoverLoader::SpotifyImageLoaded(const QUrl& url, const QImage& image) {
  if (!remote_spotify_tasks_.contains(url))
    return;

  Task task = remote_spotify_tasks_.take(url);
  QImage scaled = ScaleAndPad(image);
  emit ImageLoaded(task.id, scaled);
  emit ImageLoaded(task.id, scaled, image);

  qLog(Debug) << "Spotify image was for task" << task.id;
}

void AlbumCoverLoader::RemoteFetchFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;
  reply->deleteLater();

  Task task = remote_tasks_.take(reply);

  // Handle redirects.
  QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (redirect.isValid()) {
    if (++task.redirects > kMaxRedirects) {
      return;  // Give up.
    }
    QNetworkRequest request = reply->request();
    request.setUrl(redirect.toUrl());
    QNetworkReply* redirected_reply = network_->get(request);
    connect(redirected_reply, SIGNAL(finished()), SLOT(RemoteFetchFinished()));

    remote_tasks_.insert(redirected_reply, task);
    return;
  }

  if (reply->error() == QNetworkReply::NoError) {
    // Try to load the image
    QImage image;
    if (image.load(reply, 0)) {
      QImage scaled = ScaleAndPad(image);
      emit ImageLoaded(task.id, scaled);
      emit ImageLoaded(task.id, scaled, image);
      return;
    }
  }

  NextState(&task);
}

QImage AlbumCoverLoader::ScaleAndPad(const QImage& image) const {
  if (image.isNull())
    return image;

  // Scale the image down
  QImage copy;
  if (scale_) {
    copy = image.scaled(QSize(height_, height_),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
  } else {
    copy = image;
  }

  if (!padding_)
    return copy;

  // Pad the image to height_ x height_
  QImage padded_image(height_, height_, QImage::Format_ARGB32);
  padded_image.fill(0);

  QPainter p(&padded_image);
  p.drawImage((height_ - copy.width()) / 2, (height_ - copy.height()) / 2,
              copy);
  p.end();

  return padded_image;
}

QPixmap AlbumCoverLoader::TryLoadPixmap(const QString& automatic,
                                        const QString& manual,
                                        const QString& filename) {
  QPixmap ret;
  if (manual == Song::kManuallyUnsetCover)
    return ret;
  if (!manual.isEmpty())
    ret.load(manual);
  if (ret.isNull()) {
    if (automatic == Song::kEmbeddedCover && !filename.isNull())
      ret = QPixmap::fromImage(Song::LoadEmbeddedArt(filename));
    else if (!automatic.isEmpty())
      ret.load(automatic);
  }
  return ret;
}

void AlbumCoverLoader::SetDefaultOutputImage(const QImage &image) {
  default_ = ScaleAndPad(image);
}

quint64 AlbumCoverLoader::LoadImageAsync(const Song &song) {
  return LoadImageAsync(song.art_automatic(), song.art_manual(),
                        song.url().toLocalFile(), song.image());
}
