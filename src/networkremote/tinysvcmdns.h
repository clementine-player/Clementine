#ifndef TINYSVCMDNS_H
#define TINYSVCMDNS_H

#include "zeroconf.h"

struct mdnsd;

class TinySVCMDNS : public Zeroconf {
 public:
  TinySVCMDNS();
  virtual ~TinySVCMDNS();

  virtual void Publish(
      const QString& domain,
      const QString& type,
      const QString& name,
      quint16 port);

 private:
  mdnsd* mdnsd_;
};

#endif  // TINYSVCMDNS_H
