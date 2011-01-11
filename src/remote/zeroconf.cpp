#include "zeroconf.h"

#include "config.h"

#ifdef Q_OS_DARWIN
#include "bonjour.h"
#endif

#ifdef HAVE_DBUS
#include "avahi.h"
#endif

Zeroconf* Zeroconf::instance_ = NULL;

Zeroconf* Zeroconf::GetZeroconf() {
  if (!instance_) {
  #ifdef Q_OS_DARWIN
    return new Bonjour();
  #endif

  #ifdef HAVE_DBUS
    return new Avahi();
  #endif
  }

  return instance_;
}
