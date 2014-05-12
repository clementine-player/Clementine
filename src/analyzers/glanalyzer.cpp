/***************************************************************************
                          gloscope.cpp  -  description
                             -------------------
    begin                : Jan 17 2004
    copyright            : (C) 2004 by Adam Pigg
    email                : adam@piggz.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#ifdef HAVE_QGLWIDGET

#include <cmath>
#include "glanalyzer.h"
#include <kdebug.h>

GLAnalyzer::GLAnalyzer(QWidget* parent)
    : Analyzer::Base3D(parent, 15), m_oldy(32, -10.0f), m_peaks(32) {}

GLAnalyzer::~GLAnalyzer() {}

// METHODS =====================================================

void GLAnalyzer::analyze(const Scope& s) {
  // kdDebug() << "Scope Size: " << s.size() << endl;
  /* Scope t(32);
  if (s.size() != 32)
  {
          Analyzer::interpolate(s, t);
  }
  else
  {
          t = s;
  }*/
  uint offset = 0;
  static float peak;
  float mfactor = 0.0;
  static int drawcount;

  if (s.size() == 64) {
    offset = 8;
  }

  glRotatef(0.25f, 0.0f, 1.0f, 0.5f);  // Rotate the scene
  drawFloor();
  drawcount++;
  if (drawcount > 25) {
    drawcount = 0;
    peak = 0.0;
  }

  for (uint i = 0; i < 32; i++) {
    if (s[i] > peak) {
      peak = s[i];
    }
  }

  mfactor = 20 / peak;
  for (uint i = 0; i < 32; i++) {

    // kdDebug() << "Scope item " << i << " value: " << s[i] << endl;

    // Calculate new horizontal position (x) depending on number of samples
    x = -16.0f + i;

    // Calculating new vertical position (y) depending on the data passed by
    // amarok
    y = float(s[i + offset] * mfactor);  // This make it kinda dynamically
                                         // resize depending on the data

    // Some basic bounds checking
    if (y > 30)
      y = 30;
    else if (y < 0)
      y = 0;

    if ((y - m_oldy[i]) < -0.6f)  // Going Down Too Much
    {
      y = m_oldy[i] - 0.7f;
    }
    if (y < 0.0f) {
      y = 0.0f;
    }

    m_oldy[i] = y;  // Save value as last value

    // Peak Code
    if (m_oldy[i] > m_peaks[i].level) {
      m_peaks[i].level = m_oldy[i];
      m_peaks[i].delay = 30;
    }

    if (m_peaks[i].delay > 0) {
      m_peaks[i].delay--;
    }

    if (m_peaks[i].level > 1.0f) {
      if (m_peaks[i].delay <= 0) {
        m_peaks[i].level -= 0.4f;
      }
    }
    // Draw the bar
    drawBar(x, y);
    drawPeak(x, m_peaks[i].level);
  }

  updateGL();
}

void GLAnalyzer::initializeGL() {
  // Clear frame (next fading will be preferred to clearing)
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set clear color to black
  glClear(GL_COLOR_BUFFER_BIT);

  // Set the shading model
  glShadeModel(GL_SMOOTH);

  // Set the polygon mode to fill
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Enable depth testing for hidden line removal
  glEnable(GL_DEPTH_TEST);

  // Set blend parameters for 'composting alpha'
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLAnalyzer::resizeGL(int w, int h) {
  glViewport(0, 0, (GLint)w, (GLint)h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-16.0f, 16.0f, -10.0f, 10.0f, -50.0f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void GLAnalyzer::paintGL() {
  glMatrixMode(GL_MODELVIEW);
#if 0
        glClear( GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT );
#else
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glPushMatrix();
  glLoadIdentity();
  glBegin(GL_TRIANGLE_STRIP);
  glColor4f(0.0f, 0.0f, 0.1f, 0.08f);
  glVertex2f(20.0f, 10.0f);
  glVertex2f(-20.0f, 10.0f);
  glVertex2f(20.0f, -10.0f);
  glVertex2f(-20.0f, -10.0f);
  glEnd();
  glPopMatrix();
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);
#endif
  // swapBuffers();

  glFlush();
}

void GLAnalyzer::drawBar(float xPos, float height) {
  glPushMatrix();

  // Sets color to blue
  // Set the colour depending on the height of the bar
  glColor3f((height / 40) + 0.5f, (height / 40) + 0.625f, 1.0f);
  glTranslatef(xPos, -10.0f, 0.0f);

  glScalef(1.0f, height, 3.0f);
  drawCube();

  // Set colour to full blue
  // glColor3f(0.0f, 0.0f, 1.0f);
  // drawFrame();
  glPopMatrix();
}

void GLAnalyzer::drawFloor() {
  glPushMatrix();

  // Sets color to amarok blue
  glColor3f(0.5f, 0.625f, 1.0f);
  glTranslatef(-16.0f, -11.0f, -4.0f);

  glScalef(32.0f, 1.0f, 10.0f);
  drawCube();

  // Set colour to full blue
  glColor3f(0.0f, 0.0f, 1.0f);
  drawFrame();
  glPopMatrix();
}

void GLAnalyzer::drawPeak(float xPos, float ypos) {
  glPushMatrix();

  // Set the colour to red
  glColor3f(1.0f, 0.0f, 0.0f);
  glTranslatef(xPos, ypos - 10.0f, 0.0f);

  glScalef(1.0f, 1.0f, 3.0f);
  drawCube();

  glPopMatrix();
}

void GLAnalyzer::drawCube() {
  glPushMatrix();
  glBegin(GL_POLYGON);

  // This is the top face
  glVertex3f(0.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);

  // This is the front face
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);

  // This is the right face
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);

  // This is the left face
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);

  // This is the bottom face
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);

  // This is the back face
  glVertex3f(0.0f, 0.0f, 1.0f);
  glVertex3f(1.0f, 0.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 1.0f);

  glEnd();
  glPopMatrix();
}
void GLAnalyzer::drawFrame() {
  glPushMatrix();
  glBegin(GL_LINES);

  // This is the top face
  glVertex3f(0.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);

  // This is the front face
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);

  // This is the right face
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);

  // This is the left face
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);

  // This is the bottom face
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);

  // This is the back face
  glVertex3f(0.0f, 0.0f, 1.0f);
  glVertex3f(1.0f, 0.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 1.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 1.0f);

  glEnd();
  glPopMatrix();
}

#endif
