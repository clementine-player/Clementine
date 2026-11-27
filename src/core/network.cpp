/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
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

#include "network.h"

#include <QCoreApplication>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>

#include "core/closure.h"
#include "core/logging.h"
#include "utilities.h"

QMutex ThreadSafeNetworkDiskCache::sMutex;

ThreadSafeNetworkDiskCache::ThreadSafeNetworkDiskCache(QObject* parent)
    : QAbstractNetworkCache(parent), cache_(new QNetworkDiskCache(this)) {
  // Each instance owns a QNetworkDiskCache on its own thread instead of
  // sharing one cross-thread QObject; sMutex still guards the shared disk dir.
  cache_->setCacheDirectory(
      Utilities::GetConfigPath(Utilities::Path_NetworkCache));
}

qint64 ThreadSafeNetworkDiskCache::cacheSize() const {
  QMutexLocker l(&sMutex);
  return cache_->cacheSize();
}

QIODevice* ThreadSafeNetworkDiskCache::data(const QUrl& url) {
  QMutexLocker l(&sMutex);
  return cache_->data(url);
}

void ThreadSafeNetworkDiskCache::insert(QIODevice* device) {
  QMutexLocker l(&sMutex);
  cache_->insert(device);
}

QNetworkCacheMetaData ThreadSafeNetworkDiskCache::metaData(const QUrl& url) {
  QMutexLocker l(&sMutex);
  return cache_->metaData(url);
}

QIODevice* ThreadSafeNetworkDiskCache::prepare(
    const QNetworkCacheMetaData& metaData) {
  QMutexLocker l(&sMutex);
  return cache_->prepare(metaData);
}

bool ThreadSafeNetworkDiskCache::remove(const QUrl& url) {
  QMutexLocker l(&sMutex);
  return cache_->remove(url);
}

void ThreadSafeNetworkDiskCache::updateMetaData(
    const QNetworkCacheMetaData& metaData) {
  QMutexLocker l(&sMutex);
  cache_->updateMetaData(metaData);
}

void ThreadSafeNetworkDiskCache::clear() {
  QMutexLocker l(&sMutex);
  cache_->clear();
}

NetworkAccessManager::NetworkAccessManager(QObject* parent)
    : QNetworkAccessManager(parent), timeout_msec_(0) {
  setCache(new ThreadSafeNetworkDiskCache(this));
}

NetworkAccessManager::NetworkAccessManager(int timeout, QObject* parent)
    : QNetworkAccessManager(parent), timeout_msec_(timeout) {
  setCache(new ThreadSafeNetworkDiskCache(this));
}

QNetworkReply* NetworkAccessManager::createRequest(
    Operation op, const QNetworkRequest& request, QIODevice* outgoingData) {
  qLogCat(Debug, "NetworkRequests") << request.url();
  QByteArray user_agent = QString("%1 %2")
                              .arg(QCoreApplication::applicationName(),
                                   QCoreApplication::applicationVersion())
                              .toUtf8();

  if (request.hasRawHeader("User-Agent")) {
    // Append the existing user-agent set by a client library (such as
    // libmygpo-qt).
    user_agent += " " + request.rawHeader("User-Agent");
  }

  QNetworkRequest new_request(request);
  new_request.setRawHeader("User-Agent", user_agent);

  if (op == QNetworkAccessManager::PostOperation &&
      !new_request.header(QNetworkRequest::ContentTypeHeader).isValid()) {
    new_request.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/x-www-form-urlencoded");
  }

  // Prefer the cache unless the caller has changed the setting already
  if (request.attribute(QNetworkRequest::CacheLoadControlAttribute).toInt() ==
      QNetworkRequest::PreferNetwork) {
    new_request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                             QNetworkRequest::PreferCache);
  }

  QNetworkReply* reply =
      QNetworkAccessManager::createRequest(op, new_request, outgoingData);
  if (timeout_msec_ > 0) {
    // Since the parent is the reply, this object will be destroyed when the
    // reply is destroyed.
    NetworkTimeouts* timeout = new NetworkTimeouts(timeout_msec_, reply);
    timeout->AddReply(reply);
  }
  return reply;
}

