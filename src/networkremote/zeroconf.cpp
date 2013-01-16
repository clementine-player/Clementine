#include "zeroconf.h"

#include "config.h"

#ifdef HAVE_DBUS
#include "avahi.h"
#endif

#ifdef Q_OS_DARWIN
#include "bonjour.h"
#endif

Zeroconf* Zeroconf::sInstance = NULL;

Zeroconf::~Zeroconf() {

}

Zeroconf* Zeroconf::GetZeroconf() {
  if (!sInstance) {
    #ifdef HAVE_DBUS
      sInstance = new Avahi;
    #endif  // HAVE_DBUS
    #ifdef Q_OS_DARWIN
      sInstance = new Bonjour;
    #endif
  }

  return sInstance;
}
