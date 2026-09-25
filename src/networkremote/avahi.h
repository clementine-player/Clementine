#ifndef AVAHI_H
#define AVAHI_H

#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QList>
#include <QPair>

#include "zeroconf.h"

class OrgFreedesktopAvahiEntryGroupInterface;

class Avahi : public Zeroconf {
 public:
  Avahi();

 protected:
  void PublishInternal(const QString& domain, const QString& type,
                       const QByteArray& name, quint16 port,
                       const QList<QHostAddress>& addresses) override;
  void UnpublishInternal() override;

 private:
  // Where to advertise, as (interface index, protocol) pairs in Avahi's
  // numbering.
  typedef QList<QPair<int, int>> Targets;

  void AddServices(int generation, const QString& domain, const QString& type,
                   const QByteArray& name, quint16 port, const Targets& targets,
                   QDBusPendingReply<QDBusObjectPath> path_reply);

  // What's published, once Avahi has created it.
  OrgFreedesktopAvahiEntryGroupInterface* entry_group_;
  // Bumped whenever what's published is withdrawn, so that a publish still
  // waiting on Avahi knows it's out of date and throws its entry group away.
  int generation_;
};

#endif  // AVAHI_H
