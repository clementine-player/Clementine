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

const int MoodbarProxyStyle::kNumHues = 12;

MoodbarProxyStyle::MoodbarProxyStyle(QSlider* slider)
  : QProxyStyle(slider->style()),
    slider_(slider),
    enabled_(true),
    moodbar_style_(Style_SystemDefault),
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
      fade_source_.fill(option->palette.color(QPalette::Background));

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
    break;
  }
}

void MoodbarProxyStyle::EnsureMoodbarRendered() {
  if (moodbar_colors_dirty_) {
    moodbar_colors_ = MoodbarColors(data_, moodbar_style_, slider_->palette());
    moodbar_colors_dirty_ = false;
    moodbar_pixmap_dirty_ = true;
  }

  if (moodbar_pixmap_dirty_) {
    moodbar_pixmap_ = MoodbarPixmap(moodbar_colors_, slider_->size());
    moodbar_pixmap_dirty_ = false;
  }
}

MoodbarProxyStyle::ColorList MoodbarProxyStyle::MoodbarColors(
    const QByteArray& data, MoodbarStyle style, const QPalette& palette) {
  const int samples = data.size() / 3;

  // Set some parameters based on the moodbar style
  StyleProperties properties;
  switch(style) {
    case Style_Angry:  properties = StyleProperties(samples / 360 * 9, 45,  -45, 200, 100); break;
    case Style_Frozen: properties = StyleProperties(samples / 360 * 1, 140, 160, 50,  100); break;
    case Style_Happy:  properties = StyleProperties(samples / 360 * 2, 0,   359, 150, 250); break;
    case Style_SystemDefault:
    default: {
      const QColor highlight_color(palette.color(QPalette::Highlight));

      properties.threshold_   = samples / 360 * 3;
      properties.range_start_ = (highlight_color.hsvHue() - 20 + 360) % 360;
      properties.range_delta_ = 20;
      properties.sat_         = highlight_color.hsvSaturation();
      properties.val_         = highlight_color.value() / 2;
    }
  }

  const unsigned char* data_p =
      reinterpret_cast<const unsigned char*>(data.constData());

  int hue_distribution[360];
  int total = 0;

  memset(hue_distribution, 0, sizeof(hue_distribution));

  ColorList colors;

  // Read the colors, keeping track of some histograms
  for (int i=0; i<samples; ++i) {
    QColor color;
    color.setRed(int(*data_p++));
    color.setGreen(int(*data_p++));
    color.setBlue(int(*data_p++));

    colors << color;

    const int hue = qMax(0, color.hue());
    if (hue_distribution[hue]++ == properties.threshold_) {
      total ++;
    }
  }

  // Remap the hue values to be between rangeStart and
  // rangeStart + rangeDelta.  Every time we see an input hue
  // above the threshold, increment the output hue by
  // (1/total) * rangeDelta.
  for (int i=0, n=0 ; i<360; i++) {
    hue_distribution[i] =
        ((hue_distribution[i] > properties.threshold_ ? n++ : n )
          * properties.range_delta_ / total + properties.range_start_) % 360;
  }

  // Now huedist is a hue mapper: huedist[h] is the new hue value
  // for a bar with hue h
  for (ColorList::iterator it = colors.begin() ; it != colors.end() ; ++it) {
    const int hue = qMax(0, it->hue());

    *it = QColor::fromHsv(
          qBound(0, hue_distribution[hue], 359),
          qBound(0, it->saturation() * properties.sat_ / 100, 255),
          qBound(0, it->value() * properties.val_ / 100, 255));
  }

  return colors;
}

QPixmap MoodbarProxyStyle::MoodbarPixmap(const ColorList& colors, const QSize& size) {
  // Sample the colors and map them to screen pixels.
  ColorList screen_colors;
  for (int x=0; x<size.width(); ++x) {
    int r = 0;
    int g = 0;
    int b = 0;

    uint start = x       * colors.size() / size.width();
    uint end   = (x + 1) * colors.size() / size.width();

    if (start == end)
      end = start + 1;

    for (uint j=start; j<end; j++) {
      r += colors[j].red();
      g += colors[j].green();
      b += colors[j].blue();
    }

    const uint n = end - start;
    screen_colors.append(QColor(r/n, g/n, b/n));
  }

  QPixmap ret(size);
  QPainter p(&ret);

  for (int x=0; x<size.width(); x++) {
    int h, s, v;
    screen_colors[x].getHsv( &h, &s, &v );

    for (int y=0; y<=size.height()/2; y++) {
      float coeff = float(y) / float(size.height()/2);
      float coeff2 = 1.0f - ((1.0f - coeff) * (1.0f - coeff));
      coeff = 1.0f - (1.0f - coeff) / 2.0f;
      coeff2 = 1.f - (1.f - coeff2) / 2.0f;

      p.setPen(QColor::fromHsv(
          h,
          qBound(0, int(float(s) * coeff), 255),
          qBound(0, int(255.f - (255.f - float(v)) * coeff2), 255)));

      p.drawPoint(x, y);
      p.drawPoint(x, size.height() - 1 - y);
    }
  }

  p.end();

  return ret;
}
