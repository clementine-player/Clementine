#ifndef MACSCREENSAVER_H
#define MACSCREENSAVER_H

#include "screensaver.h"

#include <QTimer>

class MacScreensaver : public QObject, public Screensaver {
  Q_OBJECT
 public:
  MacScreensaver();

  void Inhibit();
  void Uninhibit();

 private slots:
  void Timeout();

 private:
  QTimer timer_;
};

#endif
