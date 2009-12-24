#include "systemtrayicon.h"

#include <QEvent>
#include <QWheelEvent>

SystemTrayIcon::SystemTrayIcon(QObject* parent)
  : QSystemTrayIcon(parent)
{
}

bool SystemTrayIcon::event(QEvent* event) {
  if (event->type() == QEvent::Wheel) {
    emit WheelEvent(static_cast<QWheelEvent*>(event)->delta());
    return true;
  }
  return QSystemTrayIcon::event(event);
}
