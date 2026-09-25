#ifndef BONJOUR_H
#define BONJOUR_H

#include <QList>

#include "zeroconf.h"

#ifdef __OBJC__
@class NetServicePublicationDelegate;
@class NSNetService;
#else
class NetServicePublicationDelegate;
class NSNetService;
#endif  // __OBJC__

// dns_sd.h's DNSServiceRef points to one of these.
struct _DNSServiceRef_t;

class Bonjour : public Zeroconf {
 public:
  Bonjour();
  virtual ~Bonjour();

 protected:
  void PublishInternal(const QString& domain, const QString& type,
                       const QByteArray& name, quint16 port,
                       const QList<QHostAddress>& addresses) override;
  void UnpublishInternal() override;

 private:
  NetServicePublicationDelegate* delegate_;
  // Publishing on every interface.
  NSNetService* service_;
  // Publishing on some interfaces. NSNetService can't be narrowed to them,
  // so this goes through DNSServiceRegister, once per interface.
  QList<_DNSServiceRef_t*> registrations_;
};

#endif  // BONJOUR_H
