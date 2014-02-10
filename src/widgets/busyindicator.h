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

#ifndef BUSYINDICATOR_H
#define BUSYINDICATOR_H

#include <QLabel>

class QMovie;

class BusyIndicator : public QWidget {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE set_text)

 public:
  explicit BusyIndicator(const QString& text, QWidget* parent = nullptr);
  explicit BusyIndicator(QWidget* parent = nullptr);
  ~BusyIndicator();

  QString text() const;
  void set_text(const QString& text);

 protected:
  void showEvent(QShowEvent* event);
  void hideEvent(QHideEvent* event);

 private:
  void Init(const QString& text);

 private:
  QMovie* movie_;
  QLabel* label_;
};

#endif  // BUSYINDICATOR_H
