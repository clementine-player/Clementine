#ifndef TINYSVCMDNS_H
#define TINYSVCMDNS_H

#include <QList>

#include "zeroconf.h"

struct mdnsd;

class TinySVCMDNS : public Zeroconf {
 public:
  TinySVCMDNS();
  virtual ~TinySVCMDNS();

 protected:
  virtual void PublishInternal(const QString& domain, const QString& type,
                               const QByteArray& name, quint16 port);

 private:
  void CreateMdnsd(uint32_t ipv4, QString ipv6);
  QList<mdnsd*> mdnsd_;
};

#endif  // TINYSVCMDNS_H
