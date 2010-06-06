/* This file is part of Clementine.

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

#ifndef PROJECTMVISUALISATION_H
#define PROJECTMVISUALISATION_H

#include <QGLWidget>
#include <QBasicTimer>

#include <boost/scoped_ptr.hpp>

#include "engines/bufferconsumer.h"

class projectM;

class ProjectMVisualisation : public QGLWidget, public BufferConsumer {
  Q_OBJECT
public:
  ProjectMVisualisation(QWidget *parent = 0);
  ~ProjectMVisualisation();

  // QGLWidget
  void paintGL();
  void resizeGL(int w, int h);
  void initializeGL();

  // BufferConsumer
  void ConsumeBuffer(GstBuffer *buffer, GstEnginePipeline*);

protected:
  // QWidget
  void hideEvent(QHideEvent *);
  void showEvent(QShowEvent *);
  void timerEvent(QTimerEvent *);

private:
  boost::scoped_ptr<projectM> projectm_;
  QBasicTimer redraw_timer_;
};

#endif // PROJECTMVISUALISATION_H
