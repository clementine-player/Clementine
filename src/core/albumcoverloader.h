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
#include "song.h"

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QQueue>

class NetworkAccessManager;
class QNetworkReply;

class AlbumCoverLoader : public QObject {
  Q_OBJECT

 public:
  AlbumCoverLoader(QObject* parent = 0);

  void SetNetwork(NetworkAccessManager* network) { network_ = network; }
  
  void Stop() { stop_requested_ = true; }

  static QString ImageCacheDir();

  void SetDesiredHeight(int height) { height_ = height; }
  void SetPadOutputImage(bool padding) { padding_ = padding; }
  void SetDefaultOutputImage(const QImage& image);


  quint64 LoadImageAsync(const Song& song);
  quint64 LoadImageAsync(const QString& art_automatic,
                         const QString& art_manual,
                         const QString& song_filename = QString(),
                         const QImage& embedded_image = QImage());

  void Clear();

  static QPixmap TryLoadPixmap(const QString& automatic, const QString& manual);

  static const char* kManuallyUnsetCover;
  static const char* kEmbeddedCover;

 signals:
  void ImageLoaded(quint64 id, const QImage& image);

 private slots:
  void ProcessTasks();
  void RemoteFetchFinished(quint64 id, QNetworkReply* reply);

 private:
  enum State {
    State_TryingManual,
    State_TryingAuto,
  };

  struct Task {
    quint64 id;
    QString art_automatic;
    QString art_manual;
    QString song_filename;
    QImage embedded_image;
    State state;
  };

  struct TryLoadResult {
    TryLoadResult(bool async, bool success, const QImage& i)
      : started_async(async), loaded_success(success), image(i) {}

    bool started_async;
    bool loaded_success;
    QImage image;
  };

  void ProcessTask(Task* task);
  void NextState(Task* task);
  TryLoadResult TryLoadImage(const Task& task);
  QImage ScaleAndPad(const QImage& image) const;
  QImage LoadFromTaglib(const QString& filename) const;

  bool stop_requested_;

  int height_;
  bool padding_;
  QImage default_;

  QMutex mutex_;
  QQueue<Task> tasks_;
  QMap<quint64, Task> remote_tasks_;
  quint64 next_id_;

  NetworkAccessManager* network_;
};

#endif // ALBUMCOVERLOADER_H
