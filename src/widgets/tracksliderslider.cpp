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
#include "core/utilities.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QtDebug>

TrackSliderSlider::TrackSliderSlider(QWidget* parent)
  : QSlider(parent),
    popup_(new TrackSliderPopup(this))
{
  setMouseTracking(true);
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

  QMouseEvent new_event(e->type(), e->pos(), new_button, new_button, e->modifiers());
  QSlider::mousePressEvent(&new_event);

  if (new_event.isAccepted())
    e->accept();
}

void TrackSliderSlider::mouseReleaseEvent(QMouseEvent* e) {
  QSlider::mouseReleaseEvent(e);
}

void TrackSliderSlider::mouseMoveEvent(QMouseEvent* e) {
  QSlider::mouseMoveEvent(e);

  // Borrowed from QSliderPrivate::pixelPosToRangeValue
  QStyleOptionSlider opt;
  initStyleOption(&opt);
  QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
  QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

  int slider_length = sr.width();
  int slider_min = gr.x();
  int slider_max = gr.right() - slider_length + 1;

  int seconds = QStyle::sliderValueFromPosition(
      minimum(), maximum(), e->x() - slider_length/2 - slider_min + 1,
      slider_max - slider_min);

  popup_->SetText(Utilities::PrettyTime(seconds));
  popup_->SetPopupPosition(mapToGlobal(QPoint(
      e->x(), rect().center().y())));
}

void TrackSliderSlider::enterEvent(QEvent* e) {
  QSlider::enterEvent(e);
  popup_->SetMouseOverSlider(true);
}

void TrackSliderSlider::leaveEvent(QEvent* e) {
  QSlider::leaveEvent(e);
  popup_->SetMouseOverSlider(false);
}
