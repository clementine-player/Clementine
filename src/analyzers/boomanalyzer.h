// Author: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright: See COPYING file that comes with this distribution
//

#ifndef BOOMANALYZER_H
#define BOOMANALYZER_H

#include "analyzerbase.h"

/**
@author Max Howell
*/

class BoomAnalyzer : public Analyzer::Base {
  Q_OBJECT
 public:
  Q_INVOKABLE BoomAnalyzer(QWidget*);

  static const char* kName;

  virtual void init();
  virtual void transform(Scope& s);
  virtual void analyze(QPainter& p, const Scope&, bool new_frame);

 public slots:
  void changeK_barHeight(int);
  void changeF_peakSpeed(int);

 protected:
  void resizeEvent(QResizeEvent* e);

  static const uint COLUMN_WIDTH = 4;
  static const uint BAND_COUNT = 32;

  double K_barHeight, F_peakSpeed, F;

  std::vector<float> bar_height;
  std::vector<float> peak_height;
  std::vector<float> peak_speed;

  QPixmap barPixmap;
  QPixmap canvas_;
};

#endif
