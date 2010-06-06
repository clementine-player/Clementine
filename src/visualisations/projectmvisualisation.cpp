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

#include "projectmvisualisation.h"

#include <QTimerEvent>
#include <QtDebug>

#include <libprojectM/projectM.hpp>

ProjectMVisualisation::ProjectMVisualisation(QWidget *parent)
  : QGLWidget(parent),
    projectm_(NULL)
{
}

ProjectMVisualisation::~ProjectMVisualisation() {
}

void ProjectMVisualisation::showEvent(QShowEvent *) {
  redraw_timer_.start(1000/25, this);
}

void ProjectMVisualisation::hideEvent(QHideEvent *) {
  redraw_timer_.stop();
}

void ProjectMVisualisation::timerEvent(QTimerEvent* e) {
  if (e->timerId() == redraw_timer_.timerId())
    update();
}

void ProjectMVisualisation::initializeGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  projectm_.reset(new projectM("/usr/share/projectM/config.inp"));
  projectm_->selectPreset(100);
}

void ProjectMVisualisation::paintGL() {
  projectm_->renderFrame();
}

void ProjectMVisualisation::resizeGL(int w, int h) {
  projectm_->projectM_resetGL(w, h);
}

void ProjectMVisualisation::ConsumeBuffer(GstBuffer *buffer, GstEnginePipeline*) {
  const int samples = GST_BUFFER_SIZE(buffer) / sizeof(short);
  const short* data = reinterpret_cast<short*>(GST_BUFFER_DATA(buffer));

  projectm_->pcm()->addPCM16Data(data, samples);
  gst_buffer_unref(buffer);
}
