#include "tinysvcmdns.h"

extern "C" {
#include "mdns.h"
#include "mdnsd.h"
}

#include <QHostInfo>
#include <QNetworkInterface>
#include <QtEndian>

#include "core/logging.h"

TinySVCMDNS::CreateMdnsd(uint32_t ipv4, QString ipv6) {
  // Get our hostname
  QString host = QHostInfo::localHostName();

  // Start the service
  mdnsd* mdnsd = mdnsd_start_bind(ipv4);

  // Set our hostname
  mdnsd_set_hostname(
    mdnsd,
    QString(host + ".local").toUtf8().constData(),
    ipv4);

  // Add entry for ipv4
  struct rr_entry *a2_e = NULL;
  a2_e = rr_create_a(create_nlabel(hostname), ipv4);
  mdnsd_add_rr(mdnsd, a2_e);

  // Add entry for ipv6
  struct rr_entry *aaaa_e = NULL;

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_flags = AI_NUMERICHOST;
  struct addrinfo* results;
  getaddrinfo(
    ipv6.toStdString().c_str(),
    NULL,
    &hints,
    &results);
  struct sockaddr_in6* addr = (struct sockaddr_in6*)results->ai_addr;
  struct in6_addr v6addr = addr->sin6_addr;
  freeaddrinfo(results);

  aaaa_e = rr_create_aaaa(create_nlabel(hostname), &v6addr);

  mdnsd_add_rr(mdnsd, aaaa_e);

  // Add to the list
  mdnsd_.append(mdnsd);
}

TinySVCMDNS::TinySVCMDNS() {  
  // Get all network interfaces
  QList<QNetworkInterface> network_interfaces = QNetworkInterface::allInterfaces();
  foreach (QNetworkInterface network_interface, network_interfaces) {
    // Only use up and non loopback interfaces
    if (network_interface.flags().testFlag(network_interface.IsUp) 
     && !network_interface.flags().testFlag(network_interface.IsLoopBack))
    {
      uint32_t ipv4 = 0;
      QString ipv6;
      
      // Now check all network addresses for this device
      QList<QNetworkAddressEntry> network_address_entries = network_interface.addressEntries();

      foreach (QNetworkAddressEntry network_address_entry, network_address_entries) {
        QHostAddress host_address = network_address_entry.ip();
        if (host_address.protocol() == QAbstractSocket::IPv4Protocol) {
          ipv4 = qToBigEndian(host_address.toIPv4Address());
        } else if (host_address.protocol() == QAbstractSocket::IPv6Protocol) {
          ipv6 = host_address.toString();
        }
      }
      
      // Now start the service
      CreateMdnsd(ipv4, ipv6);
    }
  }
}

TinySVCMDNS::~TinySVCMDNS() {
  foreach(mdnsd* mdnsd, mdnsd_) {
    mdnsd_stop(mdnsd);
  }
}

void TinySVCMDNS::PublishInternal(
    const QString& domain,
    const QString& type,
    const QByteArray& name,
    quint16 port) {

  // Some pointless text, so tinymDNS publishes the service correctly.
  const char* txt[] = {
    "cat=nyan",
    NULL
  };
  
  foreach(mdnsd* mdnsd, mdnsd_) {
    mdnsd_register_svc(
        mdnsd,
        name.constData(),
        QString(type + ".local").toUtf8().constData(),
        port,
        NULL,
        txt);
  }
}
