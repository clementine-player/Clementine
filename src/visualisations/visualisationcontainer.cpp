/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "config.h"
#include "projectmvisualisation.h"
#include "visualisationcontainer.h"
#include "visualisationoverlay.h"
#include "visualisationselector.h"
#include "engines/gstengine.h"
#include "ui/iconloader.h"
#include "ui/idlehandler.h"

#include <QGLWidget>
#include <QGraphicsProxyWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QtDebug>

// Framerates
const int VisualisationContainer::kLowFramerate = 15;
const int VisualisationContainer::kMediumFramerate = 25;
const int VisualisationContainer::kHighFramerate = 35;
const int VisualisationContainer::kSuperHighFramerate = 60;

const char* VisualisationContainer::kSettingsGroup = "Visualisations";
const int VisualisationContainer::kDefaultWidth = 828;
const int VisualisationContainer::kDefaultHeight = 512;
const int VisualisationContainer::kDefaultFps = kHighFramerate;
const int VisualisationContainer::kDefaultTextureSize = 512;

VisualisationContainer::VisualisationContainer(QWidget* parent)
    : QGraphicsView(parent),
      initialised_(false),
      engine_(nullptr),
      vis_(new ProjectMVisualisation(this)),
      overlay_(new VisualisationOverlay),
      selector_(new VisualisationSelector(this)),
      overlay_proxy_(nullptr),
      menu_(new QMenu(this)),
      fps_(kDefaultFps),
      size_(kDefaultTextureSize) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (!restoreGeometry(s.value("geometry").toByteArray())) {
    resize(kDefaultWidth, kDefaultHeight);
  }
  fps_ = s.value("fps", kDefaultFps).toInt();
  size_ = s.value("size", kDefaultTextureSize).toInt();

  QShortcut* close = new QShortcut(QKeySequence::Close, this);
  connect(close, SIGNAL(activated()), SLOT(close()));

  setMinimumSize(64, 64);
}

void VisualisationContainer::Init() {
  setWindowTitle(tr("Clementine Visualization"));
  setWindowIcon(QIcon(":/icon.png"));

  // Set up the graphics view
  setScene(vis_);
  setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setFrameStyle(QFrame::NoFrame);

  // Add the overlay
  overlay_proxy_ = scene()->addWidget(overlay_);
  connect(overlay_, SIGNAL(OpacityChanged(qreal)),
          SLOT(ChangeOverlayOpacity(qreal)));
  connect(overlay_, SIGNAL(ShowPopupMenu(QPoint)), SLOT(ShowPopupMenu(QPoint)));
  ChangeOverlayOpacity(0.0);

  vis_->SetTextureSize(size_);
  SizeChanged();

  // Selector
  selector_->SetVisualisation(vis_);

  // Settings menu
  menu_->addAction(IconLoader::Load("view-fullscreen", IconLoader::Base), 
                   tr("Toggle fullscreen"), this, SLOT(ToggleFullscreen()));

  QMenu* fps_menu = menu_->addMenu(tr("Framerate"));
  QSignalMapper* fps_mapper = new QSignalMapper(this);
  QActionGroup* fps_group = new QActionGroup(this);
  AddMenuItem(tr("Low (%1 fps)").arg(kLowFramerate), kLowFramerate, fps_,
              fps_group, fps_mapper);
  AddMenuItem(tr("Medium (%1 fps)").arg(kMediumFramerate), kMediumFramerate,
              fps_, fps_group, fps_mapper);
  AddMenuItem(tr("High (%1 fps)").arg(kHighFramerate), kHighFramerate, fps_,
              fps_group, fps_mapper);
  AddMenuItem(tr("Super high (%1 fps)").arg(kSuperHighFramerate),
              kSuperHighFramerate, fps_, fps_group, fps_mapper);
  fps_menu->addActions(fps_group->actions());
  connect(fps_mapper, SIGNAL(mapped(int)), SLOT(SetFps(int)));

  QMenu* quality_menu = menu_->addMenu(tr("Quality", "Visualisation quality"));
  QSignalMapper* quality_mapper = new QSignalMapper(this);
  QActionGroup* quality_group = new QActionGroup(this);
  AddMenuItem(tr("Low (256x256)"), 256, size_, quality_group, quality_mapper);
  AddMenuItem(tr("Medium (512x512)"), 512, size_, quality_group,
              quality_mapper);
  AddMenuItem(tr("High (1024x1024)"), 1024, size_, quality_group,
              quality_mapper);
  AddMenuItem(tr("Super high (2048x2048)"), 2048, size_, quality_group,
              quality_mapper);
  quality_menu->addActions(quality_group->actions());
  connect(quality_mapper, SIGNAL(mapped(int)), SLOT(SetQuality(int)));

  menu_->addAction(tr("Select visualizations..."), selector_, SLOT(show()));

  menu_->addSeparator();
  menu_->addAction(IconLoader::Load("application-exit", IconLoader::Base),
                   tr("Close visualization"), this, SLOT(hide()));
}

