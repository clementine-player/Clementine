/* This file is part of Clementine.
   Copyright 2004, Max Howell <max.howell@methylblue.com>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
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

/* Original Author:  Max Howell  <max.howell@methylblue.com>  2004
 */

#ifndef ANALYZERS_BOOMANALYZER_H_
#define ANALYZERS_BOOMANALYZER_H_

#include "analyzerbase.h"

class BoomAnalyzer : public Analyzer::Base {
  Q_OBJECT

 public:
  Q_INVOKABLE BoomAnalyzer(QWidget*);

  static const char* kName;

  virtual void transform(Analyzer::Scope& s);
  virtual void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);

 public slots:
  void changeK_barHeight(int);
  void changeF_peakSpeed(int);

 protected:
  void resizeEvent(QResizeEvent* e);
  void paletteChange(const QPalette&);

  static const uint kColumnWidth;
  static const uint kMaxBandCount;
  static const uint kMinBandCount;

  uint bands_;
  Analyzer::Scope scope_;
  QColor fg_;

  double K_barHeight_, F_peakSpeed_, F_;

  std::vector<float> bar_height_;
  std::vector<float> peak_height_;
  std::vector<float> peak_speed_;

  QPixmap barPixmap_;
  QPixmap canvas_;
};

#endif  // ANALYZERS_BOOMANALYZER_H_
