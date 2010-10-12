#ifndef GNOMESCREENSAVER_H
#define GNOMESCREENSAVER_H

#include "screensaver.h"

#include <QtGlobal>

class GnomeScreensaver : public Screensaver {
 public:
  void Inhibit();
  void Uninhibit();

 private:
  quint32 cookie_;
};

#endif
