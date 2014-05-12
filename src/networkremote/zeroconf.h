#ifndef ZEROCONF_H
#define ZEROCONF_H

#include <QString>

class Zeroconf {
 public:
  virtual ~Zeroconf();

  void Publish(const QString& domain, const QString& type, const QString& name,
               quint16 port);

  static Zeroconf* GetZeroconf();

  // Truncate a QString to 63 bytes of UTF-8.
  static QByteArray TruncateName(const QString& name);

 protected:
  virtual void PublishInternal(const QString& domain, const QString& type,
                               const QByteArray& name, quint16 port) = 0;

 private:
  static Zeroconf* sInstance;
};

#endif  // ZEROCONF_H
