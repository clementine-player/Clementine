#include "dbusscreensaver.h"

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>

DBusScreensaver::DBusScreensaver(const QString& service, const QString& path,
                                 const QString& interface)
  : service_(service),
    path_(path),
    interface_(interface)
{
}

void DBusScreensaver::Inhibit() {
  QDBusInterface gnome_screensaver("org.gnome.ScreenSaver", "/", "org.gnome.ScreenSaver");
  QDBusReply<quint32> reply =
      gnome_screensaver.call("Inhibit", QCoreApplication::applicationName(), QObject::tr("Visualizations"));
  if (reply.isValid()) {
    cookie_ = reply.value();
  }
}

void DBusScreensaver::Uninhibit() {
  QDBusInterface gnome_screensaver("org.gnome.ScreenSaver", "/", "org.gnome.ScreenSaver");
  gnome_screensaver.call("UnInhibit", cookie_);
}
