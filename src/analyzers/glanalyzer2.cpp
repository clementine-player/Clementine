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

#include <kdebug.h>
#include <kstandarddirs.h>
#include <qimage.h>
#include <sys/time.h>

#include <cmath>
#include <cstdlib>

#include "glanalyzer2.h"

GLAnalyzer2::GLAnalyzer2(QWidget* parent) : Analyzer::Base3D(parent, 15) {
  // initialize openGL context before managing GL calls
  makeCurrent();
  loadTexture(locate("data", "amarok/data/dot.png"), dotTexture);
  loadTexture(locate("data", "amarok/data/wirl1.png"), w1Texture);
  loadTexture(locate("data", "amarok/data/wirl2.png"), w2Texture);

  show.paused = true;
  show.pauseTimer = 0.0;
  show.rotDegrees = 0.0;
  frame.rotDegrees = 0.0;
}

GLAnalyzer2::~GLAnalyzer2() {
  freeTexture(dotTexture);
  freeTexture(w1Texture);
  freeTexture(w2Texture);
}

void GLAnalyzer2::initializeGL() {
  // Set a smooth shade model
  glShadeModel(GL_SMOOTH);

  // Disable depth test (all is drawn on a 2d plane)
  glDisable(GL_DEPTH_TEST);

  // Set blend parameters for 'composting alpha'
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // superpose
  // glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );        //fade
  glEnable(GL_BLEND);

  // Clear frame with a black background
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void GLAnalyzer2::resizeGL(int w, int h) {
  // Setup screen. We're going to manually do the perspective projection
  glViewport(0, 0, (GLint)w, (GLint)h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-10.0f, 10.0f, -10.0f, 10.0f, -5.0f, 5.0f);

  // Get the aspect ratio of the screen to draw 'cicular' particles
  float ratio = static_cast<float>(w) / static_cast<float>(h), eqPixH = 60,
        eqPixW = 80;
  if (ratio >= (4.0 / 3.0)) {
    unitX = 10.0 / (eqPixH * ratio);
    unitY = 10.0 / eqPixH;
  } else {
    unitX = 10.0 / eqPixW;
    unitY = 10.0 / (eqPixW / ratio);
  }

  // Get current timestamp.
  timeval tv;
  gettimeofday(&tv, nullptr);
  show.timeStamp = static_cast<double>(tv.tv_sec) +
                   static_cast<double>(tv.tv_usec) / 1000000.0;
}

void GLAnalyzer2::paused() { analyze(Scope()); }

void GLAnalyzer2::analyze(const Scope& s) {
  bool haveNoData = s.empty();

  // if we're going into pause mode, clear timers.
  if (!show.paused && haveNoData) show.pauseTimer = 0.0;

  // if we have got data, interpolate it (asking myself why I'm doing it here..)
  if (!(show.paused = haveNoData)) {
    int bands = s.size(), lowbands = bands / 4, hibands = bands / 3,
        midbands = bands - lowbands - hibands;
    Q_UNUSED(midbands);
    float currentEnergy = 0, currentMeanBand = 0, maxValue = 0;
    for (int i = 0; i < bands; i++) {
      float value = s[i];
      currentEnergy += value;
      currentMeanBand += static_cast<float>(i) * value;
      if (value > maxValue) maxValue = value;
    }
    frame.silence = currentEnergy < 0.001;
    if (!frame.silence) {
      frame.meanBand = 100.0 * currentMeanBand / (currentEnergy * bands);
      currentEnergy = 100.0 * currentEnergy / static_cast<float>(bands);
      frame.dEnergy = currentEnergy - frame.energy;
      frame.energy = currentEnergy;
      //            printf( "%d  [%f :: %f ]\t%f \n", bands, frame.energy,
      // frame.meanBand, maxValue         );
    } else {
      frame.energy = 0.0;
    }
  }

  // update the frame
  updateGL();
}

void GLAnalyzer2::paintGL() {
  // Compute the dT since the last call to paintGL and update timings
  timeval tv;
  gettimeofday(&tv, nullptr);
  double currentTime = static_cast<double>(tv.tv_sec) +
                       static_cast<double>(tv.tv_usec) / 1000000.0;
  show.dT = currentTime - show.timeStamp;
  show.timeStamp = currentTime;

  // Clear frame
  glClear(GL_COLOR_BUFFER_BIT);

  // Shitch to MODEL matrix and reset it to default
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Fade the previous drawings.
  /*    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      glBegin( GL_TRIANGLE_STRIP );
        glColor4f( 0.0f, 0.0f, 0.0f, 0.2f );
        glVertex2f( 10.0f, 10.0f );
        glVertex2f( -10.0f, 10.0f );
        glVertex2f( 10.0f, -10.0f );
        glVertex2f( -10.0f, -10.0f );
      glEnd();*/

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);
  float alphaN = show.paused ? 0.2 : (frame.energy / 10.0),
        alphaP = show.paused ? 1.0 : (1 - frame.energy / 20.0);
  if (alphaN > 1.0) alphaN = 1.0;
  if (alphaP < 0.1) alphaP = 0.1;
  glBindTexture(GL_TEXTURE_2D, w2Texture);
  setTextureMatrix(show.rotDegrees, 0.707 * alphaP);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(1.0, 1.0);
  glVertex2f(10.0f, 10.0f);
  glTexCoord2f(0.0, 1.0);
  glVertex2f(-10.0f, 10.0f);
  glTexCoord2f(1.0, 0.0);
  glVertex2f(10.0f, -10.0f);
  glTexCoord2f(0.0, 0.0);
  glVertex2f(-10.0f, -10.0f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, w1Texture);
  setTextureMatrix(-show.rotDegrees * 2, 0.707);
  glColor4f(1.0f, 1.0f, 1.0f, alphaN);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(1.0, 1.0);
  glVertex2f(10.0f, 10.0f);
  glTexCoord2f(0.0, 1.0);
  glVertex2f(-10.0f, 10.0f);
  glTexCoord2f(1.0, 0.0);
  glVertex2f(10.0f, -10.0f);
  glTexCoord2f(0.0, 0.0);
  glVertex2f(-10.0f, -10.0f);
  glEnd();
  setTextureMatrix(0.0, 0.0);
  glDisable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // Here begins the real draw loop
  // some updates to the show
  show.rotDegrees += 40.0 * show.dT;
  frame.rotDegrees += 80.0 * show.dT;

  // handle the 'pause' status
  if (show.paused) {
    if (show.pauseTimer > 0.5) {
      if (show.pauseTimer > 0.6) show.pauseTimer -= 0.6;
      drawFullDot(0.0f, 0.4f, 0.8f, 1.0f);
      drawFullDot(0.0f, 0.4f, 0.8f, 1.0f);
    }
    show.pauseTimer += show.dT;
    return;
  }

  if (dotTexture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, dotTexture);
  } else {
    glDisable(GL_TEXTURE_2D);
  }

  glLoadIdentity();
  //    glRotatef( -frame.rotDegrees, 0,0,1 );
  glBegin(GL_QUADS);
  //     Particle * particle = particleList.first();
  //     for (; particle; particle = particleList.next())
  {
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    drawDot(0, 0, kMax(10.0, (10.0 * frame.energy)));
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    drawDot(6, 0, kMax(10.0, (5.0 * frame.energy)));
    glColor4f(0.0f, 0.4f, 1.0f, 1.0f);
    drawDot(-6, 0, kMax(10.0, (5.0 * frame.energy)));
  }
  glEnd();
}

