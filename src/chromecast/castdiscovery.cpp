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

#include "castdiscovery.h"

#include <QHostInfo>

#include "config.h"
#include "core/logging.h"

#if defined(Q_OS_DARWIN)
#include "bonjourcastdiscovery.h"
#elif defined(Q_OS_WIN32)
#include "windowscastdiscovery.h"
#elif defined(HAVE_DBUS)
#include "avahicastdiscovery.h"
#endif

const char* CastDiscovery::kServiceType = "_googlecast._tcp";

CastDiscovery* CastDiscovery::Create(QObject* parent) {
#if defined(Q_OS_DARWIN)
  return new BonjourCastDiscovery(parent);
#elif defined(Q_OS_WIN32)
  return new WindowsCastDiscovery(parent);
#elif defined(HAVE_DBUS)
  return new AvahiCastDiscovery(parent);
#else
  Q_UNUSED(parent);
  return nullptr;
#endif
}

CastDiscovery::CastDiscovery(QObject* parent) : QObject(parent) {}

void CastDiscovery::ServiceAdded(const QString& service) {
  live_services_ << service;
}

void CastDiscovery::ServiceResolved(const QString& service,
                                    const CastDevice& device) {
  if (!IsLive(service)) return;
  if (!device.is_valid()) {
    qLog(Debug) << "Ignoring incomplete Cast service" << service;
    return;
  }

  // The same advertisement can't normally change device, but if it did the
  // old device may have lost its last service.
  const QString old_id = service_ids_.value(service);
  if (!old_id.isEmpty() && old_id != device.id) ForgetService(service);
  service_ids_[service] = device.id;

  auto it = devices_.find(device.id);
  if (it != devices_.end() && *it == device) return;

  qLog(Info) << "Found Cast device" << device;
  devices_[device.id] = device;
  emit DeviceFound(device);
}

void CastDiscovery::ResolveHost(const QString& service,
                                const CastDevice& device, const QString& host) {
  QHostInfo::lookupHost(
      host, this, [this, service, device, host](const QHostInfo& info) {
        // IPv4, as Avahi is asked for: Cast devices all have an address, and
        // it's what the remote's network policy expects.
        for (const QHostAddress& address : info.addresses()) {
          if (address.protocol() != QAbstractSocket::IPv4Protocol) continue;
          CastDevice resolved = device;
          resolved.address = address;
          ServiceResolved(service, resolved);
          return;
        }
        qLog(Debug) << "No IPv4 address for Cast service" << service << "on"
                    << host << info.errorString();
      });
}

void CastDiscovery::ServiceRemoved(const QString& service) {
  live_services_.remove(service);
  ForgetService(service);
}

void CastDiscovery::ForgetService(const QString& service) {
  const QString id = service_ids_.take(service);
  if (id.isEmpty()) return;

  for (const QString& other_id : service_ids_) {
    if (other_id == id) return;
  }

  qLog(Info) << "Lost Cast device" << devices_.value(id);
  devices_.remove(id);
  emit DeviceLost(id);
}
