/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Mark Furneaux <mark@romaco.ca>
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

  static const uint HEIGHT;
  static const uint WIDTH;
  static const uint MIN_ROWS;
  static const uint MIN_COLUMNS;
  static const uint MAX_COLUMNS;
  static const uint FADE_SIZE;

  static const char* kName;

 protected:
  virtual void transform(Analyzer::Scope&);
  virtual void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);
  virtual void resizeEvent(QResizeEvent*);
  virtual void paletteChange(const QPalette&);
  virtual void framerateChanged();

  void drawBackground();
  void determineStep();

 private:
  QPixmap* bar() { return &m_barPixmap; }

  uint m_columns, m_rows;  // number of rows and columns of blocks
  uint m_y;                // y-offset from top of widget
  QPixmap m_barPixmap;
  QPixmap m_topBarPixmap;
  QPixmap m_background;
  QPixmap canvas_;
  Analyzer::Scope m_scope;     // so we don't create a vector every frame
  std::vector<float> m_store;  // current bar heights
  std::vector<float> m_yscale;

  // FIXME why can't I namespace these? c++ issue?
  std::vector<QPixmap> m_fade_bars;
  std::vector<uint> m_fade_pos;
  std::vector<int> m_fade_intensity;

  float m_step;  // rows to fall per frame
};

#endif  // ANALYZERS_BLOCKANALYZER_H_
