/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "songplaystats.h"

#include <QIcon>
#include <QPainter>

const int SongPlayStats::kIconSize = 16;
const int SongPlayStats::kLineSpacing = 2;
const int SongPlayStats::kIconTextSpacing = 6;
const int SongPlayStats::kMargin = 4;

SongPlayStats::SongPlayStats(QWidget* parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void SongPlayStats::AddItem(const QIcon& icon, const QString& text) {
  items_ << Item(icon, text);
  updateGeometry();
  update();
}

QSize SongPlayStats::sizeHint() const {
  return QSize(100, kMargin * 2 + items_.count() * kIconSize +
                        (items_.count() - 1) * kLineSpacing);
}

void SongPlayStats::paintEvent(QPaintEvent*) {
  QPainter p(this);

  int y = kMargin;
  foreach(const Item & item, items_) {
    const QRect line(kMargin, y, width() - kMargin * 2, kIconSize);
    const QRect icon_rect(line.topLeft(), QSize(kIconSize, kIconSize));
    const QRect text_rect(
        icon_rect.topRight() + QPoint(kIconTextSpacing, 0),
        QSize(line.width() - icon_rect.width() - kIconTextSpacing,
              line.height()));

    p.drawPixmap(icon_rect, item.icon_.pixmap(kIconSize));
    p.drawText(text_rect, item.text_);

    y += line.height() + kLineSpacing;
  }
}
