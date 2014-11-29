/* This file is part of Clementine.
   Copyright 2003, Max Howell <max.howell@methylblue.com>
   Copyright 2009, 2011-2012, David Sansome <me@davidsansome.com>
   Copyright 2010, 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Mark Furneaux <mark@romaco.ca>
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

/* Original Author:  Max Howell  <max.howell@methylblue.com>  2003
*/

#include "analyzerbase.h"

#include <cmath>

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>

#include "engines/enginebase.h"

// INSTRUCTIONS Base2D
// 1. do anything that depends on height() in init(), Base2D will call it before
// you are shown
// 2. otherwise you can use the constructor to initialise things
// 3. reimplement analyze(), and paint to canvas(), Base2D will update the
// widget when you return control to it
// 4. if you want to manipulate the scope, reimplement transform()
// 5. for convenience <vector> <qpixmap.h> <qwdiget.h> are pre-included
// TODO(David Sansome): make an INSTRUCTIONS file
// can't mod scope in analyze you have to use transform

// TODO(John Maguire): for 2D use setErasePixmap Qt function insetead of m_background

// make the linker happy only for gcc < 4.0
#if !(__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 0)) && \
    !defined(Q_OS_WIN32)
template class Analyzer::Base<QWidget>;
#endif

Analyzer::Base::Base(QWidget* parent, uint scopeSize)
    : QWidget(parent),
      m_timeout(40)  // msec
      ,
      m_fht(new FHT(scopeSize)),
      m_engine(nullptr),
      m_lastScope(512),
      current_chunk_(0),
      new_frame_(false),
      is_playing_(false) {}

void Analyzer::Base::hideEvent(QHideEvent*) { m_timer.stop(); }

void Analyzer::Base::showEvent(QShowEvent*) { m_timer.start(timeout(), this); }

void Analyzer::Base::transform(Scope& scope) {
  // this is a standard transformation that should give
  // an FFT scope that has bands for pretty analyzers

  // NOTE resizing here is redundant as FHT routines only calculate FHT::size()
  // values
  // scope.resize( m_fht->size() );

  float* front = static_cast<float*>(&scope.front());

  float* f = new float[m_fht->size()];
  m_fht->copy(&f[0], front);
  m_fht->logSpectrum(front, &f[0]);
  m_fht->scale(front, 1.0 / 20);

  scope.resize(m_fht->size() / 2);  // second half of values are rubbish
  delete[] f;
}

void Analyzer::Base::paintEvent(QPaintEvent* e) {
  QPainter p(this);
  p.fillRect(e->rect(), palette().color(QPalette::Window));

  switch (m_engine->state()) {
    case Engine::Playing: {
      const Engine::Scope& thescope = m_engine->scope(m_timeout);
      int i = 0;

      // convert to mono here - our built in analyzers need mono, but the
      // engines provide interleaved pcm
      for (uint x = 0; static_cast<int>(x) < m_fht->size(); ++x) {
        m_lastScope[x] =
            static_cast<double>(thescope[i] + thescope[i + 1]) / (2 * (1 << 15));
        i += 2;
      }

      is_playing_ = true;
      transform(m_lastScope);
      analyze(p, m_lastScope, new_frame_);

      // scope.resize( m_fht->size() );

      break;
    }
    case Engine::Paused:
      is_playing_ = false;
      analyze(p, m_lastScope, new_frame_);
      break;

    default:
      is_playing_ = false;
      demo(p);
  }

  new_frame_ = false;
}

int Analyzer::Base::resizeExponent(int exp) {
  if (exp < 3)
    exp = 3;
  else if (exp > 9)
    exp = 9;

  if (exp != m_fht->sizeExp()) {
    delete m_fht;
    m_fht = new FHT(exp);
  }
  return exp;
}

int Analyzer::Base::resizeForBands(int bands) {
  int exp;
  if (bands <= 8)
    exp = 4;
  else if (bands <= 16)
    exp = 5;
  else if (bands <= 32)
    exp = 6;
  else if (bands <= 64)
    exp = 7;
  else if (bands <= 128)
    exp = 8;
  else
    exp = 9;

  resizeExponent(exp);
  return m_fht->size() / 2;
}

void Analyzer::Base::demo(QPainter& p) {
  static int t = 201;  // FIXME make static to namespace perhaps

  if (t > 999) t = 1;  // 0 = wasted calculations
  if (t < 201) {
    Scope s(32);

    const double dt = static_cast<double>(t) / 200;
    for (uint i = 0; i < s.size(); ++i)
      s[i] = dt * (sin(M_PI + (i * M_PI) / s.size()) + 1.0);

    analyze(p, s, new_frame_);
  } else {
    analyze(p, Scope(32, 0), new_frame_);
  }
  ++t;
}

void Analyzer::Base::polishEvent() {
  init();  // virtual
}

void Analyzer::interpolate(const Scope& inVec, Scope& outVec) {
  double pos = 0.0;
  const double step = static_cast<double>(inVec.size()) / outVec.size();

  for (uint i = 0; i < outVec.size(); ++i, pos += step) {
    const double error = pos - std::floor(pos);
    const unsigned int64 offset = static_cast<unsigned int64>(pos);

    unsigned int64 indexLeft = offset + 0;

    if (indexLeft >= inVec.size()) indexLeft = inVec.size() - 1;

    unsigned int64 indexRight = offset + 1;

    if (indexRight >= inVec.size()) indexRight = inVec.size() - 1;

    outVec[i] = inVec[indexLeft] * (1.0 - error) + inVec[indexRight] * error;
  }
}

void Analyzer::initSin(Scope& v, const uint size) {
  double step = (M_PI * 2) / size;
  double radian = 0;

  for (uint i = 0; i < size; i++) {
    v.push_back(sin(radian));
    radian += step;
  }
}

void Analyzer::Base::timerEvent(QTimerEvent* e) {
  QWidget::timerEvent(e);
  if (e->timerId() != m_timer.timerId()) return;

  new_frame_ = true;
  update();
}
