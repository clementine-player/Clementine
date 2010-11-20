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

#include "spinbox.h"

#include <QtDebug>

SpinBox::SpinBox(QWidget *parent)
  : QSpinBox(parent),
    empty_value_(0)
{
}

int SpinBox::valueFromText(const QString &text) const {
  if (text.isEmpty())
    return empty_value_;
  return QSpinBox::valueFromText(text);
}

QString SpinBox::textFromValue(int val) const {
  if (val == empty_value_)
    return "";
  return QSpinBox::textFromValue(val);
}

void SpinBox::fixup(QString &str) const {
  if (str.isEmpty())
    return;
  QSpinBox::fixup(str);
}

QValidator::State SpinBox::validate(QString &input, int &pos) const {
  if (input.isEmpty())
    return QValidator::Acceptable;
  return QSpinBox::validate(input, pos);
}
