/* This file is part of Clementine.
   Copyright 2013, Vlad Maltsev <shedwardx@gmail.com>

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

#ifndef VKMUSICCACHE_H
#define VKMUSICCACHE_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTemporaryFile>
#include <QUrl>

class VkService;
class Application;

class VkMusicCache : public QObject {
  Q_OBJECT

public:
  explicit VkMusicCache(Application* app, VkService* service);
  ~VkMusicCache() {}
  // Return file path if file in cache otherwise
  // return internet url and add song to caching queue
  QUrl Get(const QUrl& url);
  void ForceCache(const QUrl& url);
  void BreakCurrentCaching();
  bool InCache(const QUrl& url);

private slots:
  bool InCache(const QString& filename);
  void AddToQueue(const QString& filename, const QUrl& download_url);
  void DownloadNext();
  void DownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void DownloadReadyToRead();
  void Downloaded();

private:
  struct DownloadItem {
    QString filename;
    QUrl url;

    bool operator ==(const DownloadItem& rhv) {
      return filename == rhv.filename;
    }
  };

  QString CachedFilename(const QUrl& url);

  Application* app_;
  VkService* service_;
  QList<DownloadItem> queue_;
  // Contain index of current song in queue, need for removing if song was skipped.
  // Is zero if song downloading now, and less that zero if current song not caching or cached.
  int current_cashing_index;
  DownloadItem current_download;
  bool is_downloading;
  bool is_aborted;
  int task_id;
  QFile* file_;
  QNetworkAccessManager* network_manager_;
  QNetworkReply* reply_;
};

#endif  // VKMUSICCACHE_H
