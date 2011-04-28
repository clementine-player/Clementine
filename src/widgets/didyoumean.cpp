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
#include <QPainter>
#include <QToolButton>

const int DidYouMean::kPadding = 3;

DidYouMean::DidYouMean(QWidget* buddy, QWidget* parent)
  : QWidget(parent),
    buddy_(buddy),
    close_(new QToolButton(this)) {
  close_->setToolTip(tr("Close"));
  close_->setIcon(QIcon(":/trolltech/styles/macstyle/images/closedock-16.png"));
  close_->setIconSize(QSize(16, 16));
  connect(close_, SIGNAL(clicked()), SLOT(hide()));

  setCursor(Qt::PointingHandCursor);
  close_->setCursor(Qt::ArrowCursor);

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

void DidYouMean::paintEvent(QPaintEvent* ) {
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
  const QString did_you_mean(tr("Did you mean "));

  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, did_you_mean);
  text_rect.setLeft(text_rect.left() + fontMetrics().width(did_you_mean));

  QFont bold_font(font());
  bold_font.setBold(true);
  bold_font.setItalic(true);
  p.setFont(bold_font);

  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text_);
}

void DidYouMean::SetText(const QString& text) {
  text_ = text;
  update();
}

void DidYouMean::Show(const QString& text) {
  SetText(text);
  show();
}

void DidYouMean::mouseReleaseEvent(QMouseEvent* e) {
  emit Accepted(text_);
  hide();
}
