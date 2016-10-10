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

#include "tracksliderpopup.h"
#include "tracksliderslider.h"
#include "core/timeconstants.h"
#include "core/utilities.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QtDebug>
#include <QWheelEvent>

TrackSliderSlider::TrackSliderSlider(QWidget* parent)
    : QSlider(parent),
      popup_(new TrackSliderPopup(window())),
      mouse_hover_seconds_(0) {
  setMouseTracking(true);

  popup_->hide();
  connect(this, SIGNAL(valueChanged(int)), SLOT(UpdateDeltaTime()));
}

void TrackSliderSlider::mousePressEvent(QMouseEvent* e) {
  // QSlider asks QStyle which mouse button should do what (absolute move or
  // page step).  We force our own behaviour here because it makes more sense
  // for a music player IMO.

  Qt::MouseButton new_button = e->button();
  if (e->button() == Qt::LeftButton) {
    int abs_buttons = style()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons);
    if (abs_buttons & Qt::LeftButton)
      new_button = Qt::LeftButton;
    else if (abs_buttons & Qt::MidButton)
      new_button = Qt::MidButton;
    else if (abs_buttons & Qt::RightButton)
      new_button = Qt::RightButton;
  }

  QMouseEvent new_event(e->type(), e->pos(), new_button, new_button,
                        e->modifiers());
  QSlider::mousePressEvent(&new_event);

  if (new_event.isAccepted()) e->accept();
}

void TrackSliderSlider::mouseReleaseEvent(QMouseEvent* e) {
  QSlider::mouseReleaseEvent(e);
}

void TrackSliderSlider::mouseMoveEvent(QMouseEvent* e) {
  QSlider::mouseMoveEvent(e);

  // Borrowed from QSliderPrivate::pixelPosToRangeValue
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt,
                                     QStyle::SC_SliderGroove, this);
  QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt,
                                     QStyle::SC_SliderHandle, this);

  int slider_length = sr.width();
  int slider_min = gr.x();
  int slider_max = gr.right() - slider_length + 1;

  mouse_hover_seconds_ = QStyle::sliderValueFromPosition(
      minimum() / kMsecPerSec, maximum() / kMsecPerSec,
      e->x() - slider_length / 2 - slider_min + 1, slider_max - slider_min);

  popup_->SetText(Utilities::PrettyTime(mouse_hover_seconds_));
  UpdateDeltaTime();
  popup_->SetPopupPosition(
      mapTo(window(), QPoint(e->x(), rect().center().y())));
}

void TrackSliderSlider::wheelEvent(QWheelEvent *e) {
  if (e->delta() < 0) {
    emit SeekBackward();
  } else {
    emit SeekForward();
  }
  e->accept();
}

void TrackSliderSlider::enterEvent(QEvent* e) {
  QSlider::enterEvent(e);
  if (isEnabled()) {
    popup_->show();
  }
}

void TrackSliderSlider::leaveEvent(QEvent* e) {
  QSlider::leaveEvent(e);
  popup_->hide();
}

void TrackSliderSlider::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Down) {
    emit SeekBackward();
    event->accept();
  } else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Up) {
    emit SeekForward();
    event->accept();
  } else {
    QSlider::keyPressEvent(event);
  }
}

void TrackSliderSlider::UpdateDeltaTime() {
  if (popup_->isVisible()) {
    int delta_seconds = mouse_hover_seconds_ - (value() / kMsecPerSec);
    popup_->SetSmallText(Utilities::PrettyTimeDelta(delta_seconds));
  }
}
