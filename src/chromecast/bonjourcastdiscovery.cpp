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

#include "bonjourcastdiscovery.h"

#include <QSocketNotifier>
#include <QtEndian>

#include "core/logging.h"

BonjourCastDiscovery::BonjourCastDiscovery(QObject* parent)
    : CastDiscovery(parent) {}

BonjourCastDiscovery::~BonjourCastDiscovery() {
  for (Resolve* resolve : resolves_) {
    Stop(&resolve->operation);
    delete resolve;
  }
  Stop(&browse_);
}

void BonjourCastDiscovery::Start() {
  if (browse_.ref) return;

  const DNSServiceErrorType error =
      DNSServiceBrowse(&browse_.ref, 0, kDNSServiceInterfaceIndexAny,
                       kServiceType, nullptr, &BrowseReply, this);
  if (error != kDNSServiceErr_NoError) {
    qLog(Warning) << "Couldn't look for Cast devices, error" << error;
    browse_.ref = nullptr;
    return;
  }
  Watch(&browse_);
  qLog(Debug) << "Looking for Cast devices";
}

void BonjourCastDiscovery::Watch(Operation* op) {
  op->notifier = new QSocketNotifier(DNSServiceRefSockFD(op->ref),
                                     QSocketNotifier::Read, this);
  DNSServiceRef ref = op->ref;
  connect(op->notifier, &QSocketNotifier::activated, this, [ref]() {
    // This may stop the operation, which deletes the notifier later.
    DNSServiceProcessResult(ref);
  });
}

void BonjourCastDiscovery::Stop(Operation* op) {
  if (op->notifier) {
    // Stop() can run inside the notifier's own signal.
    op->notifier->setEnabled(false);
    op->notifier->deleteLater();
    op->notifier = nullptr;
  }
  if (op->ref) {
    DNSServiceRefDeallocate(op->ref);
    op->ref = nullptr;
  }
}

QString BonjourCastDiscovery::ServiceKey(uint32_t iface, const char* name,
                                         const char* domain) {
  return QString("%1/%2.%3")
      .arg(iface)
      .arg(QString::fromUtf8(name), QString::fromUtf8(domain));
}

void DNSSD_API BonjourCastDiscovery::BrowseReply(
    DNSServiceRef, DNSServiceFlags flags, uint32_t iface,
    DNSServiceErrorType error, const char* name, const char* type,
    const char* domain, void* context) {
  BonjourCastDiscovery* self = static_cast<BonjourCastDiscovery*>(context);
  if (error != kDNSServiceErr_NoError) {
    qLog(Warning) << "Stopped looking for Cast devices, error" << error;
    Stop(&self->browse_);
    return;
  }

  if (flags & kDNSServiceFlagsAdd) {
    self->ServiceFound(iface, name, type, domain);
  } else {
    self->ServiceGone(ServiceKey(iface, name, domain));
  }
}

void BonjourCastDiscovery::ServiceFound(uint32_t iface, const char* name,
                                        const char* type, const char* domain) {
  const QString service = ServiceKey(iface, name, domain);
  ServiceAdded(service);
  CancelResolve(service);

  Resolve* resolve = new Resolve{this, service, Operation()};
  const DNSServiceErrorType error =
      DNSServiceResolve(&resolve->operation.ref, 0, iface, name, type, domain,
                        &ResolveReply, resolve);
  if (error != kDNSServiceErr_NoError) {
    qLog(Debug) << "Couldn't resolve Cast service" << service << ", error"
                << error;
    delete resolve;
    return;
  }
  Watch(&resolve->operation);
  resolves_[service] = resolve;
}

void BonjourCastDiscovery::ServiceGone(const QString& service) {
  CancelResolve(service);
  ServiceRemoved(service);
}

void BonjourCastDiscovery::CancelResolve(const QString& service) {
  Resolve* resolve = resolves_.take(service);
  if (!resolve) return;
  Stop(&resolve->operation);
  delete resolve;
}

void DNSSD_API BonjourCastDiscovery::ResolveReply(
    DNSServiceRef, DNSServiceFlags, uint32_t, DNSServiceErrorType error,
    const char*, const char* host, uint16_t port, uint16_t txt_length,
    const unsigned char* txt, void* context) {
  Resolve* resolve = static_cast<Resolve*>(context);
  BonjourCastDiscovery* self = resolve->owner;
  const QString service = resolve->service;

  CastDevice device;
  const bool ok = error == kDNSServiceErr_NoError &&
                  device.ParseTxt(CastDevice::SplitTxt(QByteArray(
                      reinterpret_cast<const char*>(txt), txt_length)));
  const QString host_name = QString::fromUtf8(host);
  // In network byte order.
  device.port = qFromBigEndian(port);

  // One answer is enough. Deallocating the ref from its own callback is
  // allowed.
  self->CancelResolve(service);

  if (!ok) {
    qLog(Debug) << "Couldn't resolve Cast service" << service << ", error"
                << error;
    return;
  }
  self->ResolveHost(service, device, host_name);
}
