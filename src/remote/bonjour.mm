#include "bonjour.h"

#include <QtDebug>

#import <Foundation/Foundation.h>

@interface NetServicePublicationDelegate : NSObject <NSNetServiceDelegate> {
  NSMutableArray* services_;
}

- (void)netServiceWillPublish:(NSNetService*)netService;
- (void)netService:(NSNetService*)netService
    didNotPublish:(NSDictionary*)errorDict;
- (void)netServiceDidStop:(NSNetService*)netService;

@end

@implementation NetServicePublicationDelegate

- (id)init {
  self = [super init];
  if (self) {
    services_ = [[NSMutableArray alloc] init];
  }
  return self;
}

- (void)dealloc {
  [services_ release];
  [super dealloc];
}

- (void)netServiceWillPublish: (NSNetService*)netService {
  [services_ addObject: netService];
  qDebug() << Q_FUNC_INFO
           << [[netService name] UTF8String];
}

- (void)netService: (NSNetService*)netService didNotPublish: (NSDictionary*)errorDict {
  [services_ removeObject: netService];
  qDebug() << Q_FUNC_INFO;
  NSLog(@"%@", errorDict);
}

- (void)netServiceDidStop: (NSNetService*)netService {
  [services_ removeObject: netService];
  qDebug() << Q_FUNC_INFO;
}

@end

class BonjourWrapper {
 public:
  BonjourWrapper() {
    delegate_ = [[NetServicePublicationDelegate alloc] init];
  }

  NetServicePublicationDelegate* delegate() const { return delegate_; }

 private:
  NetServicePublicationDelegate* delegate_;
};

#define QSTRING_TO_NSSTRING(x) \
    [[NSString alloc] initWithUTF8String:x.toUtf8().constData()]

Bonjour::Bonjour()
    : wrapper_(new BonjourWrapper) {
}

Bonjour::~Bonjour() {
  delete wrapper_;
}

void Bonjour::Publish(const QString& domain, const QString& type, const QString& name, quint16 port) {
  NSNetService* service = [[NSNetService alloc] initWithDomain: QSTRING_TO_NSSTRING(domain)
      type: QSTRING_TO_NSSTRING(type)
      name: QSTRING_TO_NSSTRING(name)
      port: port];
  if (service) {
    [service setDelegate: wrapper_->delegate()];
    [service publish];
  }
}
