#ifndef NETWORKPROXYFACTORY_H
#define NETWORKPROXYFACTORY_H

#include <QMutex>
#include <QNetworkProxyFactory>
#include <QUrl>

class NetworkProxyFactory : public QNetworkProxyFactory {
 public:
  // These values are persisted
  enum Mode { Mode_System = 0, Mode_Direct = 1, Mode_Manual = 2, };

  static NetworkProxyFactory* Instance();
  static const char* kSettingsGroup;

  // These methods are thread-safe
  void ReloadSettings();
  QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery& query);

 private:
  NetworkProxyFactory();

  static NetworkProxyFactory* sInstance;

  QMutex mutex_;

  Mode mode_;
  QNetworkProxy::ProxyType type_;
  QString hostname_;
  int port_;
  bool use_authentication_;
  QString username_;
  QString password_;

#ifdef Q_OS_LINUX
  QUrl env_url_;
#endif
};

#endif  // NETWORKPROXYFACTORY_H
