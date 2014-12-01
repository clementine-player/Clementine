/* This file is part of Clementine.
   Copyright 2004, Enrico Ros <eros.kde@email.it>
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

/* Original Author:  Enrico Ros  <eros.kde@email.it>  2004
 */

#include <config.h>
#ifdef HAVE_QGLWIDGET

#ifndef ANALYZERS_GLANALYZER3_H_
#define ANALYZERS_GLANALYZER3_H_

#include "analyzerbase.h"
#include <qstring.h>
#include <qptrlist.h>

class QWidget;
class Ball;
class Paddle;

class GLAnalyzer3 : public Analyzer::Base3D {
 public:
  explicit GLAnalyzer3(QWidget*);
  ~GLAnalyzer3();
  void analyze(const Scope&);
  void paused();

 protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();

 private:
  struct ShowProperties {
    double timeStamp;
    double dT;
    float colorK;
    float gridScrollK;
    float gridEnergyK;
    float camRot;
    float camRoll;
    float peakEnergy;
  } show;

  struct FrameProperties {
    bool silence;
    float energy;
    float dEnergy;
  } frame;

  static const int NUMBER_OF_BALLS = 16;

  QPtrList<Ball> balls;
  Paddle* leftPaddle, *rightPaddle;
  float unitX, unitY;
  GLuint ballTexture;
  GLuint gridTexture;

  void drawDot3s(float x, float y, float z, float size);
  void drawHFace(float y);
  void drawScrollGrid(float scroll, float color[4]);

  bool loadTexture(QString file, GLuint& textureID);
  void freeTexture(GLuint& textureID);
};

#endif
#endif  // ANALYZERS_GLANALYZER3_H_
