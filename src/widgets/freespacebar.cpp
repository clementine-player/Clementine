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

#include "freespacebar.h"

#include <QLinearGradient>
#include <QPainter>

const int FreeSpaceBar::kBarHeight = 20;
const int FreeSpaceBar::kBarBorderRadius = 8;
const int FreeSpaceBar::kMarkerSpacing = 32;

const QRgb FreeSpaceBar::kColorBg1 = qRgb(214, 207, 200);
const QRgb FreeSpaceBar::kColorBg2 = qRgb(234, 226, 218);
const QRgb FreeSpaceBar::kColorBar1 = qRgb(250, 148, 76);
const QRgb FreeSpaceBar::kColorBar2 = qRgb(214, 102, 24);
const QRgb FreeSpaceBar::kColorBorder = qRgb(174, 168, 162);


FreeSpaceBar::FreeSpaceBar(QWidget *parent)
  : QWidget(parent),
    free_(33),
    total_(100)
{
  setMinimumHeight(kBarHeight);
}

void FreeSpaceBar::set_free_bytes(quint64 bytes) {
  free_ = bytes;
  update();
}

void FreeSpaceBar::set_total_bytes(quint64 bytes) {
  total_ = bytes;
  update();
}

QSize FreeSpaceBar::sizeHint() const {
  return QSize(150, kBarHeight);
}

void FreeSpaceBar::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::HighQualityAntialiasing);

  QRect background_rect(rect());
  QRect bar_rect(background_rect);
  bar_rect.setWidth(float(bar_rect.width()) * (float(total_ - free_) / total_));

  QLinearGradient background_gradient(background_rect.topLeft(), background_rect.bottomLeft());
  background_gradient.setColorAt(0, kColorBg1);
  background_gradient.setColorAt(1, kColorBg2);

  QLinearGradient bar_gradient(bar_rect.topLeft(), bar_rect.bottomLeft());
  bar_gradient.setColorAt(0, kColorBar1);
  bar_gradient.setColorAt(1, kColorBar2);

  // Draw the background
  p.setPen(Qt::NoPen);
  p.setBrush(background_gradient);
  p.drawRoundedRect(background_rect, kBarBorderRadius, kBarBorderRadius);

  // Draw the bar foreground
  p.setBrush(bar_gradient);
  p.drawRoundedRect(bar_rect, kBarBorderRadius, kBarBorderRadius);

  // Draw marker lines over the top every few pixels
  p.setOpacity(0.35);
  p.setRenderHint(QPainter::Antialiasing, false);
  p.setPen(QPen(palette().color(QPalette::Light), 1.0));
  for (int x = background_rect.left() + kMarkerSpacing ;
       x < background_rect.right() ; x += kMarkerSpacing) {
    p.drawLine(x, background_rect.top() + 2, x, background_rect.bottom() - 2);
  }

  // Draw a border
  p.setRenderHint(QPainter::Antialiasing);
  p.setOpacity(1.0);
  p.setPen(kColorBorder);
  p.setBrush(Qt::NoBrush);
  p.drawRoundedRect(background_rect, kBarBorderRadius, kBarBorderRadius);
}
