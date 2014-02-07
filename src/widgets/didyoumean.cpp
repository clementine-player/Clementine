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

#include "core/logging.h"

#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QToolButton>

const int DidYouMean::kPadding = 3;

DidYouMean::DidYouMean(QWidget* buddy, QWidget* parent)
    : QWidget(parent, Qt::ToolTip),
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

  // Texts
  did_you_mean_ = tr("Did you mean") + ":  ";
  press_enter_ = "(" + tr("press enter") + ")";

  // Texts' sizes
  did_you_mean_size_ = QFontMetrics(normal_font_).width(did_you_mean_);
  press_enter_size_ = QFontMetrics(press_enter_font_).width(press_enter_);
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
    case QEvent::WindowDeactivate:
      hide();
      break;

    default:
      break;
  }

  return QObject::eventFilter(object, event);
}

void DidYouMean::showEvent(QShowEvent*) { UpdateGeometry(); }

void DidYouMean::UpdateGeometry() {
  const int text_height = fontMetrics().height();
  const int height = text_height + kPadding * 2;

  move(buddy_->mapToGlobal(buddy_->rect().bottomLeft()));
  // Resize to len(text to display) + total number of padding added +
  // size(close button), so the "Did you mean" widget is always fully displayed

  resize(QSize(did_you_mean_size_ +
                   QFontMetrics(correction_font_).width(correction_ + "  ") +
                   press_enter_size_ + kPadding * 6 + close_->width(),
               height));

  close_->move(kPadding, kPadding);
  close_->resize(text_height, text_height);
}

void DidYouMean::paintEvent(QPaintEvent*) {
  QPainter p(this);

  // Draw the background
  QColor bg(palette().color(QPalette::Inactive, QPalette::ToolTipBase));
  p.fillRect(0, 0, width() - 1, height() - 1, bg);

  // Border
  p.setPen(Qt::black);
  p.drawRect(0, 0, width() - 1, height() - 1);

  // Text rectangle
  QRect text_rect(kPadding + close_->width() + kPadding, kPadding,
                  rect().width() - kPadding, rect().height() - kPadding);

  // Text
  p.setFont(normal_font_);
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, did_you_mean_);
  text_rect.setLeft(text_rect.left() + p.fontMetrics().width(did_you_mean_));

  p.setFont(correction_font_);
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, correction_);
  text_rect.setLeft(text_rect.left() +
                    p.fontMetrics().width(correction_ + "  "));

  p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
  p.setFont(press_enter_font_);
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, press_enter_);
}

void DidYouMean::SetCorrection(const QString& correction) {
  correction_ = correction;
  UpdateGeometry();
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
