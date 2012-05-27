/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "moodbarproxystyle.h"
#include "core/logging.h"

#include <QEvent>
#include <QPainter>
#include <QSlider>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QTimeLine>

const int MoodbarProxyStyle::kMarginSize = 3;
const int MoodbarProxyStyle::kBorderSize = 1;
const int MoodbarProxyStyle::kArrowWidth = 17;
const int MoodbarProxyStyle::kArrowHeight = 13;

MoodbarProxyStyle::MoodbarProxyStyle(QSlider* slider)
  : QProxyStyle(slider->style()),
    slider_(slider),
    enabled_(true),
    moodbar_style_(MoodbarRenderer::Style_Normal),
    state_(MoodbarOff),
    fade_timeline_(new QTimeLine(1000, this)),
    moodbar_colors_dirty_(true),
    moodbar_pixmap_dirty_(true)
{
  slider->setStyle(this);
  slider->installEventFilter(this);

  connect(fade_timeline_, SIGNAL(valueChanged(qreal)), SLOT(FaderValueChanged(qreal)));
}

void MoodbarProxyStyle::SetMoodbarData(const QByteArray& data) {
  data_ = data;
  moodbar_colors_dirty_ = true; // Redraw next time
  NextState();
}

void MoodbarProxyStyle::SetMoodbarEnabled(bool enabled) {
  enabled_ = enabled;
  NextState();
}

void MoodbarProxyStyle::NextState() {
  const bool visible = enabled_ && !data_.isEmpty();

  if ((visible  && (state_ == MoodbarOn  || state_ == FadingToOn)) ||
      (!visible && (state_ == MoodbarOff || state_ == FadingToOff))) {
    return;
  }

  const QTimeLine::Direction direction =
      visible ? QTimeLine::Forward : QTimeLine::Backward;

  if (state_ == MoodbarOn || state_ == MoodbarOff) {
    // Start the fade from the beginning.
    fade_timeline_->setDirection(direction);
    fade_timeline_->start();

    fade_source_ = QPixmap();
    fade_target_ = QPixmap();
  } else {
    // Stop an existing fade and start fading the other direction from the
    // same place.
    fade_timeline_->stop();
    fade_timeline_->setDirection(direction);
    fade_timeline_->resume();
  }

  state_ = visible ? FadingToOn : FadingToOff;
}

void MoodbarProxyStyle::FaderValueChanged(qreal value) {
  slider_->update();
}

bool MoodbarProxyStyle::eventFilter(QObject* object, QEvent* event) {
  if (object == slider_ && event->type() == QEvent::Resize) {
    // The widget was resized, we've got to render a new pixmap.
    moodbar_pixmap_dirty_ = true;
  }

  return QProxyStyle::eventFilter(object, event);
}

void MoodbarProxyStyle::drawComplexControl(
    ComplexControl control, const QStyleOptionComplex* option,
    QPainter* painter, const QWidget* widget) const {
  if (control != CC_Slider || widget != slider_) {
    QProxyStyle::drawComplexControl(control, option, painter, widget);
    return;
  }

  const_cast<MoodbarProxyStyle*>(this)->Render(
        control, qstyleoption_cast<const QStyleOptionSlider*>(option),
        painter, widget);
}

void MoodbarProxyStyle::Render(
    ComplexControl control, const QStyleOptionSlider* option,
    QPainter* painter, const QWidget* widget) {

  const qreal fade_value = fade_timeline_->currentValue();

  // Have we finished fading?
  if (state_ == FadingToOn && fade_value == 1.0) {
    state_ = MoodbarOn;
  } else if (state_ == FadingToOff && fade_value == 0.0) {
    state_ = MoodbarOff;
  }


  switch (state_) {
  case FadingToOn:
  case FadingToOff:
    // Update the cached pixmaps if necessary
    if (fade_source_.isNull()) {
      // Draw the normal slider into the fade source pixmap.
      fade_source_ = QPixmap(option->rect.size());
      fade_source_.fill(option->palette.color(QPalette::Active, QPalette::Background));

      QPainter p(&fade_source_);
      QStyleOptionSlider opt_copy(*option);
      opt_copy.rect.moveTo(0, 0);

      QProxyStyle::drawComplexControl(control, &opt_copy, &p, widget);

      p.end();
    }

    if (fade_target_.isNull()) {
      if (state_ == FadingToOn) {
        EnsureMoodbarRendered();
      }
      fade_target_ = moodbar_pixmap_;
    }

    // Blend the pixmaps into each other
    painter->drawPixmap(option->rect, fade_source_);
    painter->setOpacity(fade_value);
    painter->drawPixmap(option->rect, fade_target_);
    painter->setOpacity(1.0);
    break;

  case MoodbarOff:
    // It's a normal slider widget.
    QProxyStyle::drawComplexControl(control, option, painter, widget);
    break;

  case MoodbarOn:
    EnsureMoodbarRendered();
    painter->drawPixmap(option->rect, moodbar_pixmap_);
    DrawArrow(option, painter);
    break;
  }
}

