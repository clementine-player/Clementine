//
// Amarok BarAnalyzer 3 - Jet Turbine: Symmetric version of analyzer 1
//
// Author: Stanislav Karchebny <berkus@users.sf.net>, (C) 2003
//
// Copyright: like rest of Amarok
//

#ifndef ANALYZER_TURBINE_H
#define ANALYZER_TURBINE_H

#include "boomanalyzer.h"

class TurbineAnalyzer : public BoomAnalyzer {
  Q_OBJECT
 public:
  Q_INVOKABLE TurbineAnalyzer(QWidget* parent) : BoomAnalyzer(parent) {}

  void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);

  static const char* kName;
};

#endif
