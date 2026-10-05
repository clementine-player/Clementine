#ifndef WINDOWSDNSSD_H
#define WINDOWSDNSSD_H

#include <QList>

#include "zeroconf.h"

// Advertises through the mDNS responder built into Windows 10 1809 and later,
// with DnsServiceRegister. That responder owns UDP port 5353, so it has to be
// the one answering the queries remotes browse with.
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
};

#endif  // WINDOWSDNSSD_H
