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

#include "didyoumean.h"

#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QToolButton>

const int DidYouMean::kPadding = 3;

DidYouMean::DidYouMean(QWidget* buddy, QWidget* parent)
  : QWidget(parent),
    buddy_(buddy),
    close_(new QToolButton(this)),
    normal_font_(font()),
    correction_font_(font()),
    press_enter_font_(font()) {
  // Close icon
  close_->setToolTip(tr("Close"));
  close_->setIcon(QIcon(":/trolltech/styles/macstyle/images/closedock-16.png"));
  close_->setIconSize(QSize(16, 16));
  connect(close_, SIGNAL(clicked()), SLOT(hide()));

  // Cursors
  setCursor(Qt::PointingHandCursor);
  close_->setCursor(Qt::ArrowCursor);

  // Fonts
  correction_font_.setBold(true);
  press_enter_font_.setBold(true);
  press_enter_font_.setPointSizeF(7.5);

  hide();
  buddy_->installEventFilter(this);
}

bool DidYouMean::eventFilter(QObject* object, QEvent* event) {
  if (object != buddy_) {
    return QObject::eventFilter(object, event);
  }

  switch (event->type()) {
    case QEvent::Move:
    case QEvent::Resize:
      if (isVisible()) {
        UpdateGeometry();
      }
      break;

    case QEvent::KeyPress:
      if (!isVisible()) {
        break;
      }

      switch (static_cast<QKeyEvent*>(event)->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          emit Accepted(correction_);
          // fallthrough
        case Qt::Key_Escape:
          hide();
          return true;

        default:
          break;
      }

      break;

    case QEvent::FocusOut:
      hide();
      break;

    default:
      break;
  }

  return QObject::eventFilter(object, event);
}

void DidYouMean::showEvent(QShowEvent*) {
  UpdateGeometry();
}

void DidYouMean::UpdateGeometry() {
  const int text_height = fontMetrics().height();
  const int height = text_height + kPadding * 2;

  move(buddy_->mapTo(parentWidget(), buddy_->rect().bottomLeft()));
  resize(QSize(buddy_->width(), height));

  close_->move(kPadding, kPadding);
  close_->resize(text_height, text_height);
}

void DidYouMean::paintEvent(QPaintEvent*) {
  QPainter p(this);

  // Draw the background
  QColor bg(palette().color(QPalette::Inactive, QPalette::ToolTipBase));
  p.fillRect(0, 0, width()-1, height()-1, bg);

  // Border
  p.setPen(Qt::black);
  p.drawRect(0, 0, width()-1, height()-1);

  // Text rectangle
  QRect text_rect(kPadding + close_->width() + kPadding,
                  kPadding,
                  rect().width() - kPadding,
                  rect().height() - kPadding);
  const QString did_you_mean(tr("Did you mean") + ":  ");

  // Text
  p.setFont(normal_font_);
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, did_you_mean);
  text_rect.setLeft(text_rect.left() + p.fontMetrics().width(did_you_mean));

  p.setFont(correction_font_);
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, correction_);
  text_rect.setLeft(text_rect.left() + p.fontMetrics().width(correction_ + "  "));

  p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
  p.setFont(press_enter_font_);
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, "(" + tr("press enter") + ")");
}

void DidYouMean::SetCorrection(const QString& correction) {
  correction_ = correction;
  update();
}

void DidYouMean::Show(const QString& correction) {
  SetCorrection(correction);
  show();
}

void DidYouMean::mouseReleaseEvent(QMouseEvent* e) {
  emit Accepted(correction_);
  hide();
}
