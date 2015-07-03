/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2005, Mark Kretschmann <markey@web.de>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
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

/* Original Author:  Max Howell  <max.howell@methylblue.com>  2003-2005
 * Original Author:  Mark Kretschmann  <markey@web.de>  2005
 */

#include "blockanalyzer.h"

#include <cmath>

#include <QMouseEvent>
#include <QResizeEvent>
#include <cstdlib>
#include <QPainter>

const uint BlockAnalyzer::kHeight = 2;
const uint BlockAnalyzer::kWidth = 4;
const uint BlockAnalyzer::kMinRows = 3;       // arbituary
const uint BlockAnalyzer::kMinColumns = 32;   // arbituary
const uint BlockAnalyzer::kMaxColumns = 256;  // must be 2**n
const uint BlockAnalyzer::kFadeSize = 90;

const char* BlockAnalyzer::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Block analyzer");

BlockAnalyzer::BlockAnalyzer(QWidget* parent)
    : Analyzer::Base(parent, 9),
      columns_(0),
      rows_(0),
      y_(0),
      barPixmap_(1, 1),
      topBarPixmap_(kWidth, kHeight),
      scope_(kMinColumns),
      store_(1 << 8, 0),
      fade_bars_(kFadeSize),
      fade_pos_(1 << 8, 50),
      fade_intensity_(1 << 8, 32) {
  setMinimumSize(kMinColumns * (kWidth + 1) - 1, kMinRows * (kHeight + 1) - 1);
  // -1 is padding, no drawing takes place there
  setMaximumWidth(kMaxColumns * (kWidth + 1) - 1);

  // mxcl says null pixmaps cause crashes, so let's play it safe
  for (uint i = 0; i < kFadeSize; ++i) fade_bars_[i] = QPixmap(1, 1);
}

BlockAnalyzer::~BlockAnalyzer() {}

void BlockAnalyzer::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);

  background_ = QPixmap(size());
  canvas_ = QPixmap(size());

  const uint oldRows = rows_;

  // all is explained in analyze()..
  // +1 to counter -1 in maxSizes, trust me we need this!
  columns_ = qMin(
      static_cast<uint>(static_cast<double>(width() + 1) / (kWidth + 1)) + 1,
      kMaxColumns);
  rows_ = static_cast<uint>(static_cast<double>(height() + 1) / (kHeight + 1));

  // this is the y-offset for drawing from the top of the widget
  y_ = (height() - (rows_ * (kHeight + 1)) + 2) / 2;

  scope_.resize(columns_);

  if (rows_ != oldRows) {
    barPixmap_ = QPixmap(kWidth, rows_ * (kHeight + 1));

    for (uint i = 0; i < kFadeSize; ++i)
      fade_bars_[i] = QPixmap(kWidth, rows_ * (kHeight + 1));

    yscale_.resize(rows_ + 1);

    const uint PRE = 1,
               PRO = 1;  // PRE and PRO allow us to restrict the range somewhat

    for (uint z = 0; z < rows_; ++z)
      yscale_[z] = 1 - (log10(PRE + z) / log10(PRE + rows_ + PRO));

    yscale_[rows_] = 0;

    determineStep();
    paletteChange(palette());
  }

  updateBandSize(columns_);
  drawBackground();
}

void BlockAnalyzer::determineStep() {
  // falltime is dependent on rowcount due to our digital resolution (ie we have
  // boxes/blocks of pixels)
  // I calculated the value 30 based on some trial and error

  // the fall time of 30 is too slow on framerates above 50fps
  const double fallTime = timeout() < 20 ? 20 * rows_ : 30 * rows_;

  step_ = static_cast<double>(rows_ * timeout()) / fallTime;
}

void BlockAnalyzer::framerateChanged() {  // virtual
  determineStep();
}

void BlockAnalyzer::transform(Analyzer::Scope& s) {
  for (uint x = 0; x < s.size(); ++x) s[x] *= 2;

  float* front = static_cast<float*>(&s.front());

  fht_->spectrum(front);
  fht_->scale(front, 1.0 / 20);

  // the second half is pretty dull, so only show it if the user has a large
  // analyzer
  // by setting to scope_.size() if large we prevent interpolation of large
  // analyzers, this is good!
  s.resize(scope_.size() <= kMaxColumns / 2 ? kMaxColumns / 2 : scope_.size());
}

