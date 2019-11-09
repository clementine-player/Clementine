#include "avahi.h"

#include <QDBusConnection>

#include "core/closure.h"
#include "core/logging.h"
#include "dbus/avahientrygroup.h"
#include "dbus/avahiserver.h"

namespace {

void AddService(const QString domain, const QString type, const QByteArray name,
                quint16 port, QDBusPendingReply<QDBusObjectPath> path_reply);
void Commit(OrgFreedesktopAvahiEntryGroupInterface* interface);
void LogCommit(QDBusPendingReply<> reply);

}  // namespace

void Avahi::PublishInternal(const QString& domain, const QString& type,
                            const QByteArray& name, quint16 port) {
  OrgFreedesktopAvahiServerInterface server_interface(
      "org.freedesktop.Avahi", "/", QDBusConnection::systemBus());
  QDBusPendingReply<QDBusObjectPath> reply = server_interface.EntryGroupNew();
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply);
  NewClosure(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &AddService,
             domain, type, name, port, reply);
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), watcher,
                   SLOT(deleteLater()));
}

namespace {

void AddService(const QString domain, const QString type, const QByteArray name,
                quint16 port, QDBusPendingReply<QDBusObjectPath> path_reply) {
  if (path_reply.isError()) {
    qLog(Warning) << "Failed to create Avahi entry group:"
                  << path_reply.error();
    qLog(Info) << "This might be because 'disable-user-service-publishing'"
               << "is set to 'yes' in avahi-daemon.conf";
    return;
  }
  qLog(Debug) << path_reply.error();
  OrgFreedesktopAvahiEntryGroupInterface* entry_group_interface =
      new OrgFreedesktopAvahiEntryGroupInterface("org.freedesktop.Avahi",
                                                 path_reply.value().path(),
                                                 QDBusConnection::systemBus());
  QDBusPendingReply<> reply = entry_group_interface->AddService(
      -1,  // Interface (all)
      -1,  // Protocol (v4 & v6)
      0,   // Flags
      // Service name, eg. Clementine
      QString::fromUtf8(name.constData(), name.size()),
      type,                  // Service type, eg. _clementine._tcp
      domain,                // Domain, eg. local
      QString(),             // Hostname (filled in by Avahi)
      port,                  // Port our service is running on
      QList<QByteArray>());  // TXT record
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply);
  NewClosure(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &Commit,
             entry_group_interface);

  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), watcher,
                   SLOT(deleteLater()));
}

void Commit(OrgFreedesktopAvahiEntryGroupInterface* interface) {
  QDBusPendingReply<> reply = interface->Commit();
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply);
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), watcher,
                   SLOT(deleteLater()));
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                   interface, SLOT(deleteLater()));
  NewClosure(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &LogCommit,
             reply);
}

void LogCommit(QDBusPendingReply<> reply) {
  qLog(Debug) << "Remote interface published on Avahi:" << reply.error();
}

}  // namespace
