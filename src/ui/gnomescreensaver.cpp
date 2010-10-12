#include "gnomescreensaver.h"

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>

void GnomeScreensaver::Inhibit() {
  QDBusInterface gnome_screensaver("org.gnome.ScreenSaver", "/", "org.gnome.ScreenSaver");
  QDBusReply<quint32> reply =
      gnome_screensaver.call("Inhibit", QCoreApplication::applicationName(), QObject::tr("Visualizations"));
  if (reply.isValid()) {
    cookie_ = reply.value();
  }
}

void GnomeScreensaver::Uninhibit() {
  QDBusInterface gnome_screensaver("org.gnome.ScreenSaver", "/", "org.gnome.ScreenSaver");
  gnome_screensaver.call("UnInhibit", cookie_);
}
