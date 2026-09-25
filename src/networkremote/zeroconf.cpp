#include "zeroconf.h"

#include "config.h"

#ifdef HAVE_DBUS
#include "avahi.h"
#endif

#ifdef Q_OS_DARWIN
#include "bonjour.h"
#endif

#ifdef Q_OS_WIN32
#include "tinysvcmdns.h"
#endif

#include <QNetworkInterface>
#include <QTextCodec>

Zeroconf* Zeroconf::sInstance = nullptr;

Zeroconf::~Zeroconf() {}

Zeroconf* Zeroconf::GetZeroconf() {
  if (!sInstance) {
#ifdef HAVE_DBUS
    sInstance = new Avahi;
#endif  // HAVE_DBUS
#ifdef Q_OS_DARWIN
    sInstance = new Bonjour;
#endif
#ifdef Q_OS_WIN32
    sInstance = new TinySVCMDNS;
#endif
  }

  return sInstance;
}

QByteArray Zeroconf::TruncateName(const QString& name) {
  QTextCodec* codec = QTextCodec::codecForName("UTF-8");
  QByteArray truncated_utf8;
  for (QChar c : name) {
    QByteArray rendered = codec->fromUnicode(&c, 1, nullptr);
    if (truncated_utf8.size() + rendered.size() >= 63) {
      break;
    }
    truncated_utf8 += rendered;
  }
  // NULL-terminate the string.
  truncated_utf8.append('\0');
  return truncated_utf8;
}

void Zeroconf::Publish(const QString& domain, const QString& type,
                       const QString& name, quint16 port,
                       const QList<QHostAddress>& addresses) {
  Unpublish();
  QByteArray truncated_name = TruncateName(name);
  PublishInternal(domain, type, truncated_name, port, addresses);
  published_ = true;
}

void Zeroconf::Unpublish() {
  if (!published_) return;
  UnpublishInternal();
  published_ = false;
}

int Zeroconf::InterfaceIndexOf(const QHostAddress& address) {
  for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
    for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
      if (entry.ip() == address) return iface.index();
    }
  }
  return -1;
}
