#include "zeroconf.h"

#include "config.h"

#ifdef HAVE_DBUS
#include "avahi.h"
#endif

Zeroconf* Zeroconf::sInstance = NULL;

Zeroconf::~Zeroconf() {

}

Zeroconf* Zeroconf::GetZeroconf() {
  if (!sInstance) {
    #ifdef HAVE_DBUS
      sInstance = new Avahi;
    #endif  // HAVE_DBUS
  }

  return sInstance;
}
