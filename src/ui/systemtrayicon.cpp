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
  : QObject(parent),
    percentage_(0),
    playing_icon_(":/tiny-start.png"),
    paused_icon_(":/tiny-pause.png")
{
#ifdef Q_OS_DARWIN
  hide();
#endif
}

QPixmap SystemTrayIcon::CreateIcon(const QPixmap& icon, const QPixmap& grey_icon) {
  QRect rect(icon.rect());

  // The angle of the line that's used to cover the icon.
  // Centered on rect.topRight()
  double angle = double(100 - song_progress()) / 100.0 * M_PI_2 + M_PI;
  double length = sqrt(pow(rect.width(), 2.0) + pow(rect.height(), 2.0));

  QPolygon mask;
  mask << rect.topRight();
  mask << rect.topRight() + QPoint(
      length * sin(angle),
      - length * cos(angle));

  if (song_progress() > 50)
    mask << rect.bottomLeft();

  mask << rect.topLeft();
  mask << rect.topRight();

  QPixmap ret(icon);
  QPainter p(&ret);

  // Draw the grey bit over the orange icon
  p.setClipRegion(mask);
  p.drawPixmap(0, 0, grey_icon);
  p.setClipping(false);

  // Draw the playing or paused icon in the top-right
  if (!current_state_icon().isNull()) {
    int height = rect.height() / 2;
    QPixmap scaled(current_state_icon().scaledToHeight(height, Qt::SmoothTransformation));

    QRect state_rect(rect.width() - scaled.width(), 0, scaled.width(), scaled.height());
    p.drawPixmap(state_rect, scaled);
  }

  p.end();

  return ret;
}

void SystemTrayIcon::SetProgress(int percentage) {
  percentage_ = percentage;
  UpdateIcon();
}

void SystemTrayIcon::SetPaused() {
  current_state_icon_ = paused_icon_;
  UpdateIcon();
}

void SystemTrayIcon::SetPlaying() {
  current_state_icon_ = playing_icon_;
  UpdateIcon();
}

void SystemTrayIcon::SetStopped() {
  current_state_icon_ = QPixmap();
  UpdateIcon();
}
