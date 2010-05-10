/* This file is part of Clementine.

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
#include "networkaccessmanager.h"

#include <QPainter>
#include <QDir>
#include <QCoreApplication>
#include <QUrl>
#include <QNetworkReply>

const char* AlbumCoverLoader::kManuallyUnsetCover = "(unset)";

AlbumCoverLoader::AlbumCoverLoader(QObject* parent)
  : QObject(parent),
    stop_requested_(false),
    height_(120),
    next_id_(0),
    network_(NULL)
{
}

QString AlbumCoverLoader::ImageCacheDir() {
  return QString("%1/.config/%2/albumcovers/")
      .arg(QDir::homePath(), QCoreApplication::organizationName());
}

void AlbumCoverLoader::Clear() {
  QMutexLocker l(&mutex_);
  tasks_.clear();
}

quint64 AlbumCoverLoader::LoadImageAsync(const QString& art_automatic,
                                         const QString& art_manual) {
  Task task;
  task.art_automatic = art_automatic;
  task.art_manual = art_manual;
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
    emit ImageLoaded(task->id, ScaleAndPad(result.image));
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
    emit ImageLoaded(task->id, QImage());
  }
}

AlbumCoverLoader::TryLoadResult AlbumCoverLoader::TryLoadImage(
    const Task& task) {
  QString filename;
  switch (task.state) {
    case State_TryingAuto:   filename = task.art_automatic; break;
    case State_TryingManual: filename = task.art_manual;    break;
  }

  if (filename == kManuallyUnsetCover)
    return TryLoadResult(false, true, QImage());

  if (filename.toLower().startsWith("http://")) {
    network_->Get(QUrl(filename), this, "RemoteFetchFinished", task.id, true);

    remote_tasks_.insert(task.id, task);
    return TryLoadResult(true, false, QImage());
  }

  QImage image(filename);
  return TryLoadResult(false, !image.isNull(), image);
}

void AlbumCoverLoader::RemoteFetchFinished(quint64 id, QNetworkReply* reply) {
  Task task = remote_tasks_.take(id);

  if (reply->error() == QNetworkReply::NoError) {
    // Try to load the image
    QImage image;
    if (image.load(reply, 0)) {
      emit ImageLoaded(task.id, ScaleAndPad(image));
      reply->deleteLater();
      return;
    }
  }

  reply->deleteLater();
  NextState(&task);
}

QImage AlbumCoverLoader::ScaleAndPad(const QImage &image) const {
  if (image.isNull())
    return image;

  // Scale the image down
  QImage copy = image.scaled(QSize(height_, height_),
                             Qt::KeepAspectRatio, Qt::SmoothTransformation);

  // Pad the image to height_ x height_
  QImage bigger_image(height_, height_, QImage::Format_ARGB32);
  bigger_image.fill(0);

  QPainter p(&bigger_image);
  p.drawImage((height_ - copy.width()) / 2, (height_ - copy.height()) / 2,
              copy);
  p.end();

  return bigger_image;
}

QPixmap AlbumCoverLoader::TryLoadPixmap(const QString &automatic, const QString &manual) {
  QPixmap ret;
  if (manual == kManuallyUnsetCover)
    return ret;
  if (!manual.isEmpty())
    ret.load(manual);
  if (!automatic.isEmpty() && ret.isNull())
    ret.load(automatic);
  return ret;
}
