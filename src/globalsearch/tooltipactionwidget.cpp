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
#include <QMouseEvent>
#include <QPainter>

const int TooltipActionWidget::kBorder = 16;
const int TooltipActionWidget::kSpacing = 6;
const int TooltipActionWidget::kTopPadding = 3;
const int TooltipActionWidget::kFadeDurationMsec = 200;

TooltipActionWidget::TooltipActionWidget(QWidget* parent)
  : QWidget(parent),
    kTextHeight(fontMetrics().height()),
    shortcut_width_(0),
    description_width_(0)
{
  setMouseTracking(true);
}

void TooltipActionWidget::SetActions(QList<QAction*> actions) {
  actions_ = actions;
  action_opacities_.clear();

  int h = kTopPadding + kTextHeight * actions.count();
  shortcut_width_ = 0;
  description_width_ = 0;

  foreach (const QAction* action, actions) {
    shortcut_width_ =
        qMax(shortcut_width_,
             fontMetrics().width(action->shortcut().toString(QKeySequence::NativeText)));
    description_width_ =
        qMax(description_width_, fontMetrics().width(action->text()));

    QTimeLine* timeline = new QTimeLine(kFadeDurationMsec, this);
    connect(timeline, SIGNAL(valueChanged(qreal)), SLOT(update()));
    action_opacities_ << timeline;
  }

  size_hint_ = QSize(
      kBorder*2 + shortcut_width_ + kSpacing + description_width_, h);

  updateGeometry();
  update();
}

void TooltipActionWidget::paintEvent(QPaintEvent*) {
  int y = kTopPadding;

  QPainter p(this);
  p.setPen(palette().color(QPalette::Text));

  for (int i=0 ; i<actions_.count() ; ++i) {
    const QAction* action = actions_[i];
    const QTimeLine* timeline = action_opacities_[i];

    const QRect shortcut_rect(kBorder, y, shortcut_width_, kTextHeight);
    const QRect description_rect(shortcut_rect.right() + kSpacing, y,
                                 description_width_, kTextHeight);

    const qreal shortcut_opacity = 0.4 + 0.3 * timeline->currentValue();
    const qreal description_opacity = 0.7 + 0.3 * timeline->currentValue();

    p.setOpacity(shortcut_opacity);
    p.drawText(shortcut_rect, Qt::AlignRight | Qt::AlignVCenter,
               action->shortcut().toString(QKeySequence::NativeText));

    p.setOpacity(description_opacity);
    p.drawText(description_rect, Qt::AlignVCenter, action->text());

    y += kTextHeight;
  }
}

int TooltipActionWidget::ActionAt(const QPoint& pos) const {
  return (pos.y() - kTopPadding) / kTextHeight;
}

void TooltipActionWidget::mouseMoveEvent(QMouseEvent* e) {
  const int action = ActionAt(e->pos());

  for (int i=0 ; i<actions_.count() ; ++i) {
    if (i == action) {
      StartAnimation(i, QTimeLine::Forward);
    } else {
      StartAnimation(i, QTimeLine::Backward);
    }
  }
}

void TooltipActionWidget::leaveEvent(QEvent* e) {
  for (int i=0 ; i<actions_.count() ; ++i) {
    StartAnimation(i, QTimeLine::Backward);
  }
}

void TooltipActionWidget::StartAnimation(int i, QTimeLine::Direction direction) {
  QTimeLine* timeline = action_opacities_[i];
  if (timeline->direction() != direction) {
    timeline->setDirection(direction);
    if (timeline->state() != QTimeLine::Running)
      timeline->resume();
  }
}

void TooltipActionWidget::mousePressEvent(QMouseEvent* e) {
  const int action = ActionAt(e->pos());
  if (action >= 0 && action < actions_.count()) {
    actions_[action]->trigger();
  }
}
