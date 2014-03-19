#ifndef AVAHI_H
#define AVAHI_H

#include "zeroconf.h"

class Avahi : public Zeroconf {
 protected:
  virtual void PublishInternal(const QString& domain, const QString& type,
                               const QByteArray& name, quint16 port);
};

#endif  // AVAHI_H
