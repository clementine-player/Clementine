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

#ifndef CHROMECAST_AVAHICASTDISCOVERY_H_
#define CHROMECAST_AVAHICASTDISCOVERY_H_

#include <QSet>

#include "castdiscovery.h"

class QDBusMessage;

// Browses with avahi-daemon over D-Bus.
//
// The browser is created with ServiceBrowserPrepare (Avahi 0.8 and later) and
// only started once its signals are connected. ServiceBrowserNew starts
// straight away, so devices that answer quickly could be reported before
// anything is listening.
class AvahiCastDiscovery : public CastDiscovery {
  Q_OBJECT

 public:
  explicit AvahiCastDiscovery(QObject* parent = nullptr);
  ~AvahiCastDiscovery();

  void Start() override;

 private slots:
  void ItemNew(int iface, int protocol, const QString& name,
               const QString& type, const QString& domain, uint flags);
  void ItemRemove(int iface, int protocol, const QString& name,
                  const QString& type, const QString& domain, uint flags);
  void Failure(const QString& error);

 private:
  static QString ServiceKey(int iface, int protocol, const QString& name,
                            const QString& domain);

  void BrowserPrepared(const QDBusMessage& reply);
  void Resolved(const QString& service, const QDBusMessage& reply);

  QString browser_path_;
  // Services Avahi has reported and not yet removed. A resolve that finishes
  // after its service went away is ignored.
  QSet<QString> services_;
};

#endif  // CHROMECAST_AVAHICASTDISCOVERY_H_
