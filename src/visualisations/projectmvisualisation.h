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

#include <QGraphicsScene>
#include <QBasicTimer>

#include <boost/scoped_ptr.hpp>

#include "engines/bufferconsumer.h"

class projectM;

class ProjectMVisualisation : public QGraphicsScene, public BufferConsumer {
  Q_OBJECT
public:
  ProjectMVisualisation(QObject *parent = 0);
  ~ProjectMVisualisation();

  // BufferConsumer
  void ConsumeBuffer(GstBuffer *buffer, GstEnginePipeline*);

public slots:
  void SetTextureSize(int size);

protected:
  // QGraphicsScene
  void drawBackground(QPainter *painter, const QRectF &rect);

private slots:
  void SceneRectChanged(const QRectF& rect);

private:
  boost::scoped_ptr<projectM> projectm_;

  int texture_size_;
};

#endif // PROJECTMVISUALISATION_H
