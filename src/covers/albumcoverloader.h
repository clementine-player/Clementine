/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
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

#ifndef COVERS_ALBUMCOVERLOADER_H_
#define COVERS_ALBUMCOVERLOADER_H_

#include "albumcoverloaderoptions.h"
#include "config.h"
#include "core/song.h"

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QUrl>

class NetworkAccessManager;
class QNetworkReply;

class AlbumCoverLoader : public QObject {
  Q_OBJECT

 public:
  explicit AlbumCoverLoader(QObject* parent = nullptr);

  void Stop() { stop_requested_ = true; }

  static QString ImageCacheDir();

  quint64 LoadImageAsync(const AlbumCoverLoaderOptions& options,
                         const Song& song);
  virtual quint64 LoadImageAsync(const AlbumCoverLoaderOptions& options,
                                 const QString& art_automatic,
                                 const QString& art_manual,
                                 const QString& song_filename = QString(),
                                 const QImage& embedded_image = QImage());

  void CancelTask(quint64 id);
  void CancelTasks(const QSet<quint64>& ids);

  static QPixmap TryLoadPixmap(const QString& automatic, const QString& manual,
                               const QString& filename = QString());
  static QImage ScaleAndPad(const AlbumCoverLoaderOptions& options,
                            const QImage& image);

 signals:
  void ImageLoaded(quint64 id, const QImage& image);
  void ImageLoaded(quint64 id, const QImage& scaled, const QImage& original);

 protected slots:
  void ProcessTasks();
  void RemoteFetchFinished(QNetworkReply* reply);
#ifdef HAVE_SPOTIFY
  void SpotifyImageLoaded(const QString& url, const QImage& image);
#endif

 protected:
  enum State { State_TryingManual, State_TryingAuto, };

  struct Task {
    Task() : redirects(0) {}

    AlbumCoverLoaderOptions options;

    quint64 id;
    QString art_automatic;
    QString art_manual;
    QString song_filename;
    QImage embedded_image;
    State state;
    int redirects;
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

  bool stop_requested_;

  QMutex mutex_;
  QQueue<Task> tasks_;
  QMap<QNetworkReply*, Task> remote_tasks_;
  QMap<QString, Task> remote_spotify_tasks_;
  quint64 next_id_;

  NetworkAccessManager* network_;

  bool connected_spotify_;

  static const int kMaxRedirects = 3;
};

#endif  // COVERS_ALBUMCOVERLOADER_H_
