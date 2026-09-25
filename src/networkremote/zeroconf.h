#ifndef ZEROCONF_H
#define ZEROCONF_H

#include <QHostAddress>
#include <QList>
#include <QString>

class Zeroconf {
 public:
  virtual ~Zeroconf();

  // Advertises the service. With no |addresses| it goes out on every
  // interface; otherwise only on the interfaces those addresses are on, so
  // clients aren't pointed at an address nothing is listening on. Publishing
  // again replaces what was published before.
  void Publish(const QString& domain, const QString& type, const QString& name,
               quint16 port,
               const QList<QHostAddress>& addresses = QList<QHostAddress>());

  // Withdraws whatever Publish advertised.
  void Unpublish();

  static Zeroconf* GetZeroconf();

  // Truncate a QString to 63 bytes of UTF-8.
  static QByteArray TruncateName(const QString& name);

  // The index of the network interface |address| is assigned to, or -1 if
  // it isn't assigned to any.
  static int InterfaceIndexOf(const QHostAddress& address);

 protected:
  virtual void PublishInternal(const QString& domain, const QString& type,
                               const QByteArray& name, quint16 port,
                               const QList<QHostAddress>& addresses) = 0;
  virtual void UnpublishInternal() = 0;

 private:
  static Zeroconf* sInstance;
  bool published_ = false;
};

#endif  // ZEROCONF_H
