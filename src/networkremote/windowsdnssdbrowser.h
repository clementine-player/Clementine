/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef NETWORKREMOTE_WINDOWSDNSSDBROWSER_H_
#define NETWORKREMOTE_WINDOWSDNSSDBROWSER_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <memory>

// Browses for one DNS-SD service type with the mDNS responder built into
// Windows 10 1809 and later, through DnsServiceBrowse and DnsServiceResolve,
// and resolves each service found. Used by the Cast and AirPlay discoveries.
//
// Windows calls back on threads of its own. The callbacks only copy what
// they were given and post it to the main thread, where the signals come.
class WindowsDnsSdBrowser : public QObject {
  Q_OBJECT

 public:
  // What a service resolved to.
  struct Service {
    // Its instance name, eg. "Living Room._airplay._tcp.local".
    QString instance;
    QString host;
    // Null if Windows only gave the host name.
    QHostAddress address;
    quint16 port = 0;
    // "key=value" entries.
    QList<QByteArray> txt;
  };

  // |service_type| is eg. "_googlecast._tcp".
  explicit WindowsDnsSdBrowser(const QString& service_type,
                               QObject* parent = nullptr);
  ~WindowsDnsSdBrowser();

  // Whether this version of Windows has the API.
  static bool Available();

  // False if it couldn't start.
  bool Start();

 signals:
  // |service| identifies one advertisement: its instance name, lowercased.
  void ServiceAdded(const QString& service);
  void ServiceRemoved(const QString& service);
  void ServiceResolved(const QString& service,
                       const WindowsDnsSdBrowser::Service& resolved);

 private:
  struct Shared;
  struct Browse;
  struct Resolve;

  void PtrRecord(const QString& instance, bool removed);
  void StartResolve(const QString& instance);

  const QString service_type_;
  QSet<QString> live_;
  // Shared with Windows' callbacks, which may still run after this is
  // destroyed.
  Shared* shared_;
  std::unique_ptr<Browse> browse_;
};

#endif  // NETWORKREMOTE_WINDOWSDNSSDBROWSER_H_
