#include "zeroconf.h"

#ifdef Q_OS_DARWIN
#include "bonjour.h"
#endif

Zeroconf* Zeroconf::instance_ = NULL;

Zeroconf* Zeroconf::GetZeroconf() {
  if (!instance_) {
  #ifdef Q_OS_DARWIN
    return new Bonjour();
  #endif
  }

  return instance_;
}
