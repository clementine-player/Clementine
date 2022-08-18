/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2005, Mark Kretschmann <markey@web.de>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2022, Andrew Reading <andrew@areading.me>

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

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <cmath>
#include <cstdlib>

const uint BlockAnalyzer::kHeight = 2;
const uint BlockAnalyzer::kWidth = 4;
const uint BlockAnalyzer::kMinRows = 3;       // arbitrary
const uint BlockAnalyzer::kMaxRows = 256;     // arbitrary
const uint BlockAnalyzer::kMinColumns = 32;   // arbitrary
const uint BlockAnalyzer::kMaxColumns = 256;  // must be 2**n
const uint BlockAnalyzer::kFadeSize = 90;
const uint BlockAnalyzer::kFadeInitial = 32;

const char* BlockAnalyzer::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Block analyzer");

BlockAnalyzer::BlockAnalyzer(QWidget* parent)
    : Analyzer::Base(parent, 9),
      scope_(kMinColumns),
      columns_(0),
      rows_(0),
      y_(0),
      canvas_(),
      rthresh_(kMaxRows + 1, 0.f),
      bg_grad_(kMaxRows + 1, 0),
      fade_bars_(kFadeSize, 0),
      bandinfo_(kMaxColumns) {
  // Right and bottom edges are 1px padding.
  setMinimumSize(kMinColumns * (kWidth + 1) - 1, kMinRows * (kHeight + 1) - 1);
  setMaximumWidth(kMaxColumns * (kWidth + 1) - 1);

  setAttribute(Qt::WA_OpaquePaintEvent, true);
}

BlockAnalyzer::~BlockAnalyzer() {}

void BlockAnalyzer::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);

  uint newRows, newCols;

  // all is explained in analyze()..
  // +1 to counter -1 in maxSizes, trust me we need this!
  newCols = 1 + (width() + 1) / (kWidth + 1);
  newRows = 0 + (height() + 1) / (kHeight + 1);
  newCols = qMin(kMaxColumns, qMax(kMinColumns, newCols));
  newRows = qMin(kMaxRows, qMax(kMinRows, newRows));

  if (newCols != columns_) {
    columns_ = newCols;
    scope_.resize(columns_);

    updateBandSize(columns_);
    bandinfo_.fill(FHTBand());
  }

  if (rows_ != newRows) {
    rows_ = newRows;

    // this is the y-offset for drawing from the top of the widget
    y_ = (height() - (rows_ * (kHeight + 1)) + 2) / 2;

    const float PRE = 1.f,
                PRO =
                    1.f,  // PRE and PRO allow us to restrict the range somewhat
        SCL = log10f(PRE + PRO + (1.f * rows_));

    for (uint z = 0; z < rows_; ++z)
      rthresh_[z] = 1.f - log10f(PRE + (1.f * z)) / SCL;

    rthresh_[rows_] = 0.f;

    determineStep();
    paletteChange(palette());
  }

  canvas_ = QImage(columns_ * (kWidth + 1), rows_ * (kHeight + 1),
                   QImage::Format_ARGB32_Premultiplied);
  canvas_.fill(pad_color_);
}

void BlockAnalyzer::determineStep() {
  // falltime is dependent on rowcount
  // the fall time of 30 is too slow on framerates above 50fps
  const float rFallTime = 1.f / (timeout() < 20 ? 20.f : 30.f);
  step_ = timeout() * rFallTime;
}

void BlockAnalyzer::framerateChanged() {  // virtual
  determineStep();
}

void BlockAnalyzer::transform(Analyzer::Scope& s) {
  for (uint x = 0; x < s.size(); ++x) s[x] *= 2.f;

  fht_->spectrum(s.data());
  fht_->scale(s.data(), 1.f / 20.f);

  // the second half is pretty dull, so only show it if the user has a large
  // analyzer
  // by setting to scope_.size() if large we prevent interpolation of large
  // analyzers, this is good!
  s.resize(scope_.size() <= kMaxColumns / 2 ? kMaxColumns / 2 : scope_.size());
}

