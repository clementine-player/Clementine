#ifndef WINDOWSDNSSD_H
#define WINDOWSDNSSD_H

#include <QList>
#include <memory>

#include "zeroconf.h"

class TinySVCMDNS;

// Advertises through the mDNS responder built into Windows 10 1809 and later,
// with DnsServiceRegister. That responder owns UDP port 5353, so there
// tinysvcmdns never sees the multicast queries remotes browse with. Where the
// API is missing, or a registration fails, this falls back to tinysvcmdns.
class WindowsDnsSd : public Zeroconf {
 public:
  WindowsDnsSd();
  ~WindowsDnsSd() override;

  // Whether this version of Windows has DnsServiceRegister.
  static bool IsAvailable();

 protected:
  void PublishInternal(const QString& domain, const QString& type,
                       const QByteArray& name, quint16 port,
                       const QList<QHostAddress>& addresses) override;
  void UnpublishInternal() override;

 private:
  struct Registration;

  // Registers the service on one interface (0 for all of them), answering
  // with |addresses| if there are any. Returns false if it failed.
  bool Register(const QString& instance_name, const QString& host, quint16 port,
                int iface, const QList<QHostAddress>& addresses);
  void DeregisterAll();

  QList<Registration*> registrations_;
  // Created the first time a registration fails, and used from then on.
  std::unique_ptr<TinySVCMDNS> fallback_;
};

#endif  // WINDOWSDNSSD_H
