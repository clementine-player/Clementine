//
//
// C++ Interface: Sonogram
//
// Description:
//
//
// Author: Melchior FRANZ <mfranz@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SONOGRAM_H
#define SONOGRAM_H

#include "analyzerbase.h"

/**
@author Melchior FRANZ
*/

class Sonogram : public Analyzer::Base {
  Q_OBJECT
 public:
  Q_INVOKABLE Sonogram(QWidget*);
  ~Sonogram();

  static const char* kName;

 protected:
  void analyze(QPainter& p, const Scope&, bool new_frame);
  void transform(Scope&);
  void demo(QPainter& p);
  void resizeEvent(QResizeEvent*);

  QPixmap canvas_;
};

#endif
