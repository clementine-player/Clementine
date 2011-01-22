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
    #if defined(Q_OS_DARWIN)
      instance_ = new Bonjour();
    #elif defined(HAVE_DBUS)
      instance_ = new Avahi();
    #endif
  }

  return instance_;
}
