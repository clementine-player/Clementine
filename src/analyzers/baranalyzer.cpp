/* This file is part of Clementine.
   Copyright 2003, Mark Kretschmann <markey@web.de>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Alibek Omarov <a1ba.omarov@gmail.com>
   Copyright 2014, Mark Furneaux <mark@romaco.ca>
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
  m_bg = parent->palette().color(QPalette::Background);

  QColor fg(parent->palette().color(QPalette::Highlight).lighter(150));

  double dr = static_cast<double>(m_bg.red() - fg.red()) /
              (NUM_ROOFS - 1);  // -1 because we start loop below at 0
  double dg = static_cast<double>(m_bg.green() - fg.green()) / (NUM_ROOFS - 1);
  double db = static_cast<double>(m_bg.blue() - fg.blue()) / (NUM_ROOFS - 1);

  for (uint i = 0; i < NUM_ROOFS; ++i) {
    m_pixRoof[i] = QPixmap(COLUMN_WIDTH, 1);
    m_pixRoof[i].fill(QColor(fg.red() + static_cast<int>(dr * i), fg.green() + static_cast<int>(dg * i),
                             fg.blue() + static_cast<int>(db * i)));
  }
}

void BarAnalyzer::resizeEvent(QResizeEvent* e) { init(); }

// METHODS =====================================================

void BarAnalyzer::init() {
  const double MAX_AMPLITUDE = 1.0;
  const double F = static_cast<double>(height() - 2) / (log10(255) * MAX_AMPLITUDE);

  BAND_COUNT = width() / 5;
  MAX_DOWN = static_cast<int>(0 - (qMax(1, height() / 50)));
  MAX_UP = static_cast<int>(qMax(1, height() / 25));

  barVector.resize(BAND_COUNT, 0);
  roofVector.resize(BAND_COUNT, height() - 5);
  roofVelocityVector.resize(BAND_COUNT, ROOF_VELOCITY_REDUCTION_FACTOR);
  m_roofMem.resize(BAND_COUNT);
  m_scope.resize(BAND_COUNT);

  // generate a list of values that express amplitudes in range 0-MAX_AMP as
  // ints from 0-height() on log scale
  for (uint x = 0; x < 256; ++x) {
    m_lvlMapper[x] = static_cast<uint>(F * log10(x + 1));
  }

  m_pixBarGradient = QPixmap(height() * COLUMN_WIDTH, height());
  m_pixCompose = QPixmap(size());
  canvas_ = QPixmap(size());
  canvas_.fill(palette().color(QPalette::Background));

  QPainter p(&m_pixBarGradient);
  QColor rgb(palette().color(QPalette::Highlight));

  for (int x = 0, r = rgb.red(), g = rgb.green(), b = rgb.blue(), r2 = 255 - r; x < height();
       ++x) {
    for (int y = x; y > 0; --y) {
      const double fraction = static_cast<double>(y) / height();

      //          p.setPen( QColor( r + (int)(r2 * fraction), g, b - (int)(255 *
      // fraction) ) );
      p.setPen(QColor(r + static_cast<int>(r2 * fraction), g, b));
      p.drawLine(x * COLUMN_WIDTH, height() - y, (x + 1) * COLUMN_WIDTH,
                 height() - y);
    }
  }

  setMinimumSize(QSize(BAND_COUNT * COLUMN_WIDTH, 10));
}

void BarAnalyzer::analyze(QPainter& p, const Scope& s, bool new_frame) {
  if (!new_frame) {
    p.drawPixmap(0, 0, canvas_);
    return;
  }
  // Analyzer::interpolate( s, m_bands );

  Scope& v = m_scope;
  Analyzer::interpolate(s, v);
  QPainter canvas_painter(&canvas_);

  canvas_.fill(palette().color(QPalette::Background));

  for (uint i = 0, x = 0, y2; i < v.size(); ++i, x += COLUMN_WIDTH + 1) {
    // assign pre[log10]'d value
    y2 = static_cast<uint>(v[i] *
                           256);  // 256 will be optimised to a bitshift //no, it's a float
    y2 = m_lvlMapper[(y2 > 255) ? 255 : y2];  // lvlMapper is array of ints with
                                              // values 0 to height()

    int change = y2 - barVector[i];

    // using the best of Markey's, piggz and Max's ideas on the way to shift the
    // bars
    // we have the following:
    // 1. don't adjust shift when doing small up movements
    // 2. shift large upwards with a bias towards last value
    // 3. fall downwards at a constant pace

    /*if ( change > MAX_UP ) //anything too much greater than 2 gives "jitter"
           //add some dynamics - makes the value slightly closer to what it was last time
           y2 = ( barVector[i] + MAX_UP );
           //y2 = ( barVector[i] * 2 + y2 ) / 3;
        else*/ if (change <
                                                                                   MAX_DOWN)
      y2 = barVector[i] + MAX_DOWN;

    if (static_cast<int>(y2) > roofVector[i]) {
      roofVector[i] = static_cast<int>(y2);
      roofVelocityVector[i] = 1;
    }

    // remember where we are
    barVector[i] = y2;

    if (m_roofMem[i].size() > NUM_ROOFS)
      m_roofMem[i].erase(m_roofMem[i].begin());

    // blt last n roofs, a.k.a motion blur
    for (uint c = 0; c < m_roofMem[i].size(); ++c)
      // bitBlt( m_pComposePixmap, x, m_roofMem[i]->at( c ), m_roofPixmaps[ c ]
      // );
      // bitBlt( canvas(), x, m_roofMem[i][c], &m_pixRoof[ NUM_ROOFS - 1 - c ]
      // );
      canvas_painter.drawPixmap(x, m_roofMem[i][c],
                                m_pixRoof[NUM_ROOFS - 1 - c]);

    // blt the bar
    canvas_painter.drawPixmap(x, height() - y2, *gradient(), y2 * COLUMN_WIDTH,
                              height() - y2, COLUMN_WIDTH, y2);
    /*bitBlt( canvas(), x, height() - y2,
            gradient(), y2 * COLUMN_WIDTH, height() - y2, COLUMN_WIDTH, y2,
       Qt::CopyROP );*/

    m_roofMem[i].push_back(height() - roofVector[i] - 2);

    // set roof parameters for the NEXT draw
    if (roofVelocityVector[i] != 0) {
      if (roofVelocityVector[i] > 32)  // no reason to do == 32
        roofVector[i] -=
            (roofVelocityVector[i] - 32) / 20;  // trivial calculation

      if (roofVector[i] < 0) {
        roofVector[i] = 0;  // not strictly necessary
        roofVelocityVector[i] = 0;
      } else {
        ++roofVelocityVector[i];
      }
    }
  }

  p.drawPixmap(0, 0, canvas_);
}
