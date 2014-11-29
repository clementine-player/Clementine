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

#ifndef ANALYZERS_GLANALYZER2_H_
#define ANALYZERS_GLANALYZER2_H_

#include <config.h>
#ifdef HAVE_QGLWIDGET

#include "analyzerbase.h"
#include <qstring.h>
#include <qptrlist.h>

class GLAnalyzer2 : public Analyzer::Base3D {
 public:
  explicit GLAnalyzer2(QWidget*);
  ~GLAnalyzer2();
  void analyze(const Scope&);
  void paused();

 protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();

 private:
  struct ShowProperties {
    bool paused;
    double timeStamp;
    double dT;
    double pauseTimer;
    float rotDegrees;
  } show;

  struct FrameProperties {
    float energy;
    float dEnergy;
    float meanBand;
    float rotDegrees;
    bool silence;
  } frame;

  GLuint dotTexture;
  GLuint w1Texture;
  GLuint w2Texture;
  float unitX, unitY;

  void drawDot(float x, float y, float size);
  void drawFullDot(float r, float g, float b, float a);
  void setTextureMatrix(float rot, float scale);

  bool loadTexture(QString file, GLuint& textureID);
  void freeTexture(GLuint& textureID);
};

#endif
#endif  // ANALYZERS_GLANALYZER2_H_