void GLAnalyzer2::drawDot(float x, float y, float size) {
  float sizeX = size * unitX, sizeY = size * unitY, pLeft = x - sizeX,
        pTop = y + sizeY, pRight = x + sizeX, pBottom = y - sizeY;
  glTexCoord2f(0, 0);  // Bottom Left
  glVertex2f(pLeft, pBottom);
  glTexCoord2f(0, 1);  // Top Left
  glVertex2f(pLeft, pTop);
  glTexCoord2f(1, 1);  // Top Right
  glVertex2f(pRight, pTop);
  glTexCoord2f(1, 0);  // Bottom Right
  glVertex2f(pRight, pBottom);
}

void GLAnalyzer2::drawFullDot(float r, float g, float b, float a) {
  glBindTexture(GL_TEXTURE_2D, dotTexture);
  glEnable(GL_TEXTURE_2D);
  glColor4f(r, g, b, a);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(1.0, 1.0);
  glVertex2f(10.0f, 10.0f);
  glTexCoord2f(0.0, 1.0);
  glVertex2f(-10.0f, 10.0f);
  glTexCoord2f(1.0, 0.0);
  glVertex2f(10.0f, -10.0f);
  glTexCoord2f(0.0, 0.0);
  glVertex2f(-10.0f, -10.0f);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void GLAnalyzer2::setTextureMatrix(float rot, float scale) {
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  if (rot != 0.0 || scale != 0.0) {
    glTranslatef(0.5f, 0.5f, 0.0f);
    glRotatef(rot, 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, 1.0f);
    glTranslatef(-0.5f, -0.5f, 0.0f);
  }
  glMatrixMode(GL_MODELVIEW);
}

bool GLAnalyzer2::loadTexture(QString fileName, GLuint& textureID) {
  // reset texture ID to the default EMPTY value
  textureID = 0;

  // load image
  QImage tmp;
  if (!tmp.load(fileName)) return false;

  // convert it to suitable format (flipped RGBA)
  QImage texture = QGLWidget::convertToGLFormat(tmp);
  if (texture.isNull()) return false;

  // get texture number and bind loaded image to that texture
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, 4, texture.width(), texture.height(), 0,
               GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
  return true;
}

void GLAnalyzer2::freeTexture(GLuint& textureID) {
  if (textureID > 0) glDeleteTextures(1, &textureID);
  textureID = 0;
}

#endif