void BlockAnalyzer::analyze(QPainter& p, const Analyzer::Scope& s,
                            bool new_frame) {
  // y = 2 3 2 1 0 2
  //     . . . . # .
  //     . . . # # .
  //     # . # # # #
  //     # # # # # #
  //
  // visual aid for how this analyzer works.
  // y represents the number of blanks
  // y starts from the top and increases in units of blocks

  // yscale_ looks similar to: { 0.7, 0.5, 0.25, 0.15, 0.1, 0 }
  // if it contains 6 elements there are 5 rows in the analyzer

  if (!new_frame) {
    p.drawPixmap(0, 0, canvas_);
    return;
  }

  QPainter canvas_painter(&canvas_);

  Analyzer::interpolate(s, scope_);

  // update the graphics with the new colour
  if (psychedelic_enabled_) {
    paletteChange(QPalette());
  }

  // Paint the background
  canvas_painter.drawPixmap(0, 0, background_);

  for (uint y, x = 0; x < scope_.size(); ++x) {
    // determine y
    for (y = 0; scope_[x] < yscale_[y]; ++y) continue;

    // this is opposite to what you'd think, higher than y
    // means the bar is lower than y (physically)
    if (static_cast<float>(y) > store_[x])
      y = static_cast<int>(store_[x] += step_);
    else
      store_[x] = y;

    // if y is lower than fade_pos_, then the bar has exceeded the kHeight of
    // the fadeout
    // if the fadeout is quite faded now, then display the new one
    if (y <= fade_pos_[x] /*|| fade_intensity_[x] < kFadeSize / 3*/) {
      fade_pos_[x] = y;
      fade_intensity_[x] = kFadeSize;
    }

    if (fade_intensity_[x] > 0) {
      const uint offset = --fade_intensity_[x];
      const uint y = y_ + (fade_pos_[x] * (kHeight + 1));
      canvas_painter.drawPixmap(x * (kWidth + 1), y, fade_bars_[offset], 0, 0,
                                kWidth, height() - y);
    }

    if (fade_intensity_[x] == 0) fade_pos_[x] = rows_;

    // REMEMBER: y is a number from 0 to rows_, 0 means all blocks are glowing,
    // rows_ means none are
    canvas_painter.drawPixmap(x * (kWidth + 1), y * (kHeight + 1) + y_, *bar(),
                              0, y * (kHeight + 1), bar()->width(),
                              bar()->height());
  }

  for (uint x = 0; x < store_.size(); ++x)
    canvas_painter.drawPixmap(x * (kWidth + 1),
                              static_cast<int>(store_[x]) * (kHeight + 1) + y_,
                              topBarPixmap_);

  p.drawPixmap(0, 0, canvas_);
}

static inline void adjustToLimits(int& b, int& f, uint& amount) {
  // with a range of 0-255 and maximum adjustment of amount,
  // maximise the difference between f and b

  if (b < f) {
    if (b > 255 - f) {
      amount -= f;
      f = 0;
    } else {
      amount -= (255 - f);
      f = 255;
    }
  } else {
    if (f > 255 - b) {
      amount -= f;
      f = 0;
    } else {
      amount -= (255 - f);
      f = 255;
    }
  }
}

void BlockAnalyzer::psychedelicModeChanged(bool enabled) {
  psychedelic_enabled_ = enabled;
  // reset colours back to normal
  paletteChange(QPalette());
}

/**
 * Clever contrast function
 *
 * It will try to adjust the foreground color such that it contrasts well with
 *the background
 * It won't modify the hue of fg unless absolutely necessary
 * @return the adjusted form of fg
 */
