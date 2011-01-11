#include "avahi.h"

#include <QDBusConnection>
#include <QHostInfo>
#include <QtConcurrentRun>

#include <QtDebug>

#include "dbus/avahientrygroup.h"
#include "dbus/avahiserver.h"

void Avahi::Publish(
    const QString& domain, const QString& type, const QString& name, quint16 port) {
  QtConcurrent::run(Avahi::SyncPublish, domain, type, name, port);
}

void Avahi::SyncPublish(const QString& domain, const QString& type, const QString& name, quint16 port) {
  OrgFreedesktopAvahiServerInterface server_interface(
      "org.freedesktop.Avahi",
      "/",
      QDBusConnection::systemBus());

  QDBusPendingReply<QDBusObjectPath> reply = server_interface.EntryGroupNew();
  reply.waitForFinished();

  OrgFreedesktopAvahiEntryGroupInterface entry_group_interface(
      "org.freedesktop.Avahi",
      reply.value().path(),
      QDBusConnection::systemBus());

  QDBusPendingReply<> add_reply = entry_group_interface.AddService(
      -1,                         // Interface (Unspecified, ie. all interfaces)
      -1,                         // Protocol (Unspecified, ie. IPv4 & IPv6)
      0,                          // Flags
      name,                       // Service name
      type,                       // Service type
      domain,                     // Domain, ie. local
      QString::null,              // Hostname (Avahi fills it if it's null)
      port,                       // Port
      QList<QByteArray>());       // TXT record
  add_reply.waitForFinished();

  QDBusPendingReply<> commit_reply = entry_group_interface.Commit();
  commit_reply.waitForFinished();
  if (commit_reply.isValid()) {
    qDebug() << "Remote interface published on Avahi";
  }
}
