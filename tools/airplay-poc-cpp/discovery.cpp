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

#include "discovery.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QTimer>

#include "logging.h"

namespace airplay {

namespace {

const char kAvahiService[] = "org.freedesktop.Avahi";
const char kServerInterface[] = "org.freedesktop.Avahi.Server";
const char kServer2Interface[] = "org.freedesktop.Avahi.Server2";
const char kBrowserInterface[] = "org.freedesktop.Avahi.ServiceBrowser";
const char kRaopType[] = "_raop._tcp";
const char kAirPlayType[] = "_airplay._tcp";

// From avahi-common/address.h.
const int kAvahiIfUnspec = -1;
const int kAvahiProtoUnspec = -1;

template <typename Callback>
void CallAvahi(QObject* receiver, const QString& path, const char* interface,
               const char* method, const QList<QVariant>& arguments,
               Callback callback) {
  QDBusMessage call =
      QDBusMessage::createMethodCall(kAvahiService, path, interface, method);
  call.setArguments(arguments);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(
      QDBusConnection::systemBus().asyncCall(call), receiver);
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, receiver,
                   [watcher, callback](QDBusPendingCallWatcher*) {
                     callback(watcher->reply());
                     watcher->deleteLater();
                   });
}

}  // namespace

QString VersionName(Version version) {
  return version == Version::kAirPlay2 ? "AirPlay2" : "AirPlay1";
}

quint64 Device::features() const {
  const QString f = txt.value("ft", txt.value("features"));
  const QStringList parts = f.split(',');
  const quint64 low = parts.value(0).toULongLong(nullptr, 16);
  const quint64 high = parts.value(1).toULongLong(nullptr, 16);
  return (high << 32) | low;
}

Version Device::version() const {
  const quint64 f = features();
  if (f & (quint64(1) << 38) || f & (quint64(1) << 48)) {
    return Version::kAirPlay2;
  }
  return Version::kAirPlay1;
}

bool Device::password_required() const {
  if (txt.value("pw").compare("true", Qt::CaseInsensitive) == 0) return true;
  for (const char* key : {"flags", "sf"}) {
    if (txt.value(key).toUInt(nullptr, 16) & kStatusPasswordRequired) {
      return true;
    }
  }
  return false;
}

QHostAddress Device::address() const {
  for (const QHostAddress& a : addresses) {
    if (a.protocol() == QAbstractSocket::IPv4Protocol) return a;
  }
  return addresses.value(0);
}

Discovery::Discovery(QObject* parent) : QObject(parent) {}

void Discovery::Browse(int msec) {
  for (const char* type : {kRaopType, kAirPlayType}) {
    const QString t(type);
    CallAvahi(
        this, "/", kServer2Interface, "ServiceBrowserPrepare",
        {kAvahiIfUnspec, kAvahiProtoUnspec, t, QString(), 0u},
        [this, t](const QDBusMessage& reply) { BrowserPrepared(t, reply); });
  }
  QTimer::singleShot(msec, this, SLOT(Done()));
}

void Discovery::BrowserPrepared(const QString& type,
                                const QDBusMessage& reply) {
  if (reply.type() == QDBusMessage::ErrorMessage) {
    qCWarning(airplay_log) << "Couldn't look for" << type
                           << "with Avahi (it needs Avahi 0.8 or later):"
                           << reply.errorMessage();
    return;
  }
  const QString path =
      reply.arguments().value(0).value<QDBusObjectPath>().path();
  browsers_ << path;
  QDBusConnection::systemBus().connect(
      kAvahiService, path, kBrowserInterface, "ItemNew", this,
      SLOT(ItemNew(int, int, QString, QString, QString, uint)));
  CallAvahi(this, path, kBrowserInterface, "Start", {},
            [type](const QDBusMessage& reply) {
              if (reply.type() == QDBusMessage::ErrorMessage) {
                qCWarning(airplay_log) << "Couldn't start looking for" << type
                                       << reply.errorMessage();
              }
            });
}

void Discovery::ItemNew(int iface, int protocol, const QString& name,
                        const QString& type, const QString& domain, uint) {
  // The address in the protocol the service was seen over.
  CallAvahi(this, "/", kServerInterface, "ResolveService",
            {iface, protocol, name, type, domain, protocol, 0u},
            [this](const QDBusMessage& reply) { Resolved(reply); });
}

void Discovery::Resolved(const QDBusMessage& reply) {
  if (reply.type() == QDBusMessage::ErrorMessage) {
    qCDebug(airplay_log) << "Couldn't resolve a service:"
                         << reply.errorMessage();
    return;
  }
  // (interface, protocol, name, type, domain, host, aprotocol, address, port,
  // txt, flags)
  const QList<QVariant> args = reply.arguments();
  if (args.size() < 10) return;

  QString name = args[2].toString();
  const bool raop = args[3].toString() == kRaopType;
  // RAOP names are "<MAC>@<name>".
  if (raop && name.contains('@')) name = name.section('@', 1);

  Device* device = &devices_[name];
  device->name = name;
  const QHostAddress address(args[7].toString());
  if (!device->addresses.contains(address)) device->addresses << address;
  const quint16 port = args[8].value<quint16>();
  if (raop) {
    device->raop_port = port;
  } else {
    device->airplay_port = port;
  }
  for (const QByteArray& entry : qdbus_cast<QList<QByteArray>>(args[9])) {
    const int eq = entry.indexOf('=');
    const QString key = QString::fromUtf8(eq < 0 ? entry : entry.left(eq));
    if (!device->txt.contains(key)) {
      device->txt[key] =
          eq < 0 ? QString() : QString::fromUtf8(entry.mid(eq + 1));
    }
  }
}

void Discovery::Done() {
  for (const QString& path : browsers_) {
    QDBusConnection::systemBus().call(
        QDBusMessage::createMethodCall(kAvahiService, path, kBrowserInterface,
                                       "Free"),
        QDBus::NoBlock);
  }
  browsers_.clear();
  emit Finished(devices_.values());
}

}  // namespace airplay
