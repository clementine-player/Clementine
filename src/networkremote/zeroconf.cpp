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
    #ifdef Q_OS_WIN32
      sInstance = new TinySVCMDNS;
    #endif
  }

  return sInstance;
}
