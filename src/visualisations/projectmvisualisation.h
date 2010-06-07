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
#include <QSet>

#include <boost/scoped_ptr.hpp>

#include "engines/bufferconsumer.h"

class projectM;

class ProjectMVisualisation : public QGraphicsScene, public BufferConsumer {
  Q_OBJECT
public:
  ProjectMVisualisation(QObject *parent = 0);
  ~ProjectMVisualisation();

  projectM* projectm() const { return projectm_.get(); }

  QSet<int> selected_indices() const { return selected_indices_; }
  bool is_selected(int preset) { return selected_indices_.contains(preset); }
  void set_selected(int preset, bool selected);
  void set_all_selected(bool selected);

  int mode() const { return mode_; }
  void set_mode(int mode);

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
  void Load();
  void Save();

private:
  boost::scoped_ptr<projectM> projectm_;
  int mode_;
  QSet<int> selected_indices_;

  int texture_size_;
};

#endif // PROJECTMVISUALISATION_H
