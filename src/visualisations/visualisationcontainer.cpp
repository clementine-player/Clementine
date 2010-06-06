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
#include "visualisationcontainer.h"
#include "visualisationoverlay.h"
#include "engines/gstengine.h"

#include <QHBoxLayout>
#include <QSettings>
#include <QGLWidget>
#include <QtDebug>
#include <QGraphicsProxyWidget>

#include <QLabel>

const char* VisualisationContainer::kSettingsGroup = "Visualisations";
const int VisualisationContainer::kDefaultWidth = 828;
const int VisualisationContainer::kDefaultHeight = 512;
const int VisualisationContainer::kDefaultFps = 35;

VisualisationContainer::VisualisationContainer(QWidget *parent)
  : QGraphicsView(parent),
    engine_(NULL),
    vis_(new ProjectMVisualisation(this)),
    overlay_(new VisualisationOverlay),
    fps_(kDefaultFps)
{
  setWindowTitle(tr("Clementine Visualisation"));

  // Set up the graphics view
  setScene(vis_);
  setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Add the overlay
  overlay_proxy_ = scene()->addWidget(overlay_);
  connect(overlay_, SIGNAL(OpacityChanged(qreal)), SLOT(ChangeOverlayOpacity(qreal)));
  ChangeOverlayOpacity(0.0);

  // Load settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (!restoreGeometry(s.value("geometry").toByteArray())) {
    resize(kDefaultWidth, kDefaultHeight);
  }
  fps_ = s.value("fps", kDefaultFps).toInt();

  SizeChanged();
}

void VisualisationContainer::SetEngine(GstEngine* engine) {
  engine_ = engine;
  if (isVisible())
    engine_->AddBufferConsumer(vis_);
}

void VisualisationContainer::showEvent(QShowEvent* e) {
  QGraphicsView::showEvent(e);
  update_timer_.start(1000 / fps_, this);

  if (engine_)
    engine_->AddBufferConsumer(vis_);
}

void VisualisationContainer::hideEvent(QHideEvent* e) {
  QGraphicsView::hideEvent(e);
  update_timer_.stop();

  if (engine_)
    engine_->RemoveBufferConsumer(vis_);
}

void VisualisationContainer::resizeEvent(QResizeEvent* e) {
  QGraphicsView::resizeEvent(e);
  SizeChanged();
}

void VisualisationContainer::SizeChanged() {
  // Save the geometry
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("geometry", saveGeometry());

  // Resize the scene
  if (scene())
    scene()->setSceneRect(QRect(QPoint(0, 0), size()));

  // Resize the overlay
  overlay_->resize(size());
}

void VisualisationContainer::timerEvent(QTimerEvent* e) {
  if (e->timerId() == update_timer_.timerId())
    scene()->update();
}

void VisualisationContainer::SetActions(QAction *previous, QAction *play_pause,
                                        QAction *stop, QAction *next) {
  overlay_->SetActions(previous, play_pause, stop, next);
}

void VisualisationContainer::SongMetadataChanged(const Song &metadata) {
  overlay_->SetSongTitle(QString("%1 - %2").arg(metadata.artist(), metadata.title()));
}

void VisualisationContainer::Stopped() {
  overlay_->SetSongTitle(tr("Clementine"));
}

void VisualisationContainer::ChangeOverlayOpacity(qreal value) {
  overlay_proxy_->setOpacity(value);
}

void VisualisationContainer::enterEvent(QEvent *) {
  overlay_->SetVisible(true);
}

void VisualisationContainer::leaveEvent(QEvent *) {
  overlay_->SetVisible(false);
}

void VisualisationContainer::mouseMoveEvent(QMouseEvent *) {
  overlay_->SetVisible(true);
}
