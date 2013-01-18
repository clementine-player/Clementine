#ifndef ZEROCONF_H
#define ZEROCONF_H

#include <QString>

class Zeroconf {
 public:
  virtual ~Zeroconf();

  virtual void Publish(
      const QString& domain,
      const QString& type,
      const QString& name,
      quint16 port) = 0;

  static Zeroconf* GetZeroconf();

 private:
  static Zeroconf* sInstance;
};

#endif  // ZEROCONF_H
