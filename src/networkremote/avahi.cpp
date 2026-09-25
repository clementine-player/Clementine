#include "avahi.h"

#include <QDBusConnection>

#include "core/closure.h"
#include "core/logging.h"
#include "dbus/avahientrygroup.h"
#include "dbus/avahiserver.h"

namespace {

// From avahi-common/address.h.
const int kAvahiIfUnspec = -1;
const int kAvahiProtoUnspec = -1;
const int kAvahiProtoInet = 0;
const int kAvahiProtoInet6 = 1;

void LogIfFailed(QDBusPendingReply<> reply, const QString& what) {
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply);
  NewClosure(watcher, &QDBusPendingCallWatcher::finished, watcher,
             [reply, what](QDBusPendingCallWatcher*) {
               if (reply.isError()) {
                 qLog(Warning)
                     << "Avahi failed to" << what << ":" << reply.error();
               } else {
                 qLog(Debug) << "Avahi did:" << what;
               }
             });
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), watcher,
                   SLOT(deleteLater()));
}

}  // namespace

Avahi::Avahi() : entry_group_(nullptr), generation_(0) {}

void Avahi::PublishInternal(const QString& domain, const QString& type,
                            const QByteArray& name, quint16 port,
                            const QList<QHostAddress>& addresses) {
  Targets targets;
  if (addresses.isEmpty()) {
    targets << qMakePair(kAvahiIfUnspec, kAvahiProtoUnspec);
  } else {
    // Avahi advertises per interface and protocol, answering with that
    // interface's addresses of that family.
    for (const QHostAddress& address : addresses) {
      const int iface = InterfaceIndexOf(address);
      if (iface < 0) {
        qLog(Warning) << "Not advertising the remote on" << address.toString()
                      << "- it isn't on any network interface";
        continue;
      }
      const QPair<int, int> target(
          iface, address.protocol() == QAbstractSocket::IPv6Protocol
                     ? kAvahiProtoInet6
                     : kAvahiProtoInet);
      if (!targets.contains(target)) targets << target;
    }
    if (targets.isEmpty()) return;
  }

  const int generation = generation_;
  OrgFreedesktopAvahiServerInterface server_interface(
      "org.freedesktop.Avahi", "/", QDBusConnection::systemBus());
  QDBusPendingReply<QDBusObjectPath> reply = server_interface.EntryGroupNew();
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply);
  NewClosure(watcher, &QDBusPendingCallWatcher::finished, watcher,
             [this, generation, domain, type, name, port, targets,
              reply](QDBusPendingCallWatcher*) {
               AddServices(generation, domain, type, name, port, targets,
                           reply);
             });
  QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), watcher,
                   SLOT(deleteLater()));
}

void Avahi::AddServices(int generation, const QString& domain,
                        const QString& type, const QByteArray& name,
                        quint16 port, const Targets& targets,
                        QDBusPendingReply<QDBusObjectPath> path_reply) {
  if (path_reply.isError()) {
    qLog(Warning) << "Failed to create Avahi entry group:"
                  << path_reply.error();
    qLog(Info) << "This might be because 'disable-user-service-publishing'"
               << "is set to 'yes' in avahi-daemon.conf";
    return;
  }

  OrgFreedesktopAvahiEntryGroupInterface* group =
      new OrgFreedesktopAvahiEntryGroupInterface("org.freedesktop.Avahi",
                                                 path_reply.value().path(),
                                                 QDBusConnection::systemBus());
  if (generation != generation_) {
    // Withdrawn, or published again, while Avahi was creating the group.
    LogIfFailed(group->Free(), "free an out of date entry group");
    group->deleteLater();
    return;
  }
  entry_group_ = group;

  // D-Bus delivers calls from one connection in order, so Avahi has every
  // service before the commit.
  for (const QPair<int, int>& target : targets) {
    LogIfFailed(
        group->AddService(target.first,   // Interface
                          target.second,  // Protocol
                          0,              // Flags
                          // Service name, eg. Clementine
                          QString::fromUtf8(name.constData(), name.size()),
                          type,       // Service type, eg. _clementine._tcp
                          domain,     // Domain, eg. local
                          QString(),  // Hostname (filled in by Avahi)
                          port,       // Port our service is running on
                          QList<QByteArray>()),  // TXT record
        QString("add the service on interface %1, protocol %2")
            .arg(target.first)
            .arg(target.second));
  }
  LogIfFailed(group->Commit(), "publish the remote");
}

void Avahi::UnpublishInternal() {
  ++generation_;
  if (!entry_group_) return;

  LogIfFailed(entry_group_->Free(), "withdraw the remote");
  entry_group_->deleteLater();
  entry_group_ = nullptr;
}
