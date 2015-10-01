#include "recordingnetworkaccessmanager.h"

#include <QMutexLocker>
#include <QNetworkReply>

#include "core/closure.h"
#include "core/logging.h"

QMutex RecordingNetworkAccessManager::sMutex;
QList<RecordingNetworkAccessManager::NetworkStat*>*
RecordingNetworkAccessManager::sTrafficStats;

RecordingNetworkAccessManager::NetworkStat::NetworkStat(
    QString url, QNetworkAccessManager::Operation op, int bytes_received)
  : url(url), operation(op), bytes_received(bytes_received) {}

RecordingNetworkAccessManager::RecordingNetworkAccessManager(QObject* parent)
    : QNetworkAccessManager(parent) {
  StaticInit();
}

QNetworkReply* RecordingNetworkAccessManager::createRequest(
    Operation op, const QNetworkRequest& req, QIODevice* data) {
  QNetworkReply* reply = QNetworkAccessManager::createRequest(op, req, data);
  NewClosure(
      reply, SIGNAL(finished()), this, SLOT(Finished(QNetworkReply*)), reply);
  // TODO: Listen to uploadProgress() too.
  return reply;
}

void RecordingNetworkAccessManager::Finished(QNetworkReply* reply) const {
  RecordRequest(*reply);
}

void RecordingNetworkAccessManager::StaticInit() {
  QMutexLocker l(&sMutex);
  if (sTrafficStats == nullptr) {
    sTrafficStats = new QList<NetworkStat*>;
  }
}

void RecordingNetworkAccessManager::RecordRequest(const QNetworkReply& reply) {
  qLog(Debug) << "Recording request" << reply.request().url() << sTrafficStats;
  QMutexLocker l(&sMutex);
  sTrafficStats->append(new NetworkStat(
      reply.request().url().toString(), reply.operation(), reply.size()));
  qLog(Debug) << "Stats:" << sTrafficStats->count();
}

QList<RecordingNetworkAccessManager::NetworkStat*>
RecordingNetworkAccessManager::GetNetworkStatistics() {
  QMutexLocker l(&sMutex);
  return *sTrafficStats;
}
