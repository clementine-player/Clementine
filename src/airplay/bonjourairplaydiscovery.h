/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AIRPLAY_BONJOURAIRPLAYDISCOVERY_H_
#define AIRPLAY_BONJOURAIRPLAYDISCOVERY_H_

#include <dns_sd.h>

#include <QMap>

#include "airplaydiscovery.h"

class QSocketNotifier;

// Browses with mDNSResponder through the dns_sd API.
//
// Every DNSServiceRef has a socket of its own, which a QSocketNotifier
// watches so that its callbacks run on the main thread, from
// DNSServiceProcessResult.
class BonjourAirPlayDiscovery : public AirPlayDiscovery {
  Q_OBJECT

 public:
  explicit BonjourAirPlayDiscovery(QObject* parent = nullptr);
  ~BonjourAirPlayDiscovery();

  void Start() override;

 private:
  // A DNSServiceRef and the notifier that watches it.
  struct Operation {
    DNSServiceRef ref = nullptr;
    QSocketNotifier* notifier = nullptr;
  };

  // A resolve in progress. The callback gets a pointer to it.
  struct Resolve {
    BonjourAirPlayDiscovery* owner;
    QString service;
    // The instance name, which is the receiver's name.
    QString name;
    Operation operation;
  };

  static void DNSSD_API BrowseReply(DNSServiceRef ref, DNSServiceFlags flags,
                                    uint32_t iface, DNSServiceErrorType error,
                                    const char* name, const char* type,
                                    const char* domain, void* context);
  static void DNSSD_API ResolveReply(DNSServiceRef ref, DNSServiceFlags flags,
                                     uint32_t iface, DNSServiceErrorType error,
                                     const char* full_name, const char* host,
                                     uint16_t port, uint16_t txt_length,
                                     const unsigned char* txt, void* context);

  static QString ServiceKey(uint32_t iface, const char* name,
                            const char* domain);

  // Starts watching |op|'s socket.
  void Watch(Operation* op);
  static void Stop(Operation* op);

  void ServiceFound(uint32_t iface, const char* name, const char* type,
                    const char* domain);
  void ServiceGone(const QString& service);
  void CancelResolve(const QString& service);

  Operation browse_;
  // By service.
  QMap<QString, Resolve*> resolves_;
};

#endif  // AIRPLAY_BONJOURAIRPLAYDISCOVERY_H_
