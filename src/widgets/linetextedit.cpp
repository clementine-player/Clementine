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

#include "linetextedit.h"

#include <QKeyEvent>

LineTextEdit::LineTextEdit(QWidget* parent) : QTextEdit(parent) {
  setWordWrapMode(QTextOption::NoWrap);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setTabChangesFocus(true);
  sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);
}

QSize LineTextEdit::sizeHint() const {
  QFontMetrics fm(font());

  static const int kMargin = 5;
  int h = 2 * kMargin + qMax(fm.height(), 14);
  int w = 2 * kMargin + fm.width("W") * 15;

  return QSize(w, h);
}

QSize LineTextEdit::minimumSizeHint() const { return sizeHint(); }

void LineTextEdit::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
    e->ignore();
  } else {
    QTextEdit::keyPressEvent(e);
  }
}
