#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QObject>
#include <QMap>

class QNetworkAccessManager;
class QNetworkDiskCache;
class QNetworkReply;
class QUrl;

// It's like QNetworkAccessManager, but threadsafe, and sets our User-Agent
// on all requests
class NetworkAccessManager : public QObject {
  Q_OBJECT

 public:
  NetworkAccessManager(QObject* parent = 0);

  // Only use this from the main thread
  QNetworkAccessManager* network() const { return network_; }

  // Thread-safe.  slot should take (quint64, QNetworkReply*)
  void Get(const QUrl& url, QObject* receiver, const char* slot,
           quint64 id, bool force_cache = false);

 private slots:
  void RunGet(const QUrl& url, QObject* receiver, const char* slot,
              quint64 id, bool force_cache);
  void RequestFinished();

 private:
  QNetworkAccessManager* network_;
  QNetworkDiskCache* cache_;

  struct Receiver {
    QObject* receiver;
    const char* slot;
    quint64 id;
  };

  QMap<QNetworkReply*, Receiver> pending_replies_;
};

#endif // NETWORKACCESSMANAGER_H
