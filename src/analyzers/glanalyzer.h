/* This file is part of Clementine.
   Copyright 2004, Adam Pigg <adam@piggz.co.uk>
   Copyright 2009, David Sansome <davidsansome@gmail.com>
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

/* Original Author:  Adam Pigg  <adam@piggz.co.uk>  2004
 */

#ifndef ANALYZERS_GLANALYZER_H_
#define ANALYZERS_GLANALYZER_H_

#include <config.h>
#ifdef HAVE_QGLWIDGET

#include "analyzerbase.h"

typedef struct {
  float level;
  uint delay;
} peak_tx;

class GLAnalyzer : public Analyzer::Base3D {
 private:
  std::vector<float> m_oldy;
  std::vector<peak_tx> m_peaks;

  void drawCube();
  void drawFrame();
  void drawBar(float xPos, float height);
  void drawPeak(float xPos, float ypos);
  void drawFloor();

  GLfloat x, y;

 public:
  explicit GLAnalyzer(QWidget*);
  ~GLAnalyzer();
  void analyze(const Scope&);

 protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
};

#endif
#endif  // ANALYZERS_GLANALYZER_H_
