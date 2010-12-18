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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class QToolButton;

class LineEditInterface {
 public:
  LineEditInterface(QObject* object) : object_(object) {}
  virtual ~LineEditInterface() {}
  virtual void clear() = 0;
  virtual void setFocus() = 0;
  virtual void setText(const QString&) = 0;
  virtual QString text() const = 0;
  virtual void set_hint(const QString&) = 0;

  QObject* object() const { return object_; }

 private:
  QObject* object_;
};

class LineEdit : public QLineEdit, public LineEditInterface {
  Q_OBJECT
  Q_PROPERTY(QString hint READ hint WRITE set_hint);
  Q_PROPERTY(bool has_clear_button READ has_clear_button WRITE set_clear_button);

 public:
  LineEdit(QWidget* parent = 0);

  QString hint() const { return hint_; }
  void set_hint(const QString& hint);
  void clear_hint() { set_hint(QString()); }

  bool has_clear_button() const { return has_clear_button_; }
  void set_clear_button(bool visible);

  void clear() { QLineEdit::clear(); }
  void setFocus() { QLineEdit::setFocus(); }
  void setText(const QString& t) { QLineEdit::setText(t); }
  QString text() const { return QLineEdit::text(); }

 protected:
  void paintEvent(QPaintEvent* e);
  void resizeEvent(QResizeEvent*);

 private:
  QString hint_;

  bool has_clear_button_;
  QToolButton* clear_button_;
};

#endif // LINEEDIT_H