void BlockAnalyzer::analyze(QPainter& p, const Analyzer::Scope& s,
                            bool new_frame) {
  float yf;
  uint x, y;

  if (p.paintEngine() == 0) return;
  if (canvas_.isNull()) return;

  p.setCompositionMode(QPainter::CompositionMode_Source);

  if (!new_frame) {
    p.drawImage(0, 0, canvas_, 0, 0, width(), height(), Qt::NoFormatConversion);
    return;
  }

  Analyzer::interpolate(s, scope_);

  // Update the color palettes.
  if (psychedelic_enabled_) paletteChange(QPalette());

  // Visual Aid
  //
  // This analyzer maintains a list of intensity thresholds for each row of
  // the analyzer. For each frequency band (represented column-wise, one per
  // band), the spectral power calculation obtained from the analyzer scope
  // output is compared against these thresholds to determine the row indices
  // at which the regions become active. While inactive regions are dark,
  // active regions and all those below the corresponding transition region
  // are "lit up".
  //
  // So, where
  //     . indicates block is inactive/dark
  //     # indicates block is active  /lit,
  // what is drawn is (for example)
  //
  //    COLUMNS/Bands
  //     . . . . # .    R
  //     . . . # # .    O
  //     # . # # # #    W
  //     # # # # # #    S
  //
  // y = 2 3 2 1 0 2
  //
  // Here y is the row index for which the intensity threshold is met, with
  // 0 indicating the topmost row. The nRows+1 intensity values are stored
  // in rthresh_[], sorted in decreasing order (the top, y=0 region would
  // be the most spectrally intense); the additional, final value is always
  // zero and exists mostly as a sort of loop optimization.
  //
  // For the above illustration, rthresh_[] might have values similar to
  //     { 0.7, 0.5, 0.25, 0.15, 0.1, 0 }
  //
  // Now, consider two "frames" that occur sequentially after each other. Where
  //     . indicates block is inactive/dark
  //     o indicates block is inactive/dark and fading out (was active)
  //     # indicates block is active  /lit,
  //   [ ] indicates block is the bar topper
  //
  //         frame 1        ====>        frame 2
  //      COLUMNS/Bands               COLUMNS/Bands
  //     .  .  .  . [#] .     R     .  .  .  .  o  .
  //     .  .  . [#] #  .     O     .  . [#] o [#] .
  //    [#] . [#] #  # [#]    W     o  .  #  o  # [#]
  //     # [#] #  #  #  #     S    [#] o  # [#] #  #
  //
  //     2  3  2  1  0  2  = B_y =  3  4  1  1  1  2
  //
  // After a previously active region becomes inactive, for a period of time
  // it is drawn in a color that darkens over time. These are based upon the
  // the current color scheme and get stored within fade_bars_[].
  // Additionally, a rowwise gradient is applied to active bands to help keep
  // the spectrum display visually interesting, with colors darkening as
  // intensities decrease -- that is, as rthresh_[] values decrease. The
  // inactive-active transition area is drawn with the brightest color and
  // acts as a "bar topper"; this topper should visually rise and fall over
  // time.
  //
  // As in the transition example above, bands (columns) are drawn vertically
  // from top to bottom, progressing from left to right. Supposing Y_r is the
  // row coordinate, B_y is the band coordinate, and
  //   0 <= Y_r,B_y < nRows <= kMaxRows,
  // the drawing procedure for each band can be described as follows:
  //   a. Y_r < B_y
  //        First the '.' regions that have not been recently active are
  //        darkened (background). This is determined via the band's
  //        fade_intensity and fade_row values.
  //   b. Y_r < B_y
  //        Recently active areas are drawn using a special darkening-fade
  //        color, until either some number of frames have elapsed or they
  //        became active since the countdown began.
  //   c. Y_r = B_y
  //        The transition region is drawn as a bar topper.
  //   d. Y_r > B_y < nRows
  //        Each subsequent region below the transition region should be active.
  //        Draw these using a gradient that darkens as Y_r -> nRows.
  // The logic for these can be found in the colorFromRowAndBand() function.
  //

  // Update band information.
  for (x = 0; x < scope_.size(); ++x) {
    const float& bandthr = scope_[x];
    FHTBand& band = bandinfo_[x];

    // Calculate activity transition row values.
    //   Note:   rows_ < rthresh_.size()
    for (y = 0; y < rows_; ++y) {
      if (bandthr >= rthresh_[y]) break;
    }

    // y <= band height :: band matches or exceeds power from last frame.
    // y >  band height :: band lost power since last frame.
    if ((yf = 1.f * y) <= band.height) {
      band.height = yf;
      band.row = y;
    } else {
      // This band has lost power since the last-recorded maximal threshold
      // value. Gradually decrease this until it meets the current value.
      band.height += step_;
      band.row = y = static_cast<uint>(band.height);
    }

    // y <= band fade_row :: the current threshold exceeds the previously-
    // marked position in which to begin fade-out. Use the current position
    // as a new marker and start/restart fade_intensity, the fade-out period
    // counter.
    if (y <= band.fade_row) {
      band.fade_row = y;
      band.fade_intensity = kFadeSize;
    }

    // Check the fade-out period counter. If expired (i.e., <= 0), the
    // fade-out effect is complete. Otherwise, continue downcounting and
    // select the next color for the fade-out sequence.
    if (band.fade_intensity <= 0) {
      // fade_intensity <= 0: Done with fade out effect (time expired).
      band.fade_row = rows_;
      band.fade_coloridx = 0;
    } else {
      // fade_intensity >  0: Continue effect; continue color change.
      band.fade_coloridx = --band.fade_intensity;
    }
  }

  // A block will be drawn and colored according to each band (column) of
  // the FHT spectrum data. This block is a kWidth x kHeight region, along
  // with 1-px of padding on its right and bottom.
  //
  //            Conditional (FHTBand)           Block State / Color
  //            =====================           ===================
  //         0   <  y < fade_row & fade-out     : Inactive  / BG color
  //    fade_row <  y < row      & fade-out     : Fade-out  / darkening
  //         0   <  y < row      & no fade-out  : Inactive  / BG color
  //    row      == y                           : Threshold / FG color
  //    row      <  y < rows_                   : Active    / Vert. gradient
  //          {1-px padding region}             : Padding   / Pad color
  //

  //
  // Paint the canvas in one go in order to mimize cache thrashing.
  //
  QRgb* line;       // Current scanline.
  uint px_w, px_h;  // Current width and height in pixels (just to avoid cast).
  uint to_x;        // [0, width())   Current and ending x pixel coordinate.
  uint to_y;        // [0, height())  Current and ending y pixel coordinate.
  uint blk_r;       // [0, rows_)     Current block's row.
  uint blk_c;       // [0, columns_)  Current block's column.

  quint32 padcolor = pad_color_.rgba();
  quint32 blkcolor;

  px_w = static_cast<uint>(width());
  px_h = static_cast<uint>(height());

  // Draw empty top padding, if needed (when y_ > 0. weird window size?).
  for (y = 0; y < y_; ++y) {
    line = reinterpret_cast<QRgb*>(canvas_.scanLine(y));
    for (x = 0; x < px_w; line[x++] = padcolor)
      ;
  }

  // Draw the texture in one shot, iterating in a row-major fashion.
  for (blk_r = 0; blk_r < rows_; ++blk_r) {
    to_y = qMin(y + kHeight, px_h);

    // This block may take several 1-px high scanlines. Each column needs
    // to be filled accordingly for each of these rows.
    for (; y < to_y; ++y) {
      line = reinterpret_cast<QRgb*>(canvas_.scanLine(y));

      for (x = 0, blk_c = 0; blk_c < columns_; ++blk_c) {
        to_x = qMin(x + kWidth, px_w);

        // Draw [x, to_x], then padding on the right.
        blkcolor = colorFromRowAndBand(blk_r, bandinfo_[blk_c]);

        for (; x < to_x; line[x++] = blkcolor)
          ;
        if (x < px_w) line[x++] = padcolor;
      }

      // If extra space remains in line, fill to the right edge.
      for (; x < px_w; line[x++] = padcolor)
        ;
    }

    // Draw a full line of padding below the just-drawn region (if in bounds).
    if (y < px_h) {
      line = reinterpret_cast<QRgb*>(canvas_.scanLine(y++));
      for (x = 0; x < px_w; line[x++] = padcolor)
        ;
    }
  }

  // If not at bottom boundary yet, pad remaining lines.
  while (y < px_h) {
    line = reinterpret_cast<QRgb*>(canvas_.scanLine(y++));
    for (x = 0; x < px_w; line[x++] = padcolor)
      ;
  }

  p.drawImage(0, 0, canvas_, 0, 0, width(), height(), Qt::NoFormatConversion);
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
static QColor ensureContrast(const QColor& bg, const QColor& fg,
                             uint _amount = 150) {
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
    // a third of the colour wheel automatically guarantees contrast
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
  QColor bg, bgdark, fg;

  bg = palette().color(QPalette::Background);
  bgdark = bg.darker(112);

  if (psychedelic_enabled_)
    fg = getPsychedelicColor(scope_, 10, 75);
  else
    fg = ensureContrast(bg, palette().color(QPalette::Highlight));

  fg_color_ = fg;
  bg_color_ = bgdark;
  pad_color_ = bg;

  // Calculate background gradient colors.
  {
    const float dr = 15.f * (bg.red() - fg.red()) / (16.f * rows_);
    const float dg = 15.f * (bg.green() - fg.green()) / (16.f * rows_);
    const float db = 15.f * (bg.blue() - fg.blue()) / (16.f * rows_);

    for (uint y = 0; y < rows_; ++y) {
      bg_grad_[y] = qRgba(fg.red() + static_cast<int>(dr * y),
                          fg.green() + static_cast<int>(dg * y),
                          fg.blue() + static_cast<int>(db * y), 255);
    }

    bg_grad_[rows_] = bg.rgba();
  }

  // make a complimentary fadebar colour
  // TODO(John Maguire): dark is not always correct, dumbo!
  {
    int h, s, v;

    bg.darker(150).getHsv(&h, &s, &v);
    fg = QColor::fromHsv(h + 120, s, v);

    const float r = 1.f * bgdark.red();
    const float g = 1.f * bgdark.green();
    const float b = 1.f * bgdark.blue();
    const float dr = 1.f * fg.red() - r;
    const float dg = 1.f * fg.green() - g;
    const float db = 1.f * fg.blue() - b;

    const float fFscl = 1. * kFadeSize;
    const float frlogFscl = 1.f / log10f(fFscl);

    for (uint y = 0; y < kFadeSize; ++y) {
      const float lrY = 1.f - (frlogFscl * log10f(fFscl - y));
      fade_bars_[y] =
          qRgba(static_cast<int>(r + lrY * dr), static_cast<int>(g + lrY * dg),
                static_cast<int>(b + lrY * db), 255);
    }
  }
}
