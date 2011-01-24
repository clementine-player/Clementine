#include "avahi.h"

#include <QDBusConnection>
#include <QHostInfo>

#include <QtDebug>

#include "dbus/avahientrygroup.h"
#include "dbus/avahiserver.h"

Avahi::Avahi()
  : server_(NULL),
    entry_group_(NULL)
{
}

void Avahi::Publish(const QString& domain,
                    const QString& type,
                    const QString& name,
                    quint16 port) {
  if (server_) {
    // Already published
    return;
  }

  domain_ = domain;
  type_ = type;
  name_ = name;
  port_ = port;

  server_ = new OrgFreedesktopAvahiServerInterface(
      "org.freedesktop.Avahi",
      "/",
      QDBusConnection::systemBus(),
      this);

  QDBusPendingReply<QDBusObjectPath> reply = server_->EntryGroupNew();
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply);
  connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
          SLOT(EntryGroupNewFinished(QDBusPendingCallWatcher*)));
}

void Avahi::EntryGroupNewFinished(QDBusPendingCallWatcher* call) {
  call->deleteLater();
  QDBusPendingReply<QDBusObjectPath> reply = *call;

  if (reply.isError()) {
    qWarning() << "Failed to create new Avahi entry group:" << call->error().message();
    return;
  }

  entry_group_ = new OrgFreedesktopAvahiEntryGroupInterface(
      "org.freedesktop.Avahi",
      reply.value().path(),
      QDBusConnection::systemBus(),
      this);

  QDBusPendingReply<> add_reply = entry_group_->AddService(
      -1,                         // Interface (Unspecified, ie. all interfaces)
      -1,                         // Protocol (Unspecified, ie. IPv4 & IPv6)
      0,                          // Flags
      name_,                      // Service name
      type_,                      // Service type
      domain_,                    // Domain, ie. local
      QString::null,              // Hostname (Avahi fills it if it's null)
      port_,                      // Port
      QList<QByteArray>());       // TXT record
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(add_reply);
  connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
          SLOT(AddServiceFinished(QDBusPendingCallWatcher*)));
}

void Avahi::AddServiceFinished(QDBusPendingCallWatcher* call) {
  call->deleteLater();

  if (call->isError()) {
    qWarning() << "Failed to add Avahi service:" << call->error().message();
    return;
  }

  QDBusPendingReply<> commit_reply = entry_group_->Commit();
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(commit_reply);
  connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
          SLOT(CommitFinished(QDBusPendingCallWatcher*)));
}

void Avahi::CommitFinished(QDBusPendingCallWatcher* call) {
  call->deleteLater();

  if (call->isError()) {
    qDebug() << "Remote interface published on Avahi";
  } else {
    qWarning() << "Failed to commit Avahi changes:" << call->error().message();
  }
}
