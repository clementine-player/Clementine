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
  void PublishInternal(const QString& domain, const QString& type,
                       const QByteArray& name, quint16 port,
                       const QList<QHostAddress>& addresses) override;
  void UnpublishInternal() override;

 private:
  // One per network interface, answering from its IPv4 address.
  struct Responder {
    mdnsd* server;
    uint32_t ipv4;  // In network byte order, as tinysvcmdns takes it.
  };

  void Start();
  void Stop();
  void CreateMdnsd(uint32_t ipv4, QString ipv6);

  QList<Responder> responders_;
};

#endif  // TINYSVCMDNS_H
