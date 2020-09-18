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

#include "prettyimageview.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QTimer>
#include <QtDebug>

#include "prettyimage.h"

PrettyImageView::PrettyImageView(QNetworkAccessManager* network,
                                 QWidget* parent)
    : QScrollArea(parent),
      network_(network),
      container_(new QWidget(this)),
      layout_(new QHBoxLayout(container_)),
      current_index_(-1),
      scroll_animation_(
          new QPropertyAnimation(horizontalScrollBar(), "value", this)),
      recursion_filter_(false) {
  setWidget(container_);
  setWidgetResizable(true);
  setMinimumHeight(PrettyImage::kTotalHeight + 10);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFrameShape(QFrame::NoFrame);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  scroll_animation_->setDuration(250);
  scroll_animation_->setEasingCurve(QEasingCurve::InOutCubic);
  connect(horizontalScrollBar(), SIGNAL(sliderReleased()),
          SLOT(ScrollBarReleased()));
  connect(horizontalScrollBar(), SIGNAL(actionTriggered(int)),
          SLOT(ScrollBarAction(int)));

  layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
  layout_->setContentsMargins(6, 6, 6, 6);
  layout_->setSpacing(6);
  layout_->addSpacing(200);
  layout_->addSpacing(200);

  container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

bool PrettyImageView::eventFilter(QObject* obj, QEvent* event) {
  // Work around infinite recursion in QScrollArea resizes.
  if (recursion_filter_) {
    return false;
  }
  recursion_filter_ = true;
  bool ret = QScrollArea::eventFilter(obj, event);
  recursion_filter_ = false;
  return ret;
}

void PrettyImageView::AddImage(const QUrl& url) {
  PrettyImage* image = new PrettyImage(url, network_, container_);
  connect(image, SIGNAL(destroyed()), SLOT(ScrollToCurrent()));
  connect(image, SIGNAL(Loaded()), SLOT(ScrollToCurrent()));

  layout_->insertWidget(layout_->count() - 1, image);
  if (current_index_ == -1) ScrollTo(0);
}

void PrettyImageView::mouseReleaseEvent(QMouseEvent* e) {
  // Find the image that was clicked on
  QWidget* widget = container_->childAt(container_->mapFrom(this, e->pos()));
  if (!widget) return;

  // Get the index of that image
  const int index = layout_->indexOf(widget) - 1;
  if (index == -1) return;

  if (index == current_index_) {
    // Show the image fullsize
    PrettyImage* pretty_image = qobject_cast<PrettyImage*>(widget);
    if (pretty_image) {
      pretty_image->ShowFullsize();
    }
  } else {
    // Scroll to the image
    ScrollTo(index);
  }
}

void PrettyImageView::ScrollTo(int index, bool smooth) {
  current_index_ = qBound(0, index, layout_->count() - 3);
  const int layout_index = current_index_ + 1;

  const QWidget* target_widget = layout_->itemAt(layout_index)->widget();
  if (!target_widget) return;

  const int current_x = horizontalScrollBar()->value();
  const int target_x = target_widget->geometry().center().x() - width() / 2;

  if (current_x == target_x) return;

  if (smooth) {
    scroll_animation_->setStartValue(current_x);
    scroll_animation_->setEndValue(target_x);
    scroll_animation_->start();
  } else {
    scroll_animation_->stop();
    horizontalScrollBar()->setValue(target_x);
  }
}

void PrettyImageView::ScrollToCurrent() { ScrollTo(current_index_); }

void PrettyImageView::ScrollBarReleased() {
  // Find the nearest widget to where the scroll bar was released
  const int current_x = horizontalScrollBar()->value() + width() / 2;
  int layout_index = 1;
  for (; layout_index < layout_->count() - 1; ++layout_index) {
    const QWidget* widget = layout_->itemAt(layout_index)->widget();
    if (widget && widget->geometry().right() > current_x) {
      break;
    }
  }

  ScrollTo(layout_index - 1);
}

void PrettyImageView::ScrollBarAction(int action) {
  switch (action) {
    case QAbstractSlider::SliderSingleStepAdd:
    case QAbstractSlider::SliderPageStepAdd:
      ScrollTo(current_index_ + 1);
      break;

    case QAbstractSlider::SliderSingleStepSub:
    case QAbstractSlider::SliderPageStepSub:
      ScrollTo(current_index_ - 1);
      break;
  }
}

void PrettyImageView::resizeEvent(QResizeEvent* e) {
  QScrollArea::resizeEvent(e);
  ScrollTo(current_index_, false);
}

void PrettyImageView::wheelEvent(QWheelEvent* e) {
  const int d = e->delta() > 0 ? -1 : 1;
  ScrollTo(current_index_ + d, true);
}
