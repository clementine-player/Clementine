/* This file is part of Clementine.
   Copyright 2003-2005, Max Howell <max.howell@methylblue.com>
   Copyright 2005, Mark Kretschmann <markey@web.de>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
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
  virtual void psychedelicModeChanged(bool);

  /**
   * Resizes the widget to a new geometry according to @p e
   * @param e The resize-event
   */
  void resizeEvent(QResizeEvent* e);
  void colorChanged();

  uint band_count_;
  int max_down_;
  int max_up_;
  static const uint kRoofHoldTime = 48;
  static const int kRoofVelocityReductionFactor = 32;
  static const uint kNumRoofs = 16;
  static const uint kColumnWidth = 4;

  static const char* kName;

 protected:
  QPixmap pixRoof_[kNumRoofs];
  // vector<uint> roofMem_[band_count_];

  // Scope m_bands; //copy of the Scope to prevent creating/destroying a Scope
  // every iteration
  uint lvlMapper_[256];
  std::vector<aroofMemVec> roofMem_;
  std::vector<uint> barVector_;           // positions of bars
  std::vector<int> roofVector_;           // positions of roofs
  std::vector<uint> roofVelocityVector_;  // speed that roofs falls

  const QPixmap* gradient() const { return &pixBarGradient_; }

 private:
  QPixmap pixBarGradient_;
  QPixmap pixCompose_;
  QPixmap canvas_;
  Analyzer::Scope scope_;  // so we don't create a vector every frame
  QColor bg_;
};

#endif  // ANALYZERS_BARANALYZER_H_
