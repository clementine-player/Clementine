/* This file is part of Clementine.
   Copyright 2004, Melchior FRANZ <mfranz@kde.org>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
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

/* Original Author:  Melchior FRANZ  <mfranz@kde.org>  2004
 */

#ifndef ANALYZERS_SONOGRAM_H_
#define ANALYZERS_SONOGRAM_H_

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
  void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);
  void transform(Analyzer::Scope&);
  void demo(QPainter& p);
  void resizeEvent(QResizeEvent*);

  QPixmap canvas_;
};

#endif  // ANALYZERS_SONOGRAM_H_
