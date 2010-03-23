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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEdit : public QLineEdit {
  Q_OBJECT
  Q_PROPERTY(QString hint READ GetHint WRITE SetHint);

 public:
  LineEdit(QWidget* parent = 0);

  QString GetHint() const { return hint_; }
  void SetHint(const QString& hint);

  void paintEvent(QPaintEvent* e);

 private:
  QString hint_;
};

#endif // LINEEDIT_H
