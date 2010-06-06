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
#include <QPainter>
#include <QPaintEngine>
#include <QtDebug>
#include <QGLWidget>
#include <QGraphicsView>

#include <libprojectM/projectM.hpp>
#include <GL/gl.h>

ProjectMVisualisation::ProjectMVisualisation(QObject *parent)
  : QGraphicsScene(parent),
    projectm_(NULL)
{
  connect(this, SIGNAL(sceneRectChanged(QRectF)), SLOT(SceneRectChanged(QRectF)));
}

ProjectMVisualisation::~ProjectMVisualisation() {
}

void ProjectMVisualisation::drawBackground(QPainter* p, const QRectF&) {
  p->beginNativePainting();

  if (!projectm_) {
    projectm_.reset(new projectM("/usr/share/projectM/config.inp"));
  }

  projectm_->projectM_resetGL(sceneRect().width(), sceneRect().height());
  projectm_->renderFrame();

  p->endNativePainting();
}

void ProjectMVisualisation::SceneRectChanged(const QRectF &rect) {
  if (projectm_)
    projectm_->projectM_resetGL(rect.width(), rect.height());
}

void ProjectMVisualisation::ConsumeBuffer(GstBuffer *buffer, GstEnginePipeline*) {
  const int samples_per_channel = GST_BUFFER_SIZE(buffer) / sizeof(short) / 2;
  const short* data = reinterpret_cast<short*>(GST_BUFFER_DATA(buffer));

  if (projectm_)
    projectm_->pcm()->addPCM16Data(data, samples_per_channel);
  gst_buffer_unref(buffer);
}
