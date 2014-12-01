/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2005, Mark Kretschmann <markey@web.de>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Mark Furneaux <mark@romaco.ca>
   Copyright 2014, Krzysztof A. Sobiecki <sobkas@gmail.com>
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

/* Original Author:  Max Howell  <max.howell@methylblue.com>  2003-2005
 * Original Author:  Mark Kretschmann  <markey@web.de>  2005
*/

#ifndef ANALYZERS_BARANALYZER_H_
#define ANALYZERS_BARANALYZER_H_

#include "analyzerbase.h"

typedef std::vector<uint> aroofMemVec;

class BarAnalyzer : public Analyzer::Base {
  Q_OBJECT

 public:
  Q_INVOKABLE BarAnalyzer(QWidget*);

  void init();
  virtual void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);
  // virtual void transform( Scope& );

  /**
   * Resizes the widget to a new geometry according to @p e
   * @param e The resize-event
   */
  void resizeEvent(QResizeEvent* e);

  uint BAND_COUNT;
  int MAX_DOWN;
  int MAX_UP;
  static const uint ROOF_HOLD_TIME = 48;
  static const int ROOF_VELOCITY_REDUCTION_FACTOR = 32;
  static const uint NUM_ROOFS = 16;
  static const uint COLUMN_WIDTH = 4;

  static const char* kName;

 protected:
  QPixmap m_pixRoof[NUM_ROOFS];
  // vector<uint> m_roofMem[BAND_COUNT];

  // Scope m_bands; //copy of the Scope to prevent creating/destroying a Scope
  // every iteration
  uint m_lvlMapper[256];
  std::vector<aroofMemVec> m_roofMem;
  std::vector<uint> barVector;           // positions of bars
  std::vector<int> roofVector;           // positions of roofs
  std::vector<uint> roofVelocityVector;  // speed that roofs falls

  const QPixmap* gradient() const { return &m_pixBarGradient; }

 private:
  QPixmap m_pixBarGradient;
  QPixmap m_pixCompose;
  QPixmap canvas_;
  Analyzer::Scope m_scope;  // so we don't create a vector every frame
  QColor m_bg;
};

#endif  // ANALYZERS_BARANALYZER_H_
