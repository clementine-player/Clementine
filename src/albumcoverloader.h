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

#ifndef ALBUMCOVERLOADER_H
#define ALBUMCOVERLOADER_H

#include "backgroundthread.h"

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QQueue>

class AlbumCoverLoader : public QObject {
  Q_OBJECT

 public:
  AlbumCoverLoader(QObject* parent = 0);

  static QString ImageCacheDir();

  void SetDesiredHeight(int height) { height_ = height; }
  quint64 LoadImageAsync(const QString& art_automatic, const QString& art_manual);

  void Clear();

  static QImage TryLoadImage(const QString& automatic, const QString& manual);
  static QPixmap TryLoadPixmap(const QString& automatic, const QString& manual);

  static const char* kManuallyUnsetCover;

 signals:
  void ImageLoaded(quint64 id, const QImage& image);

 private slots:
  void ProcessTasks();

 private:
  struct Task {
    quint64 id;
    QString art_automatic;
    QString art_manual;
  };

  int height_;

  QMutex mutex_;
  QQueue<Task> tasks_;
  quint64 next_id_;
};

#endif // ALBUMCOVERLOADER_H
