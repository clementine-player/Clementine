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

#ifndef CHROMECAST_CASTDISCOVERY_H_
#define CHROMECAST_CASTDISCOVERY_H_

#include <QMap>
#include <QObject>
#include <QString>

#include "castdevice.h"

// Finds Cast devices on the local network by browsing for _googlecast._tcp.
//
// A device is advertised on every interface and address family it's on, so
// the platform's browser reports each of those as a separate service. This
// class merges them by device id: DeviceFound comes once per device, and
// DeviceLost only when the last of its services goes away.
class CastDiscovery : public QObject {
  Q_OBJECT

 public:
  static const char* kServiceType;

  // The discovery for this platform, or nullptr if there isn't one.
  static CastDiscovery* Create(QObject* parent = nullptr);

  explicit CastDiscovery(QObject* parent = nullptr);

  virtual void Start() = 0;

  QList<CastDevice> devices() const { return devices_.values(); }

 signals:
  // A device appeared, or its name or address changed.
  void DeviceFound(const CastDevice& device);
  void DeviceLost(const QString& id);

 protected:
  // |service| identifies one advertisement, eg. by interface, protocol and
  // instance name, and is only compared with other values from the same
  // subclass.
  void ServiceResolved(const QString& service, const CastDevice& device);
  void ServiceRemoved(const QString& service);

 private:
  // Service to the id of the device it advertises.
  QMap<QString, QString> service_ids_;
  QMap<QString, CastDevice> devices_;
};

#endif  // CHROMECAST_CASTDISCOVERY_H_
