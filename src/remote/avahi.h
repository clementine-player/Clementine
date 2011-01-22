#ifndef AVAHI_H
#define AVAHI_H

#include "zeroconf.h"

#include <QObject>

class OrgFreedesktopAvahiEntryGroupInterface;
class OrgFreedesktopAvahiServerInterface;

class QDBusPendingCallWatcher;

class Avahi : public QObject, public Zeroconf {
  Q_OBJECT

public:
  Avahi();

  virtual void Publish(const QString& domain,
                       const QString& type,
                       const QString& name,
                       quint16 port);

private slots:
  void EntryGroupNewFinished(QDBusPendingCallWatcher* call);
  void AddServiceFinished(QDBusPendingCallWatcher* call);
  void CommitFinished(QDBusPendingCallWatcher* call);

private:
  OrgFreedesktopAvahiServerInterface* server_;
  OrgFreedesktopAvahiEntryGroupInterface* entry_group_;

  QString domain_;
  QString type_;
  QString name_;
  quint16 port_;
};

#endif
