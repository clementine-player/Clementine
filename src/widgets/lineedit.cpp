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

#include "lineedit.h"

#include <QPainter>
#include <QPaintEvent>

LineEdit::LineEdit(QWidget* parent)
  : QLineEdit(parent),
    LineEditInterface(this)
{
}

void LineEdit::SetHint(const QString& hint) {
  hint_ = hint;
  update();
}

void LineEdit::paintEvent(QPaintEvent* e) {
  QLineEdit::paintEvent(e);

  if (!hasFocus() && displayText().isEmpty() && !hint_.isEmpty()) {
    QPainter p(this);

    QFont font;
    font.setItalic(true);
    font.setPointSize(font.pointSize()-1);

    QFontMetrics m(font);
    const int kBorder = (height() - m.height()) / 2;

    p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
    p.setFont(font);

    QRect r(rect().topLeft() + QPoint(kBorder + 5, kBorder),
            rect().bottomRight() - QPoint(kBorder, kBorder));
    p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, hint_);
  }
}
