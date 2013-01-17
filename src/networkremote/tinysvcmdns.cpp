#include "tinysvcmdns.h"

extern "C" {
#include "mdnsd.h"
}

#include <QHostInfo>
#include <QNetworkInterface>

#include "core/logging.h"

namespace {

uint32_t GetLocalIPAddress() {
  QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
  foreach (const QHostAddress& address, addresses) {
    // TODO: Add ipv6 support to tinysvcmdns.
    if (address.protocol() == QAbstractSocket::IPv4Protocol &&
        !address.isInSubnet(QHostAddress::parseSubnet("127.0.0.1/8"))) {
      return address.toIPv4Address();
    }
  }
  return 0;
}

}  // namespace

TinySVCMDNS::TinySVCMDNS()
    : mdnsd_(NULL) {
  uint32_t ip_address = GetLocalIPAddress();
  if (ip_address == 0) {
    qLog(Warning) << "Could not publish service over mDNS as there is no"
                  << "non-local IPv4 interface";
    return;
  }
  mdnsd_ = mdnsd_start();
  mdnsd_set_hostname(
      mdnsd_,
      QHostInfo::localHostName().toUtf8().constData(),
      ip_address);
}

TinySVCMDNS::~TinySVCMDNS() {
  if (mdnsd_) {
    mdnsd_stop(mdnsd_);
  }
}

void TinySVCMDNS::Publish(
    const QString& domain,
    const QString& type,
    const QString& name,
    quint16 port) {
  if (!mdnsd_) {
    return;
  }
  mdnsd_register_svc(
      mdnsd_,
      name.toUtf8().constData(),
      QString(type + ".local").toUtf8().constData(),
      port,
      NULL,
      NULL);
}
