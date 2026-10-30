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

#include "airplaydiscovery.h"

#include <QHostInfo>

#include "config.h"
#include "core/logging.h"

#if defined(Q_OS_DARWIN)
#include "bonjourairplaydiscovery.h"
#elif defined(Q_OS_WIN32)
#include "windowsairplaydiscovery.h"
#elif defined(HAVE_DBUS)
#include "avahiairplaydiscovery.h"
#endif

const char AirPlayDiscovery::kAirPlayServiceType[] = "_airplay._tcp";
const char AirPlayDiscovery::kRaopServiceType[] = "_raop._tcp";

AirPlayDiscovery* AirPlayDiscovery::Create(const char* service_type,
                                           QObject* parent) {
#if defined(Q_OS_DARWIN)
  return new BonjourAirPlayDiscovery(service_type, parent);
#elif defined(Q_OS_WIN32)
  return new WindowsAirPlayDiscovery(service_type, parent);
#elif defined(HAVE_DBUS)
  return new AvahiAirPlayDiscovery(service_type, parent);
#else
  Q_UNUSED(service_type);
  Q_UNUSED(parent);
  return nullptr;
#endif
}

AirPlayDiscovery::AirPlayDiscovery(const char* service_type, QObject* parent)
    : QObject(parent), service_type_(service_type) {}

bool AirPlayDiscovery::ParseService(const QString& instance,
                                    const QList<QByteArray>& txt,
                                    AirPlayDevice* device) const {
  if (qstrcmp(service_type_, kRaopServiceType) == 0) {
    return device->ParseRaopService(instance, txt);
  }
  if (!device->ParseTxt(txt)) return false;
  device->name = instance;
  return true;
}

void AirPlayDiscovery::ServiceAdded(const QString& service) {
  live_services_ << service;
}

void AirPlayDiscovery::ServiceResolved(const QString& service,
                                       const AirPlayDevice& device) {
  if (!IsLive(service)) return;
  if (!device.is_valid()) {
    qLog(Debug) << "Ignoring incomplete AirPlay service" << service;
    return;
  }

  // The same advertisement can't normally change device, but if it did the
  // old device may have lost its last service.
  const QString old_id = service_ids_.value(service);
  if (!old_id.isEmpty() && old_id != device.id) ForgetService(service);
  service_ids_[service] = device.id;

  auto it = devices_.find(device.id);
  if (it != devices_.end() && *it == device) return;

  qLog(Info) << "Found AirPlay receiver" << device;
  devices_[device.id] = device;
  emit DeviceFound(device);
}

void AirPlayDiscovery::ResolveHost(const QString& service,
                                   const AirPlayDevice& device,
                                   const QString& host) {
  QHostInfo::lookupHost(
      host, this, [this, service, device, host](const QHostInfo& info) {
        // IPv4, as Avahi is asked for: AirPlay receivers all have one.
        for (const QHostAddress& address : info.addresses()) {
          if (address.protocol() != QAbstractSocket::IPv4Protocol) continue;
          AirPlayDevice resolved = device;
          resolved.address = address;
          ServiceResolved(service, resolved);
          return;
        }
        qLog(Debug) << "No IPv4 address for AirPlay service" << service << "on"
                    << host << info.errorString();
      });
}

void AirPlayDiscovery::ServiceRemoved(const QString& service) {
  live_services_.remove(service);
  ForgetService(service);
}

void AirPlayDiscovery::ForgetService(const QString& service) {
  const QString id = service_ids_.take(service);
  if (id.isEmpty()) return;

  for (const QString& other_id : service_ids_) {
    if (other_id == id) return;
  }

  qLog(Info) << "Lost AirPlay receiver" << devices_.value(id);
  devices_.remove(id);
  emit DeviceLost(id);
}