void MoodbarProxyStyle::EnsureMoodbarRendered() {
  if (moodbar_colors_dirty_) {
    moodbar_colors_ = MoodbarRenderer::Colors(data_, moodbar_style_, slider_->palette());
    moodbar_colors_dirty_ = false;
    moodbar_pixmap_dirty_ = true;
  }

  if (moodbar_pixmap_dirty_) {
    moodbar_pixmap_ = MoodbarPixmap(moodbar_colors_, slider_->size(), slider_->palette());
    moodbar_pixmap_dirty_ = false;
  }
}

QRect MoodbarProxyStyle::subControlRect(
    ComplexControl cc, const QStyleOptionComplex* opt,
    SubControl sc, const QWidget* widget) const {
  if (cc != QStyle::CC_Slider || widget != slider_) {
    return QProxyStyle::subControlRect(cc, opt, sc, widget);
  }

  switch (state_) {
    case MoodbarOff:
    case FadingToOff:
      break;

    case MoodbarOn:
    case FadingToOn:
      switch (sc) {
        case SC_SliderGroove:
          return opt->rect.adjusted(kMarginSize, kMarginSize,
                                    -kMarginSize, -kMarginSize);

        case SC_SliderHandle: {
          const QStyleOptionSlider* slider_opt =
              qstyleoption_cast<const QStyleOptionSlider*>(opt);

          const int x =
              (slider_opt->sliderValue - slider_opt->minimum) * (opt->rect.width() - kArrowWidth) /
              (slider_opt->maximum - slider_opt->minimum);

          return QRect(QPoint(opt->rect.left() + x, opt->rect.top()),
                       QSize(kArrowWidth, kArrowHeight));
        }

        default:
          break;
      }
  }

  return QProxyStyle::subControlRect(cc, opt, sc, widget);
}

void MoodbarProxyStyle::DrawArrow(const QStyleOptionSlider* option,
                                  QPainter* painter) const {
  // Get the dimensions of the arrow
  const QRect rect = subControlRect(CC_Slider, option, SC_SliderHandle, slider_);

  // Make a polygon
  QPolygon poly;
  poly << rect.topLeft()
       << rect.topRight()
       << QPoint(rect.center().x(), rect.bottom());

  // Draw it
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->translate(0.5, 0.5);
  painter->setPen(Qt::black);
  painter->setBrush(slider_->palette().brush(QPalette::Active, QPalette::Base));
  painter->drawPolygon(poly);
  painter->restore();
}

QPixmap MoodbarProxyStyle::MoodbarPixmap(const ColorVector& colors,
                                         const QSize& size, const QPalette& palette) {
  QRect rect(QPoint(0, 0), size);
  QRect border_rect(rect);
  border_rect.adjust(kMarginSize, kMarginSize, -kMarginSize, -kMarginSize);

  QRect inner_rect(border_rect);
  inner_rect.adjust(kBorderSize, kBorderSize, -kBorderSize, -kBorderSize);

  QPixmap ret(size);
  QPainter p(&ret);

  // Draw the moodbar
  MoodbarRenderer::Render(colors, &p, inner_rect);

  // Draw the border
  p.setPen(QPen(Qt::black,
                kBorderSize, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
  p.drawRect(border_rect.adjusted(0, 0, -1, -1));

  // Draw the outer bit
  p.setPen(QPen(palette.brush(QPalette::Active, QPalette::Background),
                kMarginSize, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
  p.drawRect(rect.adjusted(1, 1, -2, -2));

  p.end();

  return ret;
}
