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

#ifndef AIRPLAY_AIRPLAYDISCOVERY_H_
#define AIRPLAY_AIRPLAYDISCOVERY_H_

#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>

#include "airplaydevice.h"

// Finds AirPlay receivers on the local network by browsing for _airplay._tcp.
//
// A device is advertised on every interface and address family it's on, so
// the platform's browser reports each of those as a separate service. This
// class merges them by device id: DeviceFound comes once per device, and
// DeviceLost only when the last of its services goes away.
class AirPlayDiscovery : public QObject {
  Q_OBJECT

 public:
  static const char* kServiceType;

  // The discovery for this platform, or nullptr if there isn't one.
  static AirPlayDiscovery* Create(QObject* parent = nullptr);

  explicit AirPlayDiscovery(QObject* parent = nullptr);

  virtual void Start() = 0;

  QList<AirPlayDevice> devices() const { return devices_.values(); }

 signals:
  // A device appeared, or its name or address changed.
  void DeviceFound(const AirPlayDevice& device);
  void DeviceLost(const QString& id);

 protected:
  // Subclasses report each advertisement as it comes and goes, then what it
  // resolves to. |service| identifies one advertisement, eg. by interface,
  // protocol and instance name, and is only compared with other values from
  // the same subclass.
  void ServiceAdded(const QString& service);
  void ServiceRemoved(const QString& service);
  bool IsLive(const QString& service) const {
    return live_services_.contains(service);
  }

  // Ignored unless |service| is still live, so a resolve that finishes after
  // its advertisement went away doesn't bring the device back.
  void ServiceResolved(const QString& service, const AirPlayDevice& device);
  // For platforms whose resolver gives a host name rather than an address:
  // looks up |host|'s IPv4 address, then calls ServiceResolved.
  void ResolveHost(const QString& service, const AirPlayDevice& device,
                   const QString& host);

 private:
  // Forgets which device |service| advertised, and emits DeviceLost if it
  // was that device's last one.
  void ForgetService(const QString& service);

  QSet<QString> live_services_;
  // Service to the id of the device it advertises.
  QMap<QString, QString> service_ids_;
  QMap<QString, AirPlayDevice> devices_;
};

#endif  // AIRPLAY_AIRPLAYDISCOVERY_H_
