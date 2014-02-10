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
                       const QString& name, quint16 port) {
  QByteArray truncated_name = TruncateName(name);
  PublishInternal(domain, type, truncated_name, port);
}
