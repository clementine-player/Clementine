/* This file is part of Clementine.
   Copyright 2010, 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
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

#ifndef CORE_NETWORK_H_
#define CORE_NETWORK_H_

#include <QAbstractNetworkCache>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QNetworkDiskCache;

class ThreadSafeNetworkDiskCache : public QAbstractNetworkCache {
 public:
  explicit ThreadSafeNetworkDiskCache(QObject* parent);

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
  explicit NetworkAccessManager(QObject* parent = nullptr);
  explicit NetworkAccessManager(int timeout, QObject* parent = nullptr);

 protected:
  QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                               QIODevice* outgoingData);
  int timeout_msec_;
};

class RedirectFollower : public QObject {
  Q_OBJECT

 public:
  explicit RedirectFollower(QNetworkReply* first_reply, int max_redirects = 5);

  bool hit_redirect_limit() const { return redirects_remaining_ < 0; }
  QNetworkReply* reply() const { return current_reply_; }

  // These are all forwarded to the current reply.
  QNetworkReply::NetworkError error() const { return current_reply_->error(); }
  QString errorString() const { return current_reply_->errorString(); }
  QVariant attribute(QNetworkRequest::Attribute code) const {
    return current_reply_->attribute(code);
  }
  QVariant header(QNetworkRequest::KnownHeaders header) const {
    return current_reply_->header(header);
  }
  qint64 bytesAvailable() const { return current_reply_->bytesAvailable(); }
  QUrl url() const { return current_reply_->url(); }
  QByteArray readAll() { return current_reply_->readAll(); }
  void abort() { current_reply_->abort(); }

 signals:
  // These are all forwarded from the current reply.
  void readyRead();
  void error(QNetworkReply::NetworkError);
  void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
  void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

  // This is NOT emitted when a request that has a redirect finishes.
  void finished();

 private slots:
  void ReadyRead();
  void ReplyFinished();

 private:
  void ConnectReply(QNetworkReply* reply);

 private:
  QNetworkReply* current_reply_;
  int redirects_remaining_;
};

class NetworkTimeouts : public QObject {
  Q_OBJECT

 public:
  explicit NetworkTimeouts(int timeout_msec, QObject* parent = nullptr);

  // TODO(John Maguire): Template this to avoid code duplication.
  void AddReply(QNetworkReply* reply);
  void AddReply(RedirectFollower* reply);
  void SetTimeout(int msec) { timeout_msec_ = msec; }

 protected:
  void timerEvent(QTimerEvent* e);

 private slots:
  void ReplyFinished();
  void RedirectFinished(RedirectFollower* redirect);

 private:
  int timeout_msec_;
  QMap<QNetworkReply*, int> timers_;
  QMap<RedirectFollower*, int> redirect_timers_;
};

#endif  // CORE_NETWORK_H_
