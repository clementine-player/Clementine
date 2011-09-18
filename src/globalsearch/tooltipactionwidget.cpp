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

#include "tooltipactionwidget.h"
#include "core/logging.h"

#include <QAction>
#include <QPainter>

const int TooltipActionWidget::kBorder = 16;
const int TooltipActionWidget::kSpacing = 6;

TooltipActionWidget::TooltipActionWidget(QWidget* parent)
  : QWidget(parent),
    kTextHeight(fontMetrics().height()),
    shortcut_width_(0),
    description_width_(0)
{
}

void TooltipActionWidget::SetActions(QList<QAction*> actions) {
  actions_ = actions;

  int h = 3 + kTextHeight * actions.count();
  shortcut_width_ = 0;
  description_width_ = 0;

  foreach (const QAction* action, actions) {
    shortcut_width_ =
        qMax(shortcut_width_,
             fontMetrics().width(action->shortcut().toString(QKeySequence::NativeText)));
    description_width_ =
        qMax(description_width_, fontMetrics().width(action->text()));
  }

  size_hint_ = QSize(
      kBorder*2 + shortcut_width_ + kSpacing + description_width_, h);

  updateGeometry();
  update();
}

void TooltipActionWidget::paintEvent(QPaintEvent*) {
  int y = 3;

  const qreal shortcut_opacity = 0.4;
  const qreal description_opacity = 0.7;

  QPainter p(this);
  p.setPen(palette().color(QPalette::Text));

  foreach (const QAction* action, actions_) {
    const QRect shortcut_rect(kBorder, y, shortcut_width_, kTextHeight);
    const QRect description_rect(shortcut_rect.right() + kSpacing, y,
                                 description_width_, kTextHeight);

    p.setOpacity(shortcut_opacity);
    p.drawText(shortcut_rect, Qt::AlignRight | Qt::AlignVCenter,
               action->shortcut().toString(QKeySequence::NativeText));

    p.setOpacity(description_opacity);
    p.drawText(description_rect, Qt::AlignVCenter, action->text());

    y += kTextHeight;
  }
}

void TooltipActionWidget::mousePressEvent(QMouseEvent* e) {

}
