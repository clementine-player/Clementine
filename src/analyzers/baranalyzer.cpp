/* This file is part of Clementine.
   Copyright 2003, Mark Kretschmann <markey@web.de>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Alibek Omarov <a1ba.omarov@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

/* Original Author:  Mark Kretschmann  <markey@web.de>  2003
*/

#include "baranalyzer.h"
#include <cmath>
#include <QtDebug>
#include <QPainter>

using Analyzer::Scope;

const char* BarAnalyzer::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Bar analyzer");

BarAnalyzer::BarAnalyzer(QWidget* parent) : Analyzer::Base(parent, 8) {
  // roof pixmaps don't depend on size() so we do in the ctor
  bg_ = parent->palette().color(QPalette::Background);

  QColor fg(parent->palette().color(QPalette::Highlight).lighter(150));

  double dr = static_cast<double>(bg_.red() - fg.red()) /
              (kNumRoofs - 1);  // -1 because we start loop below at 0
  double dg = static_cast<double>(bg_.green() - fg.green()) / (kNumRoofs - 1);
  double db = static_cast<double>(bg_.blue() - fg.blue()) / (kNumRoofs - 1);

  for (uint i = 0; i < kNumRoofs; ++i) {
    pixRoof_[i] = QPixmap(kColumnWidth, 1);
    pixRoof_[i].fill(QColor(fg.red() + static_cast<int>(dr * i),
                            fg.green() + static_cast<int>(dg * i),
                            fg.blue() + static_cast<int>(db * i)));
  }
}

void BarAnalyzer::resizeEvent(QResizeEvent* e) { init(); }

// METHODS =====================================================

void BarAnalyzer::init() {
  const double MAX_AMPLITUDE = 1.0;
  const double F =
      static_cast<double>(height() - 2) / (log10(255) * MAX_AMPLITUDE);

  band_count_ = width() / 5;
  max_down_ = static_cast<int>(0 - (qMax(1, height() / 50)));
  max_up_ = static_cast<int>(qMax(1, height() / 25));

  barVector_.resize(band_count_, 0);
  roofVector_.resize(band_count_, height() - 5);
  roofVelocityVector_.resize(band_count_, kRoofVelocityReductionFactor);
  roofMem_.resize(band_count_);
  scope_.resize(band_count_);

  // generate a list of values that express amplitudes in range 0-MAX_AMP as
  // ints from 0-height() on log scale
  for (uint x = 0; x < 256; ++x) {
    lvlMapper_[x] = static_cast<uint>(F * log10(x + 1));
  }

  pixBarGradient_ = QPixmap(height() * kColumnWidth, height());
  pixCompose_ = QPixmap(size());
  canvas_ = QPixmap(size());
  canvas_.fill(palette().color(QPalette::Background));

  updateBandSize(band_count_);
  colorChanged();
  setMinimumSize(QSize(band_count_ * kColumnWidth, 10));
}

void BarAnalyzer::colorChanged() {
  if (pixBarGradient_.isNull()) {
    return;
  }

  QPainter p(&pixBarGradient_);
  QColor rgb;
  if (psychedelic_enabled_) {
    rgb = getPsychedelicColor(scope_, 50, 100);
  } else {
    rgb = palette().color(QPalette::Highlight);
  }

  for (int x = 0, r = rgb.red(), g = rgb.green(), b = rgb.blue(), r2 = 255 - r;
       x < height(); ++x) {
    for (int y = x; y > 0; --y) {
      const double fraction = static_cast<double>(y) / height();

      //          p.setPen( QColor( r + (int)(r2 * fraction), g, b - (int)(255 *
      // fraction) ) );
      p.setPen(QColor(r + static_cast<int>(r2 * fraction), g, b));
      p.drawLine(x * kColumnWidth, height() - y, (x + 1) * kColumnWidth,
                 height() - y);
    }
  }
}

void BarAnalyzer::psychedelicModeChanged(bool enabled) {
  psychedelic_enabled_ = enabled;
  // reset colours back to normal
  colorChanged();
}

void BarAnalyzer::analyze(QPainter& p, const Scope& s, bool new_frame) {
  if (!new_frame  || engine_->state() == Engine::Paused) {
    p.drawPixmap(0, 0, canvas_);
    return;
  }
  // Analyzer::interpolate( s, m_bands );

  Analyzer::interpolate(s, scope_);
  QPainter canvas_painter(&canvas_);

  // update the graphics with the new colour
  if (psychedelic_enabled_) {
    colorChanged();
  }

  canvas_.fill(palette().color(QPalette::Background));

  for (uint i = 0, x = 0, y2; i < scope_.size(); ++i, x += kColumnWidth + 1) {
    // assign pre[log10]'d value
    y2 = static_cast<uint>(
        scope_[i] *
        256);  // 256 will be optimised to a bitshift //no, it's a float
    y2 = lvlMapper_[(y2 > 255) ? 255 : y2];  // lvlMapper is array of ints with
                                             // values 0 to height()

    int change = y2 - barVector_[i];

    // using the best of Markey's, piggz and Max's ideas on the way to shift the
    // bars
    // we have the following:
    // 1. don't adjust shift when doing small up movements
    // 2. shift large upwards with a bias towards last value
    // 3. fall downwards at a constant pace

    /*if ( change > max_up_ ) //anything too much greater than 2 gives "jitter"
           //add some dynamics - makes the value slightly closer to what it was last time
           y2 = ( barVector_[i] + max_up_ );
           //y2 = ( barVector_[i] * 2 + y2 ) / 3;
        else*/ if (change <
                                                                                    max_down_)
      y2 = barVector_[i] + max_down_;

    if (static_cast<int>(y2) > roofVector_[i]) {
      roofVector_[i] = static_cast<int>(y2);
      roofVelocityVector_[i] = 1;
    }

    // remember where we are
    barVector_[i] = y2;

    if (roofMem_[i].size() > kNumRoofs) roofMem_[i].erase(roofMem_[i].begin());

    // blt last n roofs, a.k.a motion blur
    for (uint c = 0; c < roofMem_[i].size(); ++c)
      // bitBlt( m_pComposePixmap, x, roofMem_[i]->at( c ), m_roofPixmaps[ c ]
      // );
      // bitBlt( canvas(), x, roofMem_[i][c], &pixRoof_[ kNumRoofs - 1 - c ]
      // );
      canvas_painter.drawPixmap(x, roofMem_[i][c], pixRoof_[kNumRoofs - 1 - c]);

    // blt the bar
    canvas_painter.drawPixmap(x, height() - y2, *gradient(), y2 * kColumnWidth,
                              height() - y2, kColumnWidth, y2);
    /*bitBlt( canvas(), x, height() - y2,
            gradient(), y2 * kColumnWidth, height() - y2, kColumnWidth, y2,
       Qt::CopyROP );*/

    roofMem_[i].push_back(height() - roofVector_[i] - 2);

    // set roof parameters for the NEXT draw
    if (roofVelocityVector_[i] != 0) {
      if (roofVelocityVector_[i] > 32)  // no reason to do == 32
        roofVector_[i] -=
            (roofVelocityVector_[i] - 32) / 20;  // trivial calculation

      if (roofVector_[i] < 0) {
        roofVector_[i] = 0;  // not strictly necessary
        roofVelocityVector_[i] = 0;
      } else {
        ++roofVelocityVector_[i];
      }
    }
  }

  p.drawPixmap(0, 0, canvas_);
}
