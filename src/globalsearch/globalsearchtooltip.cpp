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

#include "globalsearchtooltip.h"
#include "tooltipactionwidget.h"
#include "tooltipresultwidget.h"
#include "core/logging.h"
#include "core/utilities.h"

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QLayoutItem>
#include <QPainter>
#include <QVBoxLayout>

const qreal GlobalSearchTooltip::kBorderRadius = 8.0;
const qreal GlobalSearchTooltip::kTotalBorderWidth = 4.0;
const qreal GlobalSearchTooltip::kOuterBorderWidth = 1.0;
const qreal GlobalSearchTooltip::kArrowWidth = 10.0;
const qreal GlobalSearchTooltip::kArrowHeight = 10.0;


GlobalSearchTooltip::GlobalSearchTooltip(QWidget* event_target)
  : QWidget(NULL),
    desktop_(qApp->desktop()),
    event_target_(event_target),
    active_result_(0)
{
  setWindowFlags(Qt::Popup);
  setFocusPolicy(Qt::NoFocus);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_TranslucentBackground);

  switch_action_ = new QAction(tr("Switch provider"), this);
  switch_action_->setShortcut(QKeySequence(Qt::Key_Tab));
  connect(switch_action_, SIGNAL(triggered()), SLOT(SwitchProvider()));
}

void GlobalSearchTooltip::SetResults(const SearchProvider::ResultList& results) {
  results_ = results;

  qDeleteAll(widgets_);
  widgets_.clear();
  result_buttons_.clear();

  active_result_ = 0;

  // Using a QVBoxLayout here made some weird flickering that I couldn't figure
  // out how to fix, so do layout manually.
  int w = 0;
  int y = 9;

  // Add a widget for each result
  foreach (const SearchProvider::Result& result, results) {
    TooltipResultWidget* widget = new TooltipResultWidget(result, this);
    if (widgets_.isEmpty()) {
      // If this is the first widget then mark it as selected
      widget->setChecked(true);
    }

    AddWidget(widget, &w, &y);
    result_buttons_ << widget;
  }

  // Add the action widget
  QList<QAction*> actions;
  if (results_.count() > 1) {
    actions.append(switch_action_);
  }
  actions.append(common_actions_);

  action_widget_ = new TooltipActionWidget(this);
  action_widget_->SetActions(actions);
  AddWidget(action_widget_, &w, &y);

  // Set the width of each widget
  foreach (QWidget* widget, widgets_) {
    widget->resize(w, widget->sizeHint().height());
  }

  // Resize this widget
  y += 9;
  resize(w, y);

  inner_rect_ = rect().adjusted(
        kArrowWidth + kTotalBorderWidth, kTotalBorderWidth,
        -kTotalBorderWidth, -kTotalBorderWidth);

  foreach (QWidget* widget, widgets_) {
    widget->setMask(inner_rect_);
  }
}

void GlobalSearchTooltip::AddWidget(QWidget* widget, int* w, int* y) {
  widget->move(0, *y);
  widget->show();
  widgets_ << widget;

  QSize size_hint(widget->sizeHint());
  *y += size_hint.height();
  *w = qMax(*w, size_hint.width());
}

void GlobalSearchTooltip::ShowAt(const QPoint& pointing_to) {
  const qreal min_arrow_offset = kBorderRadius + kArrowHeight;
  const QRect screen = desktop_->screenGeometry(this);

  arrow_offset_ = min_arrow_offset +
                  qMax(0, pointing_to.y() + height() - screen.bottom());

  move(pointing_to.x(), pointing_to.y() - arrow_offset_);

  if (!isVisible())
    show();
}

bool GlobalSearchTooltip::event(QEvent* e) {
  switch (e->type()) {
  case QEvent::KeyPress: {
    QKeyEvent* ke = static_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Tab && ke->modifiers() == Qt::NoModifier) {
      SwitchProvider();
      e->accept();
      return true;
    }

    // fallthrough
  }
  case QEvent::KeyRelease:
  case QEvent::InputMethod:
  case QEvent::Shortcut:
  case QEvent::ShortcutOverride:
    if (QApplication::sendEvent(event_target_, e)) {
      return true;
    }
    break;

  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseButtonDblClick:
    if (!underMouse()) {
      const QMouseEvent* me = static_cast<QMouseEvent*>(e);
      QWidget* child = event_target_->childAt(
            event_target_->mapFromGlobal(me->globalPos()));

      if (child)
        child->setAttribute(Qt::WA_UnderMouse, true);

      Utilities::ForwardMouseEvent(me, child ? child : event_target_);
      return true;
    }
    break;

  default:
    break;
  }

  return QWidget::event(e);
}

void GlobalSearchTooltip::paintEvent(QPaintEvent*) {
  QPainter p(this);

  const QColor outer_color = Qt::black;
  const QColor inner_color = palette().color(QPalette::Highlight);
  const QColor center_color = palette().color(QPalette::Base);

  // Transparent background
  p.fillRect(rect(), Qt::transparent);

  QRect area(inner_rect_.adjusted(
      -kTotalBorderWidth/2, -kTotalBorderWidth/2,
      kTotalBorderWidth/2, kTotalBorderWidth/2));

  // Draw the border
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(QPen(outer_color, kTotalBorderWidth));
  p.drawRoundedRect(area, kBorderRadius, kBorderRadius);

  // Draw the arrow
  QPolygonF arrow;
  arrow << QPointF(kArrowWidth + 2, arrow_offset_ - kArrowHeight)
        << QPointF(0, arrow_offset_)
        << QPointF(kArrowWidth + 2, arrow_offset_ + kArrowHeight);

  p.setBrush(outer_color);
  p.setPen(outer_color);
  p.drawPolygon(arrow);

  // Now draw the inner shapes on top
  const qreal inner_border_width = kTotalBorderWidth - kOuterBorderWidth;

  QRect inner_area(inner_rect_.adjusted(
      -inner_border_width/2, -inner_border_width/2,
      inner_border_width/2, inner_border_width/2));

  // Inner border
  p.setBrush(center_color);
  p.setPen(QPen(inner_color, inner_border_width));
  p.drawRoundedRect(inner_area, kBorderRadius, kBorderRadius);

  // Inner arrow
  arrow[0].setY(arrow[0].y() + kOuterBorderWidth);
  arrow[1].setX(arrow[1].x() + kOuterBorderWidth + 1);
  arrow[2].setY(arrow[2].y() - kOuterBorderWidth);

  p.setBrush(inner_color);
  p.setPen(inner_color);
  p.drawPolygon(arrow);
}

void GlobalSearchTooltip::SwitchProvider() {
  // Find which one was checked before.
  int old_index = -1;
  for (int i=0 ; i<result_buttons_.count() ; ++i) {
    if (result_buttons_[i]->isChecked()) {
      old_index = i;
      break;
    }
  }

  if (old_index == -1)
    return;

  // Check the new one.  The auto exclusive group will take care of unchecking
  // the old one.
  active_result_ = (old_index + 1) % result_buttons_.count();
  result_buttons_[active_result_]->setChecked(true);
}