NetworkTimeouts::NetworkTimeouts(int timeout_msec, QObject* parent)
    : QObject(parent), timeout_msec_(timeout_msec) {}

void NetworkTimeouts::AddReply(QNetworkReply* reply) {
  if (timers_.contains(reply)) return;

  connect(reply, SIGNAL(destroyed()), SLOT(ReplyFinished()));
  connect(reply, SIGNAL(finished()), SLOT(ReplyFinished()));
  timers_[reply] = startTimer(timeout_msec_);
}

void NetworkTimeouts::AddReply(RedirectFollower* reply) {
  if (redirect_timers_.contains(reply)) {
    return;
  }

  NewClosure(reply, &RedirectFollower::destroyed, this,
             [this, reply](QObject*) { RedirectFinished(reply); });
  NewClosure(reply, &RedirectFollower::finished, this,
             &NetworkTimeouts::RedirectFinished, reply);
  redirect_timers_[reply] = startTimer(timeout_msec_);
}

void NetworkTimeouts::ReplyFinished() {
  QNetworkReply* reply = reinterpret_cast<QNetworkReply*>(sender());
  if (timers_.contains(reply)) {
    killTimer(timers_.take(reply));
  }
}

void NetworkTimeouts::RedirectFinished(RedirectFollower* reply) {
  if (redirect_timers_.contains(reply)) {
    killTimer(redirect_timers_.take(reply));
  }
}

void NetworkTimeouts::timerEvent(QTimerEvent* e) {
  QNetworkReply* reply = timers_.key(e->timerId());
  if (reply) {
    reply->abort();
  }

  RedirectFollower* redirect = redirect_timers_.key(e->timerId());
  if (redirect) {
    redirect->abort();
  }
}

RedirectFollower::RedirectFollower(QNetworkReply* first_reply,
                                   int max_redirects)
    : QObject(nullptr),
      current_reply_(first_reply),
      redirects_remaining_(max_redirects) {
  if (first_reply) {
    ConnectReply(first_reply);
  }
}

RedirectFollower::~RedirectFollower() {
  if (current_reply_) {
    // Disconnect first so abort() cannot trigger slots during destruction,
    // and defer reply deletion in case NAM or callers still hold references.
    current_reply_->disconnect(this);
    if (current_reply_->isRunning()) {
      current_reply_->abort();
    }
    current_reply_->deleteLater();
    current_reply_.clear();
  }
}

void RedirectFollower::ConnectReply(QNetworkReply* reply) {
  connect(reply, SIGNAL(readyRead()), SLOT(ReadyRead()));
  connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)),
          SIGNAL(error(QNetworkReply::NetworkError)));
  connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
          SIGNAL(downloadProgress(qint64, qint64)));
  connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
          SIGNAL(uploadProgress(qint64, qint64)));
  connect(reply, SIGNAL(finished()), SLOT(ReplyFinished()));
}

void RedirectFollower::ReadyRead() {
  if (!current_reply_) return;

  // Don't re-emit this signal for redirect replies.
  if (current_reply_->attribute(QNetworkRequest::RedirectionTargetAttribute)
          .isValid()) {
    return;
  }

  emit readyRead();
}

void RedirectFollower::ReplyFinished() {
  // Snapshot and disconnect before scheduling deletion or issuing a redirect,
  // so re-entrant signals cannot replace current_reply_ mid-method or fire
  // stale handlers on the completed reply.
  QNetworkReply* old_reply = current_reply_.data();
  if (!old_reply) return;

  old_reply->disconnect(this);
  old_reply->deleteLater();

  const QVariant redirect_target =
      old_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (redirect_target.isValid()) {
    if (redirects_remaining_-- == 0) {
      emit finished();
      return;
    }

    const QUrl next_url = old_reply->url().resolved(redirect_target.toUrl());

    QNetworkRequest req(old_reply->request());
    req.setUrl(next_url);

    QNetworkAccessManager* manager = old_reply->manager();
    if (!manager) {
      // Emit finished() so callers don't hang if the NAM was already destroyed.
      emit finished();
      return;
    }

    current_reply_ = manager->get(req);
    if (current_reply_) {
      ConnectReply(current_reply_);
      return;
    }
    // Fall through to emit finished() if get() failed to create a reply.
  }

  emit finished();
}
