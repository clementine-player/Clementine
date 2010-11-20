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

#ifndef SPINBOX_H
#define SPINBOX_H

#include <QSpinBox>

// A spinbox that accepts the empty string as input
class SpinBox : public QSpinBox {
  Q_OBJECT
  Q_PROPERTY(int empty_value READ empty_value WRITE set_empty_value);

 public:
  SpinBox(QWidget *parent = 0);

  // The empty_value must still be within the range of the spinbox.
  // Defaults to 0
  int empty_value() const { return empty_value_; }
  void set_empty_value(int v) { empty_value_ = v; }

 protected:
  int valueFromText(const QString &text) const;
  QString textFromValue(int val) const;
  void fixup(QString &str) const;
  QValidator::State validate(QString &input, int &pos) const;

 private:
  int empty_value_;
};

#endif // SPINBOX_H
