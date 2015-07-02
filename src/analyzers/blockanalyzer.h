/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
   Copyright 2014, Krzysztof A. Sobiecki <sobkas@gmail.com>

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

#include "analyzerbase.h"
#include <qcolor.h>

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
  static const uint kMinColumns;
  static const uint kMaxColumns;
  static const uint kFadeSize;

  static const char* kName;

 protected:
  virtual void transform(Analyzer::Scope&);
  virtual void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);
  virtual void resizeEvent(QResizeEvent*);
  virtual void paletteChange(const QPalette&);
  virtual void framerateChanged();
  virtual void psychedelicModeChanged(bool);

  void drawBackground();
  void determineStep();

 private:
  QPixmap* bar() { return &barPixmap_; }

  uint columns_, rows_;  // number of rows and columns of blocks
  uint y_;               // y-offset from top of widget
  QPixmap barPixmap_;
  QPixmap topBarPixmap_;
  QPixmap background_;
  QPixmap canvas_;
  Analyzer::Scope scope_;     // so we don't create a vector every frame
  std::vector<float> store_;  // current bar kHeights
  std::vector<float> yscale_;

  // FIXME why can't I namespace these? c++ issue?
  std::vector<QPixmap> fade_bars_;
  std::vector<uint> fade_pos_;
  std::vector<int> fade_intensity_;

  float step_;  // rows to fall per frame
};

#endif  // ANALYZERS_BLOCKANALYZER_H_