void VisualisationContainer::AddMenuItem(const QString& name, int value,
                                         int def, QActionGroup* group,
                                         QSignalMapper* mapper) {
  QAction* action = group->addAction(name);
  action->setCheckable(true);
  action->setChecked(value == def);
  mapper->setMapping(action, value);
  connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
}

void VisualisationContainer::SetEngine(GstEngine* engine) {
  engine_ = engine;

  if (isVisible()) engine_->AddBufferConsumer(vis_);
}

void VisualisationContainer::showEvent(QShowEvent* e) {
  if (!initialised_) {
    if (!QGLFormat::hasOpenGL()) {
      hide();
      QMessageBox::warning(this, tr("Clementine Visualization"),
                           tr("Your system is missing OpenGL support, "
                              "visualizations are unavailable."),
                           QMessageBox::Close);
      return;
    }
    Init();
    initialised_ = true;
  }

  QGraphicsView::showEvent(e);
  update_timer_.start(1000 / fps_, this);

  if (engine_) engine_->AddBufferConsumer(vis_);
}

void VisualisationContainer::hideEvent(QHideEvent* e) {
  QGraphicsView::hideEvent(e);
  update_timer_.stop();

  if (engine_) engine_->RemoveBufferConsumer(vis_);
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
  if (scene()) scene()->setSceneRect(QRect(QPoint(0, 0), size()));

  // Resize the overlay
  if (overlay_) overlay_->resize(size());
}

void VisualisationContainer::timerEvent(QTimerEvent* e) {
  QGraphicsView::timerEvent(e);
  if (e->timerId() == update_timer_.timerId()) scene()->update();
}

void VisualisationContainer::SetActions(QAction* previous, QAction* play_pause,
                                        QAction* stop, QAction* next) {
  overlay_->SetActions(previous, play_pause, stop, next);
}

void VisualisationContainer::SongMetadataChanged(const Song& metadata) {
  overlay_->SetSongTitle(
      QString("%1 - %2").arg(metadata.artist(), metadata.title()));
}

void VisualisationContainer::Stopped() {
  overlay_->SetSongTitle(tr("Clementine"));
}

void VisualisationContainer::ChangeOverlayOpacity(qreal value) {
  overlay_proxy_->setOpacity(value);

  // Hide the cursor if the overlay is hidden
  if (value < 0.5)
    viewport()->setCursor(Qt::BlankCursor);
  else
    viewport()->unsetCursor();
}

void VisualisationContainer::enterEvent(QEvent* e) {
  QGraphicsView::enterEvent(e);
  overlay_->SetVisible(true);
}

void VisualisationContainer::leaveEvent(QEvent* e) {
  QGraphicsView::leaveEvent(e);
  overlay_->SetVisible(false);
}

void VisualisationContainer::mouseMoveEvent(QMouseEvent* e) {
  QGraphicsView::mouseMoveEvent(e);
  overlay_->SetVisible(true);
}

void VisualisationContainer::mouseDoubleClickEvent(QMouseEvent* e) {
  QGraphicsView::mouseDoubleClickEvent(e);
  ToggleFullscreen();
}

void VisualisationContainer::contextMenuEvent(QContextMenuEvent* event) {
  QGraphicsView::contextMenuEvent(event);
  ShowPopupMenu(event->pos());
}

void VisualisationContainer::keyReleaseEvent(QKeyEvent* event) {
  if (event->matches(QKeySequence::Close) || event->key() == Qt::Key_Escape) {
    if (isFullScreen())
      ToggleFullscreen();
    else
      hide();
    return;
  }

  QGraphicsView::keyReleaseEvent(event);
}

void VisualisationContainer::ToggleFullscreen() {
  setWindowState(windowState() ^ Qt::WindowFullScreen);

  IdleHandler* screensaver = IdleHandler::GetScreensaver();
  if (screensaver)
    isFullScreen() ? screensaver->Inhibit("Visualisation") : screensaver->Uninhibit();
}

void VisualisationContainer::SetFps(int fps) {
  fps_ = fps;

  // Save settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("fps", fps_);

  update_timer_.stop();
  update_timer_.start(1000 / fps_, this);
}

void VisualisationContainer::ShowPopupMenu(const QPoint& pos) {
  menu_->popup(mapToGlobal(pos));
}

void VisualisationContainer::SetQuality(int size) {
  size_ = size;

  // Save settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("size", size_);

  vis_->SetTextureSize(size_);
}
