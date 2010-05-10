#include "networkaccessmanager.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkDiskCache>
#include <QCoreApplication>
#include <QDir>

#include <QtDebug>

NetworkAccessManager::NetworkAccessManager(QObject* parent)
      : QObject(parent),
        network_(new QNetworkAccessManager(this)),
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
  QNetworkRequest req(url);
  req.setRawHeader("User-Agent", QString("%1 %2").arg(
      QCoreApplication::applicationName(),
      QCoreApplication::applicationVersion()).toUtf8());

  if (force_cache) {
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                     QNetworkRequest::PreferCache);
  }

  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));

  Receiver r;
  r.receiver = receiver;
  r.slot = slot;
  r.id = id;

  pending_replies_.insert(reply, r);
}

void NetworkAccessManager::RequestFinished() {
  QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
  Receiver r = pending_replies_.take(reply);

  QMetaObject::invokeMethod(r.receiver, r.slot,
                            Q_ARG(quint64, r.id),
                            Q_ARG(QNetworkReply*, reply));
}
