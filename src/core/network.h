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

#ifndef NETWORK_H
#define NETWORK_H

#include <QAbstractNetworkCache>
#include <QMutex>
#include <QNetworkAccessManager>

class QNetworkDiskCache;

class ThreadSafeNetworkDiskCache : public QAbstractNetworkCache {
public:
  ThreadSafeNetworkDiskCache(QObject* parent);

  qint64 cacheSize() const;
  QIODevice* data(const QUrl& url);
  void insert(QIODevice* device);
  QNetworkCacheMetaData metaData(const QUrl& url);
  QIODevice* prepare(const QNetworkCacheMetaData& metaData);
  bool remove(const QUrl& url);
  void updateMetaData(const QNetworkCacheMetaData& metaData);

  void clear();

private:
  static QMutex sMutex;
  static QNetworkDiskCache* sCache;
};

class NetworkAccessManager : public QNetworkAccessManager {
  Q_OBJECT

public:
  NetworkAccessManager(QObject* parent = 0);

protected:
  QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                               QIODevice* outgoingData);
};

#endif // NETWORK_H
