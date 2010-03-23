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

#include <QPainter>
#include <QDir>
#include <QCoreApplication>

const char* AlbumCoverLoader::kManuallyUnsetCover = "(unset)";

AlbumCoverLoader::AlbumCoverLoader(QObject* parent)
  : QObject(parent),
    height_(120),
    next_id_(0)
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

  {
    QMutexLocker l(&mutex_);
    task.id = next_id_ ++;
    tasks_.enqueue(task);
  }

  metaObject()->invokeMethod(this, "ProcessTasks", Qt::QueuedConnection);

  return task.id;
}

void AlbumCoverLoader::ProcessTasks() {
  forever {
    // Get the next task
    Task task;
    {
      QMutexLocker l(&mutex_);
      if (tasks_.isEmpty())
        return;
      task = tasks_.dequeue();
    }

    // Try to load the image
    QImage image(TryLoadImage(task.art_automatic, task.art_manual));

    if (!image.isNull()) {
      // Scale the image down
      image = image.scaled(QSize(height_, height_), Qt::KeepAspectRatio, Qt::SmoothTransformation);

      // Pad the image to height_ x height_
      QImage bigger_image(height_, height_, QImage::Format_ARGB32);
      bigger_image.fill(0);

      QPainter p(&bigger_image);
      p.drawImage((height_ - image.width()) / 2, (height_ - image.height()) / 2,
                  image);
      p.end();

      image = bigger_image;
    }

    emit ImageLoaded(task.id, image);
  }
}

QImage AlbumCoverLoader::TryLoadImage(const QString &automatic, const QString &manual) {
  QImage ret;
  if (manual == kManuallyUnsetCover)
    return ret;
  if (!manual.isEmpty())
    ret.load(manual);
  if (!automatic.isEmpty() && ret.isNull())
    ret.load(automatic);
  return ret;
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
