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

#include "nyancatanalyzer.h"
#include "core/logging.h"

#include <cmath>

#include <QTimerEvent>

const char* NyanCatAnalyzer::kName = "Nyan nyan nyan";


NyanCatAnalyzer::NyanCatAnalyzer(QWidget* parent)
  : Analyzer::Base(parent, 9),
    cat_(":/nyancat.png"),
    timer_id_(startTimer(kFrameIntervalMs)),
    frame_(0)
{
  memset(history_, 0, sizeof(history_));

  for (int i=0 ; i<kRainbowBands ; ++i) {
    colors_[i] = QPen(QColor::fromHsv(i * 255 / kRainbowBands, 255, 255), 4.5);
  }
}

void NyanCatAnalyzer::transform(Scope& s) {
  m_fht->spectrum(&s.front());
}

void NyanCatAnalyzer::timerEvent(QTimerEvent* e) {
  if (e->timerId() == timer_id_) {
    frame_ = (frame_ + 1) % kCatFrameCount;
  } else {
    Analyzer::Base::timerEvent(e);
  }
}

void NyanCatAnalyzer::analyze(QPainter& p, const Analyzer::Scope& s) {
  // Discard the second half of the transform
  const int scope_size = s.size() / 2;

  // Transform the music into rainbows!
  for (int band=0 ; band<kRainbowBands ; ++band) {
    float* band_start = history_ + band * kHistorySize;

    // Move the history of each band across by 1 frame.
    memmove(band_start, band_start + 1, (kHistorySize - 1) * sizeof(float));

    // And set the new frame to 0.
    band_start[kHistorySize-1] = 0;
  }

  // Now accumulate the scope data into each band.  Should maybe use a series
  // of band pass filters for this, so bands can leak into neighbouring bands,
  // but for now it's a series of separate square filters.
  const int samples_per_band = scope_size / kRainbowBands;
  int sample = 0;
  for (int band=0 ; band<kRainbowBands ; ++band) {
    float* accumulator = &history_[(band+1) * kHistorySize - 1];
    for (int i=0 ; i<samples_per_band ; ++i) {
      *accumulator += s[sample++];
    }
  }

  // Create polylines for the rainbows.
  const float px_per_frame = float(width()) / kHistorySize;
  QPointF polyline[kRainbowBands * kHistorySize];
  QPointF* dest = polyline;
  float* source = history_;

  for (int band=0 ; band<kRainbowBands ; ++band) {
    // Calculate the Y position of this band.
    const float y = float(height()) / (kRainbowBands + 2) * (band + 0.5);
    const float band_scale = std::pow(2, band);

    // Add each point in the line.
    for (int x=0 ; x<kHistorySize ; ++x) {
      *dest = QPointF(px_per_frame * x, y + *source * kPixelScale * band_scale);
      ++ dest;
      ++ source;
    }
  }

  // Draw the rainbows
  p.setRenderHint(QPainter::Antialiasing);
  for (int band=kRainbowBands-1 ; band>=0 ; --band) {
    p.setPen(colors_[band]);
    p.drawPolyline(&polyline[band*kHistorySize], kHistorySize);
  }

  // Draw nyan cat (he's been waiting for this for 50 lines).
  // Nyan nyan nyan nyan.
  QRect cat_dest(width() - kCatWidth, (height() - kCatHeight) / 2,
                 kCatWidth, kCatHeight);
  p.drawPixmap(cat_dest, cat_, CatSourceRect());
}
