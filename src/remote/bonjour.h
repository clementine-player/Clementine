#ifndef BONJOUR_H
#define BONJOUR_H

#include "zeroconf.h"

class BonjourWrapper;

class Bonjour : public Zeroconf {
 public:
  Bonjour();
  virtual ~Bonjour();
  void Publish(const QString& domain, const QString& type, const QString& name, quint16 port);

 private:
  BonjourWrapper* wrapper_;
};

#endif
