/* This file is part of Clementine.
   Copyright 2003, Max Howell <max.howell@methylblue.com>
   Copyright 2009, 2011-2012, David Sansome <me@davidsansome.com>
   Copyright 2010, 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
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

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>

#include "core/arraysize.h"

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

// TODO(John Maguire): for 2D use setErasePixmap Qt function insetead of
// m_background

// make the linker happy only for gcc < 4.0
#if !(__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 0)) && \
    !defined(Q_OS_WIN32)
template class Analyzer::Base<QWidget>;
#endif

static const int sBarkBands[] = {
    100,  200,  300,  400,  510,  630,  770,  920,  1080, 1270, 1480,  1720,
    2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500};

static const int sBarkBandCount = arraysize(sBarkBands);

Analyzer::Base::Base(QWidget* parent, uint scopeSize)
    : QWidget(parent),
      timeout_(40),  // msec
      fht_(new FHT(scopeSize)),
      engine_(nullptr),
      lastScope_(512),
      new_frame_(false),
      is_playing_(false),
      barkband_table_(),
      prev_color_index_(0),
      bands_(0),
      psychedelic_enabled_(false) {}

void Analyzer::Base::hideEvent(QHideEvent*) { timer_.stop(); }

void Analyzer::Base::showEvent(QShowEvent*) { timer_.start(timeout(), this); }

void Analyzer::Base::transform(Scope& scope) {
  // this is a standard transformation that should give
  // an FFT scope that has bands for pretty analyzers

  // NOTE resizing here is redundant as FHT routines only calculate FHT::size()
  // values
  // scope.resize( fht_->size() );

  QVector<float> aux(fht_->size());
  if (aux.size() >= scope.size()) {
    std::copy(scope.begin(), scope.end(), aux.begin());
  } else {
    std::copy(scope.begin(), scope.begin() + aux.size(), aux.begin());
  }

  fht_->logSpectrum(scope.data(), aux.data());
  fht_->scale(scope.data(), 1.0 / 20);

  scope.resize(fht_->size() / 2);  // second half of values are rubbish
}

void Analyzer::Base::paintEvent(QPaintEvent* e) {
  QPainter p(this);
  p.fillRect(e->rect(), palette().color(QPalette::Window));

  switch (engine_->state()) {
    case Engine::Playing: {
      const Engine::Scope& thescope = engine_->scope(timeout_);
      int i = 0;

      // convert to mono here - our built in analyzers need mono, but the
      // engines provide interleaved pcm
      for (uint x = 0; static_cast<int>(x) < fht_->size(); ++x) {
        lastScope_[x] = static_cast<double>(thescope[i] + thescope[i + 1]) /
                        (2 * (1 << 15));
        i += 2;
      }

      is_playing_ = true;
      transform(lastScope_);
      analyze(p, lastScope_, new_frame_);

      lastScope_.resize(fht_->size());

      break;
    }
    case Engine::Paused:
      is_playing_ = false;
      analyze(p, lastScope_, new_frame_);
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

  if (exp != fht_->sizeExp()) {
    delete fht_;
    fht_ = new FHT(exp);
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
  return fht_->size() / 2;
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

void Analyzer::Base::psychedelicModeChanged(bool enabled) {
  psychedelic_enabled_ = enabled;
}

int Analyzer::Base::BandFrequency(int band) const {
  return ((kSampleRate / 2) * band + kSampleRate / 4) / bands_;
}

void Analyzer::Base::updateBandSize(const int scopeSize) {
  // prevent possible dbz in BandFrequency
  if (scopeSize == 0) {
    return;
  }

  bands_ = scopeSize;

  barkband_table_.clear();

  int barkband = 0;
  for (int i = 0; i < bands_ + 1; ++i) {
    if (barkband < sBarkBandCount - 1 &&
        BandFrequency(i) >= sBarkBands[barkband]) {
      barkband++;
    }

    barkband_table_.append(barkband);
  }
}

QColor Analyzer::Base::getPsychedelicColor(const Scope& scope,
                                           const int ampFactor,
                                           const int bias) {
  if (scope.size() > barkband_table_.size()) {
    return palette().color(QPalette::Highlight);
  }

  // Calculate total magnitudes for different bark bands.
  double bands[sBarkBandCount]{};

  for (int i = 0; i < barkband_table_.size(); ++i) {
    bands[barkband_table_[i]] += scope[i];
  }

  // Now divide the bark bands into thirds and compute their total amplitudes.
  double rgb[3]{};
  for (int i = 0; i < sBarkBandCount - 1; ++i) {
    rgb[(i * 3) / sBarkBandCount] += pow(bands[i], 2);
  }

  for (int i = 0; i < 3; ++i) {
    // bias colours for a threshold around normally amplified audio
    rgb[i] = qMin(255, (int)((sqrt(rgb[i]) * ampFactor) + bias));
  }

  return QColor::fromRgb(rgb[0], rgb[1], rgb[2]);
}

void Analyzer::Base::polishEvent() {
  init();  // virtual
}

void Analyzer::interpolate(const Scope& inVec, Scope& outVec) {
  double pos = 0.0;
  const double step = static_cast<double>(inVec.size()) / outVec.size();

  for (uint i = 0; i < outVec.size(); ++i, pos += step) {
    const double error = pos - std::floor(pos);
    const uint64_t offset = static_cast<uint64_t>(pos);

    uint64_t indexLeft = offset + 0;

    if (indexLeft >= inVec.size()) indexLeft = inVec.size() - 1;

    uint64_t indexRight = offset + 1;

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
  if (e->timerId() != timer_.timerId()) return;

  new_frame_ = true;
  update();
}
