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

#ifndef DIDYOUMEAN_H
#define DIDYOUMEAN_H

#include <QWidget>

class QToolButton;

class DidYouMean : public QWidget {
  Q_OBJECT

 public:
  DidYouMean(QWidget* buddy, QWidget* parent);

  static const int kPadding;

 public slots:
  void SetCorrection(const QString& correction);
  void Show(const QString& correction);

 signals:
  void Accepted(const QString& correction);

 protected:
  void paintEvent(QPaintEvent*);
  void showEvent(QShowEvent*);
  void mouseReleaseEvent(QMouseEvent* e);
  bool eventFilter(QObject* object, QEvent* event);

 private:
  void UpdateGeometry();

 private:
  QWidget* buddy_;
  QString correction_;

  QToolButton* close_;

  QFont normal_font_;
  QFont correction_font_;
  QFont press_enter_font_;

  QString did_you_mean_;
  QString press_enter_;

  // Size of the text to display, according to QFonts above.
  // Stored here to avoid to recompute them each time
  int did_you_mean_size_;
  int press_enter_size_;
};

#endif  // DIDYOUMEAN_H
