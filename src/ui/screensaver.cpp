#include "screensaver.h"

#include "gnomescreensaver.h"

#ifdef Q_WS_X11
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

Screensaver* Screensaver::screensaver_ = 0;

Screensaver* Screensaver::GetScreensaver() {
  if (!screensaver_) {
    #ifdef Q_WS_X11
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.gnome.ScreenSaver")) {
      screensaver_ = new GnomeScreensaver();
    }
    #endif
  }
  return screensaver_;
}
