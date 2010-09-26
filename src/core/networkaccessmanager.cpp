#include "networkaccessmanager.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkDiskCache>
#include <QCoreApplication>
#include <QDir>

#include <QtDebug>

NetworkAccessManager::NetworkAccessManager(QObject* parent,
                                           QNetworkAccessManager* injected)
      : QObject(parent),
        network_(injected ? injected : new QNetworkAccessManager(this)),
        cache_(new QNetworkDiskCache(this))
{
  cache_->setCacheDirectory(QString("%1/.config/%2/networkcache/")
      .arg(QDir::homePath(), QCoreApplication::organizationName()));
  network_->setCache(cache_);
}

void NetworkAccessManager::Get(const QUrl &url, QObject *receiver,
                               const char *slot, quint64 id, bool force_cache) {
  QMetaObject::invokeMethod(
      this, "RunGet", Qt::QueuedConnection,
      Q_ARG(QUrl, url), Q_ARG(QObject*, receiver),
      Q_ARG(const char*, slot),
      Q_ARG(quint64, id), Q_ARG(bool, force_cache));
}

void NetworkAccessManager::RunGet(const QUrl &url, QObject *receiver,
                                  const char *slot, quint64 id, bool force_cache) {
  QNetworkRequest req = CreateRequest(url, force_cache);
  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));

  Receiver r;
  r.receiver = receiver;
  r.slot = slot;
  r.id = id;

  pending_replies_.insert(reply, r);
}

QNetworkRequest NetworkAccessManager::CreateRequest(const QUrl& url, bool force_cache) {
  QNetworkRequest req(url);
  req.setRawHeader("User-Agent", QString("%1 %2").arg(
      QCoreApplication::applicationName(),
      QCoreApplication::applicationVersion()).toUtf8());

  if (force_cache) {
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                     QNetworkRequest::PreferCache);
  }
  return req;
}

void NetworkAccessManager::RequestFinished() {
  QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
  Receiver r = pending_replies_.take(reply);

  QMetaObject::invokeMethod(r.receiver, r.slot,
                            Q_ARG(quint64, r.id),
                            Q_ARG(QNetworkReply*, reply));
}

QNetworkReply* NetworkAccessManager::GetBlocking(const QUrl& url, bool force_cache) {
  QNetworkReply* reply = NULL;
  QMetaObject::invokeMethod(
      this, "RunGetBlocking", Qt::BlockingQueuedConnection,
      Q_ARG(QUrl, url), Q_ARG(bool, force_cache),
      Q_ARG(QNetworkReply**, &reply));
  return reply;
}

void NetworkAccessManager::RunGetBlocking(const QUrl& url, bool force_cache,
                                          QNetworkReply** reply) {
  QNetworkRequest req = CreateRequest(url, force_cache);
  *reply = network_->get(req);

  QEventLoop loop;
  connect(*reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();
}
