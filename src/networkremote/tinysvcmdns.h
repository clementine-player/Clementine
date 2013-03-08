#ifndef TINYSVCMDNS_H
#define TINYSVCMDNS_H

#include "zeroconf.h"
#include <QList>

struct mdnsd;

class TinySVCMDNS : public Zeroconf {
 public:
  TinySVCMDNS();
  virtual ~TinySVCMDNS();

 protected:
  virtual void PublishInternal(
      const QString& domain,
      const QString& type,
      const QByteArray& name,
      quint16 port);

 private:
  QList<mdnsd*> mdnsd_;
};

#endif  // TINYSVCMDNS_H
