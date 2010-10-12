#include "screensaver.h"

#include <QtGlobal>

#ifdef Q_WS_X11
  #include "dbusscreensaver.h"
  #include <QDBusConnection>
  #include <QDBusConnectionInterface>
#endif

#ifdef Q_OS_DARWIN
  #include "macscreensaver.h"
#endif

#include <QtDebug>

const char* Screensaver::kGnomeService   = "org.gnome.ScreenSaver";
const char* Screensaver::kGnomePath      = "/";
const char* Screensaver::kGnomeInterface = "org.gnome.ScreenSaver";
const char* Screensaver::kKdeService     = "org.kde.ScreenSaver";
const char* Screensaver::kKdePath        = "/ScreenSaver/";
const char* Screensaver::kKdeInterface   = "org.freedesktop.ScreenSaver";

Screensaver* Screensaver::screensaver_ = 0;

Screensaver* Screensaver::GetScreensaver() {
  if (!screensaver_) {
    #if defined(Q_WS_X11)
      if (QDBusConnection::sessionBus().interface()->isServiceRegistered(kGnomeService)) {
        screensaver_ = new DBusScreensaver(kGnomeService, kGnomePath, kGnomeInterface);
      } else if (QDBusConnection::sessionBus().interface()->isServiceRegistered(kKdeService)) {
        screensaver_ = new DBusScreensaver(kKdeService, kKdePath, kKdeInterface);
      }
    #elif defined(Q_OS_DARWIN)
      screensaver_ = new MacScreensaver();
    #endif
  }
  return screensaver_;
}
