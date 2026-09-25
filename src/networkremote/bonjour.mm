#include "bonjour.h"

#import <Foundation/NSNetServices.h>
#import <Foundation/NSString.h>

#include <arpa/inet.h>
#include <dispatch/dispatch.h>
#include <dns_sd.h>

#include "core/logging.h"
#include "core/scoped_nsautorelease_pool.h"

@interface NetServicePublicationDelegate : NSObject <NSNetServiceDelegate> {
}

- (void)netServiceWillPublish:(NSNetService*)netService;
- (void)netService:(NSNetService*)netService didNotPublish:(NSDictionary*)errorDict;
- (void)netServiceDidStop:(NSNetService*)netService;

@end

@implementation NetServicePublicationDelegate

- (void)netServiceWillPublish:(NSNetService*)netService {
  qLog(Debug) << "Publishing:" << [[netService name] UTF8String];
}

- (void)netService:(NSNetService*)netServie didNotPublish:(NSDictionary*)errorDict {
  qLog(Debug) << "Failed to publish remote service with Bonjour";
  NSLog(@"%@", errorDict);
}

- (void)netServiceDidStop:(NSNetService*)netService {
  qLog(Debug) << "Unpublished:" << [[netService name] UTF8String];
}

@end

namespace {

NSString* NSStringFromQString(const QString& s) {
  return [[NSString alloc] initWithUTF8String:s.toUtf8().constData()];
}
}

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

Bonjour::Bonjour() : delegate_([[NetServicePublicationDelegate alloc] init]), service_(nil) {}

Bonjour::~Bonjour() {
  UnpublishInternal();
  [delegate_ release];
}

void Bonjour::PublishInternal(const QString& domain, const QString& type, const QByteArray& name,
                              quint16 port, const QList<QHostAddress>& addresses) {
  if (addresses.isEmpty()) {
    ScopedNSAutoreleasePool pool;
    service_ = [[NSNetService alloc] initWithDomain:NSStringFromQString(domain)
                                               type:NSStringFromQString(type)
                                               name:[NSString stringWithUTF8String:name.constData()]
                                               port:port];
    if (service_) {
      [service_ setDelegate:delegate_];
      [service_ publish];
    }
    return;
  }

  QList<int> interfaces;
  for (const QHostAddress& address : addresses) {
    const int iface = InterfaceIndexOf(address);
    if (iface < 0) {
      qLog(Warning) << "Not advertising the remote on" << address.toString()
                    << "- it isn't on any network interface";
      continue;
    }
    if (!interfaces.contains(iface)) interfaces << iface;
  }

  for (int iface : interfaces) {
    DNSServiceRef registration = nullptr;
    DNSServiceErrorType error = DNSServiceRegister(
        &registration, 0, iface, name.constData(), type.toUtf8().constData(),
        domain.toUtf8().constData(), nullptr, htons(port), 0, nullptr, LogRegistration, nullptr);
    if (error == kDNSServiceErr_NoError) {
      error = DNSServiceSetDispatchQueue(registration, RegistrationQueue());
    }
    if (error != kDNSServiceErr_NoError) {
      qLog(Warning) << "Failed to publish remote service with Bonjour on interface" << iface
                    << ", error" << error;
      if (registration) DNSServiceRefDeallocate(registration);
      continue;
    }
    registrations_ << registration;
  }
}

void Bonjour::UnpublishInternal() {
  if (service_) {
    [service_ stop];
    [service_ release];
    service_ = nil;
  }
  for (DNSServiceRef registration : registrations_) {
    dispatch_sync(RegistrationQueue(), ^{
      DNSServiceRefDeallocate(registration);
    });
  }
  registrations_.clear();
}
