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

#include "moodbarrenderer.h"

#include <QPainter>
#include <QPalette>

const int MoodbarRenderer::kNumHues = 12;

ColorVector MoodbarRenderer::Colors(
    const QByteArray& data, MoodbarStyle style, const QPalette& palette) {
  const int samples = data.size() / 3;

  // Set some parameters based on the moodbar style
  StyleProperties properties;
  switch(style) {
    case Style_Angry:  properties = StyleProperties(samples / 360 * 9, 45,  -45, 200, 100); break;
    case Style_Frozen: properties = StyleProperties(samples / 360 * 1, 140, 160, 50,  100); break;
    case Style_Happy:  properties = StyleProperties(samples / 360 * 2, 0,   359, 150, 250); break;
    case Style_Normal: properties = StyleProperties(samples / 360 * 3, 0,   359, 100, 100); break;
    case Style_SystemPalette:
    default: {
      const QColor highlight_color(palette.color(QPalette::Active, QPalette::Highlight));

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

  ColorVector colors;

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

  total = qMax(total, 1);

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
  for (ColorVector::iterator it = colors.begin() ; it != colors.end() ; ++it) {
    const int hue = qMax(0, it->hue());

    *it = QColor::fromHsv(
          qBound(0, hue_distribution[hue], 359),
          qBound(0, it->saturation() * properties.sat_ / 100, 255),
          qBound(0, it->value() * properties.val_ / 100, 255));
  }

  return colors;
}

void MoodbarRenderer::Render(const ColorVector& colors, QPainter* p, const QRect& rect) {
  // Sample the colors and map them to screen pixels.
  ColorVector screen_colors;
  for (int x=0; x<rect.width(); ++x) {
    int r = 0;
    int g = 0;
    int b = 0;

    int start = x       * colors.size() / rect.width();
    int end   = (x + 1) * colors.size() / rect.width();

    if (start == end)
      end = qMin(start + 1, colors.size() - 1);

    for (int j=start; j<end; j++) {
      r += colors[j].red();
      g += colors[j].green();
      b += colors[j].blue();
    }

    const int n = qMax(1, end - start);
    screen_colors.append(QColor(r/n, g/n, b/n));
  }

  // Draw the actual moodbar.
  for (int x=0; x<rect.width(); x++) {
    int h, s, v;
    screen_colors[x].getHsv( &h, &s, &v );

    for (int y=0; y<=rect.height()/2; y++) {
      float coeff = float(y) / float(rect.height()/2);
      float coeff2 = 1.0f - ((1.0f - coeff) * (1.0f - coeff));
      coeff = 1.0f - (1.0f - coeff) / 2.0f;
      coeff2 = 1.f - (1.f - coeff2) / 2.0f;

      p->setPen(QColor::fromHsv(
          h,
          qBound(0, int(float(s) * coeff), 255),
          qBound(0, int(255.f - (255.f - float(v)) * coeff2), 255)));

      p->drawPoint(rect.left() + x, rect.top() + y);
      p->drawPoint(rect.left() + x, rect.top() + rect.height() - 1 - y);
    }
  }
}

QImage MoodbarRenderer::RenderToImage(const ColorVector& colors, const QSize& size) {
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  QPainter p(&image);
  Render(colors, &p, image.rect());
  p.end();
  return image;
}

QString MoodbarRenderer::StyleName(MoodbarStyle style) {
  switch (style) {
    case Style_Normal:        return QObject::tr("Normal");
    case Style_Angry:         return QObject::tr("Angry");
    case Style_Frozen:        return QObject::tr("Frozen");
    case Style_Happy:         return QObject::tr("Happy");
    case Style_SystemPalette: return QObject::tr("System colors");

    default:                  return QString();
  }
}
