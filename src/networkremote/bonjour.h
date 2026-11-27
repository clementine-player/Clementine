#ifndef BONJOUR_H
#define BONJOUR_H

#include <QList>

#include "zeroconf.h"

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
  // Active DNSServiceRegister handles, one per interface or
  // kDNSServiceInterfaceIndexAny.
  QList<_DNSServiceRef_t*> registrations_;
};

#endif  // BONJOUR_H
