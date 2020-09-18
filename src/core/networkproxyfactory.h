/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef CORE_NETWORKPROXYFACTORY_H_
#define CORE_NETWORKPROXYFACTORY_H_

#include <QMutex>
#include <QNetworkProxyFactory>
#include <QUrl>

class NetworkProxyFactory : public QNetworkProxyFactory {
 public:
  // These values are persisted
  enum Mode {
    Mode_System = 0,
    Mode_Direct = 1,
    Mode_Manual = 2,
  };

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

#endif  // CORE_NETWORKPROXYFACTORY_H_
