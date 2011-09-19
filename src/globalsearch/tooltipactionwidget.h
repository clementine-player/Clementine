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

#ifndef TOOLTIPACTIONWIDGET_H
#define TOOLTIPACTIONWIDGET_H

#include <QTimeLine>
#include <QWidget>


class TooltipActionWidget : public QWidget {
  Q_OBJECT

public:
  TooltipActionWidget(QWidget* parent = 0);

  static const int kBorder;
  static const int kSpacing;
  static const int kTopPadding;
  static const int kFadeDurationMsec;

  void SetActions(QList<QAction*> actions);

  QSize sizeHint() const { return size_hint_; }

protected:
  void paintEvent(QPaintEvent*);
  void mouseMoveEvent(QMouseEvent* e);
  void leaveEvent(QEvent* e);
  void mousePressEvent(QMouseEvent* e);

private:
  int ActionAt(const QPoint& pos) const;
  void StartAnimation(int i, QTimeLine::Direction direction);

private:
  const int kTextHeight;

  QList<QAction*> actions_;
  QList<QTimeLine*> action_opacities_;

  QSize size_hint_;
  int shortcut_width_;
  int description_width_;
};

#endif // TOOLTIPACTIONWIDGET_H
