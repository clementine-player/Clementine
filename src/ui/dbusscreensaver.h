#ifndef DBUSSCREENSAVER_H
#define DBUSSCREENSAVER_H

#include "screensaver.h"

#include <QString>

class DBusScreensaver : public Screensaver {
 public:
  DBusScreensaver(const QString& service, const QString& path,
                  const QString& interface);

  void Inhibit();
  void Uninhibit();

private:
  QString service_;
  QString path_;
  QString interface_;

  quint32 cookie_;
};

#endif