QColor ensureContrast(const QColor& bg, const QColor& fg, uint _amount = 150) {
  class OutputOnExit {
   public:
    explicit OutputOnExit(const QColor& color) : c(color) {}
    ~OutputOnExit() {
      int h, s, v;
      c.getHsv(&h, &s, &v);
    }

   private:
    const QColor& c;
  };

  OutputOnExit allocateOnTheStack(fg);

  int bh, bs, bv;
  int fh, fs, fv;

  bg.getHsv(&bh, &bs, &bv);
  fg.getHsv(&fh, &fs, &fv);

  int dv = abs(bv - fv);

  // value is the best measure of contrast
  // if there is enough difference in value already, return fg unchanged
  if (dv > static_cast<int>(_amount)) return fg;

  int ds = abs(bs - fs);

  // saturation is good enough too. But not as good. TODO adapt this a little
  if (ds > static_cast<int>(_amount)) return fg;

  int dh = abs(bh - fh);

  if (dh > 120) {
    // a third of the colour wheel automatically guarentees contrast
    // but only if the values are high enough and saturations significant enough
    // to allow the colours to be visible and not be shades of grey or black

    // check the saturation for the two colours is sufficient that hue alone can
    // provide sufficient contrast
    if (ds > static_cast<int>(_amount) / 2 && (bs > 125 && fs > 125))
      return fg;
    else if (dv > static_cast<int>(_amount) / 2 && (bv > 125 && fv > 125))
      return fg;
  }

  if (fs < 50 && ds < 40) {
    // low saturation on a low saturation is sad
    const int tmp = 50 - fs;
    fs = 50;
    if (static_cast<int>(_amount) > tmp)
      _amount -= tmp;
    else
      _amount = 0;
  }

  // test that there is available value to honor our contrast requirement
  if (255 - dv < static_cast<int>(_amount)) {
    // we have to modify the value and saturation of fg
    // adjustToLimits( bv, fv, amount );
    // see if we need to adjust the saturation
    if (static_cast<int>(_amount) > 0) adjustToLimits(bs, fs, _amount);

    // see if we need to adjust the hue
    if (static_cast<int>(_amount) > 0)
      fh += static_cast<int>(_amount);  // cycles around;

    return QColor::fromHsv(fh, fs, fv);
  }

  if (fv > bv && bv > static_cast<int>(_amount))
    return QColor::fromHsv(fh, fs, bv - static_cast<int>(_amount));

  if (fv < bv && fv > static_cast<int>(_amount))
    return QColor::fromHsv(fh, fs, fv - static_cast<int>(_amount));

  if (fv > bv && (255 - fv > static_cast<int>(_amount)))
    return QColor::fromHsv(fh, fs, fv + static_cast<int>(_amount));

  if (fv < bv && (255 - bv > static_cast<int>(_amount)))
    return QColor::fromHsv(fh, fs, bv + static_cast<int>(_amount));

  return Qt::blue;
}

void BlockAnalyzer::paletteChange(const QPalette&) {
  const QColor bg = palette().color(QPalette::Background);
  QColor fg;

  if (psychedelic_enabled_) {
    fg = getPsychedelicColor(scope_, 10, 75);
  } else {
    fg = ensureContrast(bg, palette().color(QPalette::Highlight));
  }

  topBarPixmap_.fill(fg);

  const double dr =
      15 * static_cast<double>(bg.red() - fg.red()) / (rows_ * 16);
  const double dg =
      15 * static_cast<double>(bg.green() - fg.green()) / (rows_ * 16);
  const double db =
      15 * static_cast<double>(bg.blue() - fg.blue()) / (rows_ * 16);
  const int r = fg.red(), g = fg.green(), b = fg.blue();

  bar()->fill(bg);

  QPainter p(bar());

  for (int y = 0; static_cast<uint>(y) < rows_; ++y)
    // graduate the fg color
    p.fillRect(
        0, y * (kHeight + 1), kWidth, kHeight,
        QColor(r + static_cast<int>(dr * y), g + static_cast<int>(dg * y),
               b + static_cast<int>(db * y)));

  {
    const QColor bg = palette().color(QPalette::Background).dark(112);

    // make a complimentary fadebar colour
    // TODO(John Maguire): dark is not always correct, dumbo!
    int h, s, v;
    palette().color(QPalette::Background).dark(150).getHsv(&h, &s, &v);
    const QColor fg(QColor::fromHsv(h + 120, s, v));

    const double dr = fg.red() - bg.red();
    const double dg = fg.green() - bg.green();
    const double db = fg.blue() - bg.blue();
    const int r = bg.red(), g = bg.green(), b = bg.blue();

    // Precalculate all fade-bar pixmaps
    for (uint y = 0; y < kFadeSize; ++y) {
      fade_bars_[y].fill(palette().color(QPalette::Background));
      QPainter f(&fade_bars_[y]);
      for (int z = 0; static_cast<uint>(z) < rows_; ++z) {
        const double Y = 1.0 - (log10(kFadeSize - y) / log10(kFadeSize));
        f.fillRect(
            0, z * (kHeight + 1), kWidth, kHeight,
            QColor(r + static_cast<int>(dr * Y), g + static_cast<int>(dg * Y),
                   b + static_cast<int>(db * Y)));
      }
    }
  }

  drawBackground();
}

void BlockAnalyzer::drawBackground() {
  const QColor bg = palette().color(QPalette::Background);
  const QColor bgdark = bg.dark(112);

  background_.fill(bg);

  QPainter p(&background_);

  if (p.paintEngine() == 0) {
    return;
  }

  for (int x = 0; (uint)x < columns_; ++x)
    for (int y = 0; (uint)y < rows_; ++y)
      p.fillRect(x * (kWidth + 1), y * (kHeight + 1) + y_, kWidth, kHeight,
                 bgdark);
}
