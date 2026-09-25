#include "tinysvcmdns.h"

extern "C" {
#include "mdnsd.h"
}

#include <QHostInfo>
#include <QNetworkInterface>
#include <QtEndian>

#include "core/logging.h"

void TinySVCMDNS::CreateMdnsd(uint32_t ipv4, QString ipv6) {
  // Get our hostname
  QString host = QHostInfo::localHostName();

  // Start the service
  mdnsd* mdnsd = mdnsd_start_bind(ipv4);

  // Set our hostname
  mdnsd_set_hostname(mdnsd, QString(host + ".local").toUtf8().constData(),
                     ipv4);

  // Add to the list
  responders_.append({mdnsd, ipv4});
}

TinySVCMDNS::TinySVCMDNS() { Start(); }

TinySVCMDNS::~TinySVCMDNS() { Stop(); }

void TinySVCMDNS::Start() {
  // Get all network interfaces
  QList<QNetworkInterface> network_interfaces =
      QNetworkInterface::allInterfaces();
  for (QNetworkInterface network_interface : network_interfaces) {
    // Only use up and non loopback interfaces
    if (network_interface.flags().testFlag(network_interface.IsUp) &&
        !network_interface.flags().testFlag(network_interface.IsLoopBack)) {
      uint32_t ipv4 = 0;
      QString ipv6;

      qLog(Debug) << "Interface" << network_interface.humanReadableName();

      // Now check all network addresses for this device
      QList<QNetworkAddressEntry> network_address_entries =
          network_interface.addressEntries();

      for (QNetworkAddressEntry network_address_entry :
           network_address_entries) {
        QHostAddress host_address = network_address_entry.ip();
        if (host_address.protocol() == QAbstractSocket::IPv4Protocol) {
          ipv4 = qToBigEndian(host_address.toIPv4Address());
          qLog(Debug) << "  ipv4:" << host_address.toString();
        } else if (host_address.protocol() == QAbstractSocket::IPv6Protocol) {
          ipv6 = host_address.toString();
          qLog(Debug) << "  ipv6:" << host_address.toString();
        }
      }

      // Now start the service
      CreateMdnsd(ipv4, ipv6);
    }
  }
}

void TinySVCMDNS::Stop() {
  for (const Responder& responder : responders_) {
    mdnsd_stop(responder.server);
  }
  responders_.clear();
}

void TinySVCMDNS::PublishInternal(const QString& domain, const QString& type,
                                  const QByteArray& name, quint16 port,
                                  const QList<QHostAddress>& addresses) {
  // Each responder answers from one interface's IPv4 address, so the chosen
  // IPv4 addresses pick which of them advertise.
  QList<uint32_t> chosen;
  for (const QHostAddress& address : addresses) {
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
      chosen << qToBigEndian(address.toIPv4Address());
    }
  }
  if (!addresses.isEmpty() && chosen.isEmpty()) {
    qLog(Warning) << "Not advertising the remote: it only listens on IPv6,"
                  << "and tinysvcmdns only advertises over IPv4";
    return;
  }

  // Some pointless text, so tinymDNS publishes the service correctly.
  const char* txt[] = {"cat=nyan", nullptr};

  for (const Responder& responder : responders_) {
    if (!addresses.isEmpty() && !chosen.contains(responder.ipv4)) continue;
    mdnsd_register_svc(responder.server, name.constData(),
                       QString(type + ".local").toUtf8().constData(), port,
                       nullptr, txt);
  }
}

void TinySVCMDNS::UnpublishInternal() {
  // tinysvcmdns can't withdraw a single service, so start the responders
  // afresh without it.
  Stop();
  Start();
}
