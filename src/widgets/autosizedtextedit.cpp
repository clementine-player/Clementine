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

#include "autosizedtextedit.h"

#include <QWheelEvent>

AutoSizedTextEdit::AutoSizedTextEdit(QWidget* parent)
  : QTextEdit(parent)
{
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void AutoSizedTextEdit::resizeEvent(QResizeEvent* e) {
  const int w = qMax(100, width());

  document()->setTextWidth(w);
  setMinimumHeight(document()->size().height());

  QTextEdit::resizeEvent(e);
}

QSize AutoSizedTextEdit::sizeHint() const {
  return minimumSize();
}

void AutoSizedTextEdit::wheelEvent(QWheelEvent* e) {
  e->ignore();
}
