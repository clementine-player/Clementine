/* This file is part of Clementine.
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>

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

#include "renametablineedit.h"

#include <QKeyEvent>

RenameTabLineEdit::RenameTabLineEdit(QWidget* parent) : QLineEdit(parent) {}

void RenameTabLineEdit::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Escape) {
    e->accept();
    emit EditingCanceled();
  } else {
    QLineEdit::keyPressEvent(e);
  }
}

void RenameTabLineEdit::focusOutEvent(QFocusEvent* e) {
  // if the user hasn't explicitly accepted, discard the value
  emit EditingCanceled();
  // we don't call the default event since it will trigger editingFished()
}
