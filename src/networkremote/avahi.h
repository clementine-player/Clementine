#ifndef AVAHI_H
#define AVAHI_H

#include "zeroconf.h"

class Avahi : public Zeroconf {
 public:
  virtual void Publish(
      const QString& domain,
      const QString& type,
      const QString& name,
      quint16 port);
};

#endif  // AVAHI_H
