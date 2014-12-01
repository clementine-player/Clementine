/* This file is part of Clementine.
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011-2012, 2014, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "nyancatanalyzer.h"

#include <cmath>

#include <QTimerEvent>
#include <QBrush>

#include "core/arraysize.h"
#include "core/logging.h"

using Analyzer::Scope;

const char* NyanCatAnalyzer::kName = "Nyanalyzer cat";
const float NyanCatAnalyzer::kPixelScale = 0.02f;

NyanCatAnalyzer::NyanCatAnalyzer(QWidget* parent)
    : Analyzer::Base(parent, 9),
      cat_(":/nyancat.png"),
      timer_id_(startTimer(kFrameIntervalMs)),
      frame_(0),
      current_buffer_(0),
      available_rainbow_width_(0),
      px_per_frame_(0),
      x_offset_(0),
      background_brush_(QColor(0x0f, 0x43, 0x73)) {
  memset(history_, 0, sizeof(history_));

  for (int i = 0; i < kRainbowBands; ++i) {
    colors_[i] = QPen(QColor::fromHsv(i * 255 / kRainbowBands, 255, 255),
                      kCatHeight / kRainbowBands, Qt::SolidLine, Qt::FlatCap,
                      Qt::RoundJoin);

    // pow constants computed so that
    // | band_scale(0) | ~= .5 and | band_scale(5) | ~= 32
    band_scale_[i] =
        -std::cos(M_PI * i / (kRainbowBands - 1)) * 0.5 * std::pow(2.3, i);
  }
}

void NyanCatAnalyzer::transform(Scope& s) { m_fht->spectrum(&s.front()); }

void NyanCatAnalyzer::timerEvent(QTimerEvent* e) {
  if (e->timerId() == timer_id_) {
    frame_ = (frame_ + 1) % kCatFrameCount;
  } else {
    Analyzer::Base::timerEvent(e);
  }
}

void NyanCatAnalyzer::resizeEvent(QResizeEvent* e) {
  // Invalidate the buffer so it's recreated from scratch in the next paint
  // event.
  buffer_[0] = QPixmap();
  buffer_[1] = QPixmap();

  available_rainbow_width_ = width() - kCatWidth + kRainbowOverlap;
  px_per_frame_ = static_cast<float>(available_rainbow_width_) / (kHistorySize - 1) + 1;
  x_offset_ = px_per_frame_ * (kHistorySize - 1) - available_rainbow_width_;
}

void NyanCatAnalyzer::analyze(QPainter& p, const Analyzer::Scope& s,
                              bool new_frame) {
  // Discard the second half of the transform
  const int scope_size = s.size() / 2;

  if ((new_frame && is_playing_) ||
      (buffer_[0].isNull() && buffer_[1].isNull())) {
    // Transform the music into rainbows!
    for (int band = 0; band < kRainbowBands; ++band) {
      float* band_start = history_ + band * kHistorySize;

      // Move the history of each band across by 1 frame.
      memmove(band_start, band_start + 1, (kHistorySize - 1) * sizeof(float));
    }

    // Now accumulate the scope data into each band.  Should maybe use a series
    // of band pass filters for this, so bands can leak into neighbouring bands,
    // but for now it's a series of separate square filters.
    const int samples_per_band = scope_size / kRainbowBands;
    int sample = 0;
    for (int band = 0; band < kRainbowBands; ++band) {
      float accumulator = 0.0;
      for (int i = 0; i < samples_per_band; ++i) {
        accumulator += s[sample++];
      }

      history_[(band + 1) * kHistorySize - 1] = accumulator * band_scale_[band];
    }

    // Create polylines for the rainbows.
    QPointF polyline[kRainbowBands * kHistorySize];
    QPointF* dest = polyline;
    float* source = history_;

    const float top_of_cat = static_cast<float>(height()) / 2 - static_cast<float>(kCatHeight) / 2;
    for (int band = 0; band < kRainbowBands; ++band) {
      // Calculate the Y position of this band.
      const float y =
          static_cast<float>(kCatHeight) / (kRainbowBands + 1) * (band + 0.5) + top_of_cat;

      // Add each point in the line.
      for (int x = 0; x < kHistorySize; ++x) {
        *dest = QPointF(px_per_frame_ * x, y + *source * kPixelScale);
        ++dest;
        ++source;
      }
    }

    // Do we have to draw the whole rainbow into the buffer?
    if (buffer_[0].isNull()) {
      for (int i = 0; i < 2; ++i) {
        buffer_[i] = QPixmap(QSize(width() + x_offset_, height()));
        buffer_[i].fill(background_brush_.color());
      }
      current_buffer_ = 0;

      QPainter buffer_painter(&buffer_[0]);
      buffer_painter.setRenderHint(QPainter::Antialiasing);
      for (int band = kRainbowBands - 1; band >= 0; --band) {
        buffer_painter.setPen(colors_[band]);
        buffer_painter.drawPolyline(&polyline[band * kHistorySize],
                                    kHistorySize);
        buffer_painter.drawPolyline(&polyline[band * kHistorySize],
                                    kHistorySize);
      }
    } else {
      const int last_buffer = current_buffer_;
      current_buffer_ = (current_buffer_ + 1) % 2;

      // We can just shuffle the buffer along a bit and draw the new frame's
      // data.
      QPainter buffer_painter(&buffer_[current_buffer_]);
      buffer_painter.setRenderHint(QPainter::Antialiasing);

      buffer_painter.drawPixmap(
          0, 0, buffer_[last_buffer], px_per_frame_, 0,
          x_offset_ + available_rainbow_width_ - px_per_frame_, 0);
      buffer_painter.fillRect(
          x_offset_ + available_rainbow_width_ - px_per_frame_, 0,
          kCatWidth - kRainbowOverlap + px_per_frame_, height(),
          background_brush_);

      for (int band = kRainbowBands - 1; band >= 0; --band) {
        buffer_painter.setPen(colors_[band]);
        buffer_painter.drawPolyline(&polyline[(band + 1) * kHistorySize - 3],
                                    3);
      }
    }
  }

  // Draw the buffer on to the widget
  p.drawPixmap(0, 0, buffer_[current_buffer_], x_offset_, 0, 0, 0);

  // Draw nyan cat (he's been waiting for this for 75 lines).
  // Nyan nyan nyan nyan.
  if (!is_playing_) {
    // Ssshhh!
    p.drawPixmap(SleepingCatDestRect(), cat_, SleepingCatSourceRect());
  } else {
    p.drawPixmap(CatDestRect(), cat_, CatSourceRect());
  }
}
