#include "bonjour.h"

#include <arpa/inet.h>
#include <dispatch/dispatch.h>
#include <dns_sd.h>

#include "core/logging.h"

namespace {

void LogRegistration(DNSServiceRef, DNSServiceFlags, DNSServiceErrorType error, const char* name,
                     const char*, const char*, void*) {
  if (error == kDNSServiceErr_NoError) {
    qLog(Debug) << "Publishing:" << name;
  } else {
    qLog(Warning) << "Failed to publish remote service with Bonjour, error" << error;
  }
}

// Registrations handed to a dispatch queue have to be deallocated on that
// same queue, so they get one of their own.
dispatch_queue_t RegistrationQueue() {
  static dispatch_queue_t queue =
      dispatch_queue_create("org.clementine-player.bonjour", DISPATCH_QUEUE_SERIAL);
  return queue;
}

}  // namespace

Bonjour::Bonjour() {}

Bonjour::~Bonjour() { UnpublishInternal(); }

void Bonjour::PublishInternal(const QString& domain, const QString& type, const QByteArray& name,
                              quint16 port, const QList<QHostAddress>& addresses) {
  QList<uint32_t> interfaces;
  if (addresses.isEmpty()) {
    interfaces << kDNSServiceInterfaceIndexAny;
  } else {
    for (const QHostAddress& address : addresses) {
      const int iface = InterfaceIndexOf(address);
      if (iface < 0) {
        qLog(Warning) << "Not advertising the remote on" << address.toString()
                      << "- it isn't on any network interface";
        continue;
      }
      const uint32_t uiface = static_cast<uint32_t>(iface);
      if (!interfaces.contains(uiface)) interfaces << uiface;
    }
  }

  // Use DNSServiceRegister with a dispatch queue even for all-interface
  // publishing because NetworkRemote's thread has no NSRunLoop for
  // NSNetService delegates.
  for (uint32_t iface : interfaces) {
    DNSServiceRef registration = nullptr;
    DNSServiceErrorType error = DNSServiceRegister(
        &registration, 0, iface, name.constData(), type.toUtf8().constData(),
        domain.toUtf8().constData(), nullptr, htons(port), 0, nullptr, LogRegistration, nullptr);
    if (error == kDNSServiceErr_NoError) {
      error = DNSServiceSetDispatchQueue(registration, RegistrationQueue());
    }
    if (error != kDNSServiceErr_NoError) {
      qLog(Warning) << "Failed to publish remote service with Bonjour on "
                       "interface"
                    << iface << ", error" << error;
      // Deallocate even if DNSServiceSetDispatchQueue failed after register.
      if (registration) DNSServiceRefDeallocate(registration);
      continue;
    }
    registrations_ << registration;
  }
}

void Bonjour::UnpublishInternal() {
  for (DNSServiceRef registration : registrations_) {
    dispatch_sync(RegistrationQueue(), ^{
      DNSServiceRefDeallocate(registration);
    });
  }
  registrations_.clear();
}
