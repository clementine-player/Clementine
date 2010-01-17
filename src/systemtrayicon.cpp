#include "systemtrayicon.h"

#include <QEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QtDebug>

#include <cmath>

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

void SystemTrayIcon::SetProgress(int percentage) {
  if (icon_.isNull()) {
    icon_ = icon().pixmap(geometry().size(), QIcon::Normal);
    grey_icon_ = icon().pixmap(geometry().size(), QIcon::Disabled);

    if (icon_.isNull())
      return;
  }

  QRect rect(icon_.rect());

  // The angle of the line that's used to cover the icon.
  // Centered on rect.topRight()
  double angle = double(100 - percentage) / 100.0 * M_PI_2 + M_PI;
  double length = sqrt(pow(rect.width(), 2.0) + pow(rect.height(), 2.0));

  QPolygon mask;
  mask << rect.topRight();
  mask << rect.topRight() + QPoint(
      length * sin(angle),
      - length * cos(angle));

  if (percentage > 50)
    mask << rect.bottomLeft();

  mask << rect.topLeft();
  mask << rect.topRight();

  QPixmap icon(icon_);
  QPainter p(&icon);
  p.setClipRegion(mask);
  p.drawPixmap(0, 0, grey_icon_);
  p.end();

  setIcon(icon);
}
