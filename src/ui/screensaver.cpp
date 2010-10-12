#include "screensaver.h"

#include <QtGlobal>

#ifdef Q_WS_X11
#include "gnomescreensaver.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#ifdef Q_OS_DARWIN
#include "macscreensaver.h"
#endif

#include <QtDebug>

Screensaver* Screensaver::screensaver_ = 0;

Screensaver* Screensaver::GetScreensaver() {
  qDebug() << Q_FUNC_INFO;
  if (!screensaver_) {
    #if defined(Q_WS_X11)
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.gnome.ScreenSaver")) {
      screensaver_ = new GnomeScreensaver();
    }
    #elif defined(Q_OS_DARWIN)
    screensaver_ = new MacScreensaver();
    #endif
  }
  return screensaver_;
}
