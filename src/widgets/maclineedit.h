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

#ifndef MACLINEEDIT_H
#define MACLINEEDIT_H

#include <QMacCocoaViewContainer>

#include "lineedit.h"

class SearchTargetWrapper;

class MacLineEdit : public QMacCocoaViewContainer, public LineEditInterface {
  Q_OBJECT
  Q_PROPERTY(QString hint READ GetHint WRITE SetHint);

 public:
  MacLineEdit(QWidget* parent = 0);
  ~MacLineEdit();

  QString GetHint() const { return hint_; }
  void SetHint(const QString& hint);
  void ClearHint() { SetHint(QString()); }

  void paintEvent(QPaintEvent* e);

  void clear() {}

  void setText(const QString&);
  QString text() const;
  void setFocus() {}

 signals:
  void textChanged(const QString& text);
  void textEdited(const QString& text);

 private:
  // Called by NSSearchFieldCell when the text changes.
  void TextChanged(const QString& text);

  QString hint_;

  friend class SearchTargetWrapper;
  SearchTargetWrapper* wrapper_;
};

#endif // MACLINEEDIT_H
