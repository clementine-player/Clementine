/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
   Copyright 2014, Krzysztof A. Sobiecki <sobkas@gmail.com>
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
 */

#ifndef ANALYZERS_BLOCKANALYZER_H_
#define ANALYZERS_BLOCKANALYZER_H_

#include <qcolor.h>

#include "analyzerbase.h"

class QResizeEvent;
class QMouseEvent;
class QPalette;

class BlockAnalyzer : public Analyzer::Base {
  Q_OBJECT

 public:
  Q_INVOKABLE BlockAnalyzer(QWidget*);
  ~BlockAnalyzer();

  static const uint kHeight;
  static const uint kWidth;
  static const uint kMinRows;
  static const uint kMaxRows;
  static const uint kMinColumns;
  static const uint kMaxColumns;
  static const uint kFadeSize;
  static const uint kFadeInitial;

  static const char* kName;

 protected:
  virtual void transform(Analyzer::Scope&);
  virtual void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);
  virtual void resizeEvent(QResizeEvent*);
  virtual void paletteChange(const QPalette&);
  virtual void framerateChanged();
  virtual void psychedelicModeChanged(bool);

  void determineStep();

 private:
  struct FHTBand {
    FHTBand()
        : height(0.f),
          row(0),
          fade_row(0),
          fade_coloridx(kMaxRows),
          fade_intensity(kFadeInitial) {}

    // Top of the spectral activity bar.
    float height;  // Foreground-Background transition row.
    uint row;      // Integer floor of the height value.

    // Vertical color fade effect (a sort of hysteresis).
    uint fade_row;       // Row in which to begin showing BG gradient.
    uint fade_coloridx;  // Current fade_bars_[] offset.
    int fade_intensity;  // Current intensity frame counter value.
  };

  inline quint32 colorFromRowAndBand(uint cur_r, const FHTBand& band);

  Analyzer::Scope scope_;

  uint columns_;  // Number of columns of blocks.
  uint rows_;     // Number of rows of blocks.
  uint y_;        // y-offset from top of widget.
  float step_;    // Rows to fall per frame (during inactivity).

  QColor fg_color_;   // Foreground/Active block color.
  QColor bg_color_;   // Background/Inactive block color.
  QColor pad_color_;  // Color of 'lines' dividing the blocks.
  QImage canvas_;     // Drawable canvas of widget.

  QVector<float> rthresh_;      // [rows_+1] Rowwise intensity thresholds.
  QVector<quint32> bg_grad_;    // [rows_+1] Vertical background gradient.
  QVector<quint32> fade_bars_;  // [kFadeSize] Block colors per fade level.
  QVector<FHTBand> bandinfo_;   // [columns_] FHT band info.
};

inline quint32 BlockAnalyzer::colorFromRowAndBand(uint r, const FHTBand& band) {
  // Calculate the block color given band info and the current row.
  // Note: 0 <= r <= rows_.
  if (r == band.row)
    return fg_color_.rgba();
  else if (r > band.row)
    return bg_grad_[r];
  else if ((band.fade_intensity > 0) && (r >= band.fade_row))
    return fade_bars_[band.fade_coloridx];
  else
    return bg_color_.rgba();
}

#endif  // ANALYZERS_BLOCKANALYZER_H_
