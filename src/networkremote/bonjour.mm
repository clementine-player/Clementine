#include "bonjour.h"

#import <Foundation/NSNetServices.h>
#import <Foundation/NSString.h>

#include "core/logging.h"
#include "core/scoped_nsautorelease_pool.h"

@interface NetServicePublicationDelegate : NSObject<NSNetServiceDelegate> {
}

- (void)netServiceWillPublish:(NSNetService*)netService;
- (void)netService:(NSNetService*)netService
     didNotPublish:(NSDictionary*)errorDict;
- (void)netServiceDidStop:(NSNetService*)netService;

@end

@implementation NetServicePublicationDelegate

- (void)netServiceWillPublish:(NSNetService*)netService {
  qLog(Debug) << "Publishing:" << [[netService name] UTF8String];
}

- (void)netService:(NSNetService*)netServie
     didNotPublish:(NSDictionary*)errorDict {
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

Bonjour::Bonjour() : delegate_([[NetServicePublicationDelegate alloc] init]) {}

Bonjour::~Bonjour() { [delegate_ release]; }

void Bonjour::PublishInternal(const QString& domain, const QString& type,
                              const QByteArray& name, quint16 port) {
  ScopedNSAutoreleasePool pool;
  NSNetService* service = [[NSNetService alloc]
      initWithDomain:NSStringFromQString(domain)
                type:NSStringFromQString(type)
                name:[NSString stringWithUTF8String:name.constData()]
                port:port];
  if (service) {
    [service setDelegate:delegate_];
    [service publish];
  }
}
