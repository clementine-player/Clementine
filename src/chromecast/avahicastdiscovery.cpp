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

#include "avahicastdiscovery.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>

#include "core/closure.h"
#include "core/logging.h"

namespace {

const char* kAvahiService = "org.freedesktop.Avahi";
const char* kServerInterface = "org.freedesktop.Avahi.Server";
const char* kServer2Interface = "org.freedesktop.Avahi.Server2";
const char* kBrowserInterface = "org.freedesktop.Avahi.ServiceBrowser";

// From avahi-common/address.h.
const int kAvahiIfUnspec = -1;
const int kAvahiProtoUnspec = -1;
const int kAvahiProtoInet = 0;

// Calls |method| on Avahi and passes the reply, or the error, to |callback|
// unless |receiver| has been destroyed by then.
template <typename Callback>
void CallAvahi(QObject* receiver, const QString& path, const char* interface,
               const char* method, const QList<QVariant>& arguments,
               Callback callback) {
  QDBusMessage call =
      QDBusMessage::createMethodCall(kAvahiService, path, interface, method);
  call.setArguments(arguments);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(
      QDBusConnection::systemBus().asyncCall(call), receiver);
  NewClosure(watcher, &QDBusPendingCallWatcher::finished, receiver,
             [watcher, callback](QDBusPendingCallWatcher*) {
               callback(watcher->reply());
             });
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), watcher,
                   SLOT(deleteLater()));
}

}  // namespace

AvahiCastDiscovery::AvahiCastDiscovery(QObject* parent)
    : CastDiscovery(parent) {}

AvahiCastDiscovery::~AvahiCastDiscovery() {
  // Avahi frees the browser when our connection closes too, but the system
  // bus connection lasts as long as Clementine does.
  if (!browser_path_.isEmpty()) {
    QDBusConnection::systemBus().call(
        QDBusMessage::createMethodCall(kAvahiService, browser_path_,
                                       kBrowserInterface, "Free"),
        QDBus::NoBlock);
  }
}

void AvahiCastDiscovery::Start() {
  if (!browser_path_.isEmpty()) return;

  CallAvahi(
      this, "/", kServer2Interface, "ServiceBrowserPrepare",
      {kAvahiIfUnspec, kAvahiProtoUnspec, QString(kServiceType), QString(), 0u},
      [this](const QDBusMessage& reply) { BrowserPrepared(reply); });
}

void AvahiCastDiscovery::BrowserPrepared(const QDBusMessage& reply) {
  if (reply.type() == QDBusMessage::ErrorMessage) {
    qLog(Warning) << "Couldn't look for Cast devices with Avahi (it needs"
                  << "Avahi 0.8 or later):" << reply.errorMessage();
    return;
  }
  browser_path_ = reply.arguments().value(0).value<QDBusObjectPath>().path();

  QDBusConnection bus = QDBusConnection::systemBus();
  bus.connect(kAvahiService, browser_path_, kBrowserInterface, "ItemNew", this,
              SLOT(ItemNew(int, int, QString, QString, QString, uint)));
  bus.connect(kAvahiService, browser_path_, kBrowserInterface, "ItemRemove",
              this,
              SLOT(ItemRemove(int, int, QString, QString, QString, uint)));
  bus.connect(kAvahiService, browser_path_, kBrowserInterface, "Failure", this,
              SLOT(Failure(QString)));

  CallAvahi(this, browser_path_, kBrowserInterface, "Start", {},
            [](const QDBusMessage& reply) {
              if (reply.type() == QDBusMessage::ErrorMessage) {
                qLog(Warning) << "Couldn't start looking for Cast devices:"
                              << reply.errorMessage();
              } else {
                qLog(Debug) << "Looking for Cast devices";
              }
            });
}

QString AvahiCastDiscovery::ServiceKey(int iface, int protocol,
                                       const QString& name,
                                       const QString& domain) {
  return QString("%1/%2/%3.%4").arg(iface).arg(protocol).arg(name, domain);
}

void AvahiCastDiscovery::ItemNew(int iface, int protocol, const QString& name,
                                 const QString& type, const QString& domain,
                                 uint) {
  const QString service = ServiceKey(iface, protocol, name, domain);
  services_ << service;

  // Ask for an IPv4 address even when the service was seen over IPv6. Cast
  // devices all have one, and it's what the remote's network policy expects.
  CallAvahi(
      this, "/", kServerInterface, "ResolveService",
      {iface, protocol, name, type, domain, kAvahiProtoInet, 0u},
      [this, service](const QDBusMessage& reply) { Resolved(service, reply); });
}

void AvahiCastDiscovery::Resolved(const QString& service,
                                  const QDBusMessage& reply) {
  if (!services_.contains(service)) return;

  if (reply.type() == QDBusMessage::ErrorMessage) {
    qLog(Debug) << "Couldn't resolve Cast service" << service << ":"
                << reply.errorMessage();
    return;
  }

  // (interface, protocol, name, type, domain, host, aprotocol, address, port,
  // txt, flags)
  const QList<QVariant> args = reply.arguments();
  if (args.size() < 10) {
    qLog(Warning) << "Unexpected reply from Avahi resolving" << service;
    return;
  }

  CastDevice device;
  if (!device.ParseTxt(qdbus_cast<QList<QByteArray>>(args[9]))) {
    qLog(Debug) << "Cast service" << service << "has no id";
    return;
  }
  device.address = QHostAddress(args[7].toString());
  device.port = args[8].value<quint16>();

  ServiceResolved(service, device);
}

void AvahiCastDiscovery::ItemRemove(int iface, int protocol,
                                    const QString& name, const QString&,
                                    const QString& domain, uint) {
  const QString service = ServiceKey(iface, protocol, name, domain);
  services_.remove(service);
  ServiceRemoved(service);
}

void AvahiCastDiscovery::Failure(const QString& error) {
  qLog(Warning) << "Avahi stopped looking for Cast devices:" << error;
}
