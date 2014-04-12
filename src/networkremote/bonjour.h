#ifndef BONJOUR_H
#define BONJOUR_H

#include "zeroconf.h"

#ifdef __OBJC__
@class NetServicePublicationDelegate;
#else
class NetServicePublicationDelegate;
#endif  // __OBJC__

class Bonjour : public Zeroconf {
 public:
  Bonjour();
  virtual ~Bonjour();

 protected:
  virtual void PublishInternal(const QString& domain, const QString& type,
                               const QByteArray& name, quint16 port);

 private:
  NetServicePublicationDelegate* delegate_;
};

#endif  // BONJOUR_H
