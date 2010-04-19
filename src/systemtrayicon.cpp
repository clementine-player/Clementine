/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "systemtrayicon.h"

#include <QApplication>
#include <QEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QWidget>
#include <QtDebug>

#include <cmath>

SystemTrayIcon::SystemTrayIcon(QObject* parent)
  : QSystemTrayIcon(parent),
    playing_icon_(":/tiny-start.png"),
    paused_icon_(":/tiny-pause.png"),
    percentage_(0)
{
#ifdef Q_OS_DARWIN
  hide();
#endif
}

SystemTrayIcon::~SystemTrayIcon() {}

bool SystemTrayIcon::event(QEvent* event) {
  if (event->type() == QEvent::Wheel) {
    emit WheelEvent(static_cast<QWheelEvent*>(event)->delta());
    return true;
  }
  return QSystemTrayIcon::event(event);
}

void SystemTrayIcon::SetProgress(int percentage) {
  percentage_ = percentage;
  Update();
}

void SystemTrayIcon::Update() {
  if (icon_.isNull()) {
#ifdef Q_OS_DARWIN
    QIcon big_icon(":icon_large.png");
    icon_ = big_icon.pixmap(128, 128, QIcon::Normal);
    QIcon big_grey_icon(":icon_large_grey.png");
    grey_icon_ = big_grey_icon.pixmap(128, 128, QIcon::Normal);
    grey_icon_.save("grey.png");
#else
    icon_ = icon().pixmap(geometry().size(), QIcon::Normal);
    grey_icon_ = icon().pixmap(geometry().size(), QIcon::Disabled);
#endif

    if (icon_.isNull())
      return;
  }

  QRect rect(icon_.rect());

  // The angle of the line that's used to cover the icon.
  // Centered on rect.topRight()
  double angle = double(100 - percentage_) / 100.0 * M_PI_2 + M_PI;
  double length = sqrt(pow(rect.width(), 2.0) + pow(rect.height(), 2.0));

  QPolygon mask;
  mask << rect.topRight();
  mask << rect.topRight() + QPoint(
      length * sin(angle),
      - length * cos(angle));

  if (percentage_ > 50)
    mask << rect.bottomLeft();

  mask << rect.topLeft();
  mask << rect.topRight();

  QPixmap icon(icon_);
  QPainter p(&icon);

  // Draw the grey bit over the orange icon
  p.setClipRegion(mask);
  p.drawPixmap(0, 0, grey_icon_);
  p.setClipping(false);

  // Draw the playing or paused icon in the top-right
  if (!current_state_icon_.isNull()) {
    int height = rect.height() / 2;
    QPixmap scaled(current_state_icon_.scaledToHeight(height, Qt::SmoothTransformation));

    QRect state_rect(rect.width() - scaled.width(), 0, scaled.width(), scaled.height());
    p.drawPixmap(state_rect, scaled);
  }

  p.end();

#ifdef Q_OS_DARWIN
  // Setting main window icon.
  QApplication::setWindowIcon(icon);
  icon.save("icon.png");
#else
  setIcon(icon);
#endif
}

void SystemTrayIcon::SetPaused() {
  current_state_icon_ = paused_icon_;
  Update();
}

void SystemTrayIcon::SetPlaying() {
  current_state_icon_ = playing_icon_;
  Update();
}

void SystemTrayIcon::SetStopped() {
  current_state_icon_ = QPixmap();
  Update();
}
