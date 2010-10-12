#include "macscreensaver.h"

#include <CoreServices/CoreServices.h>

#include <QtDebug>

MacScreensaver::MacScreensaver() {
  timer_.setInterval(30000);
  connect(&timer_, SIGNAL(timeout()), SLOT(Timeout()));
}

void MacScreensaver::Timeout() {
  UpdateSystemActivity(OverallAct);
}

void MacScreensaver::Inhibit() {
  timer_.start(30000);
  Timeout();
}

void MacScreensaver::Uninhibit() {
  timer_.stop();
}
