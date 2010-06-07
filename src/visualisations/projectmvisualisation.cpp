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

#include "config.h"
#include "projectmvisualisation.h"
#include "visualisationcontainer.h"

#include <QTimerEvent>
#include <QPainter>
#include <QPaintEngine>
#include <QtDebug>
#include <QGLWidget>
#include <QGraphicsView>
#include <QSettings>
#include <QCoreApplication>
#include <QFile>

#include <projectM.hpp>
#include <GL/gl.h>

#ifdef Q_OS_MAC
#  include "core/mac_startup.h"
#endif

ProjectMVisualisation::ProjectMVisualisation(QObject *parent)
  : QGraphicsScene(parent),
    projectm_(NULL),
    mode_(0),
    texture_size_(512)
{
  connect(this, SIGNAL(sceneRectChanged(QRectF)), SLOT(SceneRectChanged(QRectF)));
}

ProjectMVisualisation::~ProjectMVisualisation() {
}

void ProjectMVisualisation::InitProjectM() {
  // Find the projectM presets
  QStringList paths = QStringList()
      << CMAKE_INSTALL_PREFIX "/share/clementine/projectm-presets"
      << "/usr/share/clementine/projectm-presets"
      << "/usr/local/share/clementine/projectm-presets"
      << CMAKE_INSTALL_PREFIX "/share/projectM/presets"
      << "/usr/share/projectM/presets"
      << "/usr/local/share/projectM/presets";

#if defined(Q_OS_WIN32)
  paths.prepend(QCoreApplication::applicationDirPath() + "/projectm-presets");
#elif defined(Q_OS_MAC)
  paths.prepend(mac::GetResourcesPath() + "/projectm-presets");
#endif

  QString preset_path;
  foreach (const QString& path, paths) {
    if (QFile::exists(path)) {
      preset_path = path;
      break;
    }
  }

  if (preset_path.isNull()) {
    qFatal("ProjectM presets could not be found, search path was:\n  %s",
           paths.join("\n  ").toLocal8Bit().constData());
  }

  // Create projectM settings
  projectM::Settings s;
  s.meshX = 32;
  s.meshY = 24;
  s.textureSize = texture_size_;
  s.fps = 35;
  s.windowWidth = 512;
  s.windowHeight = 512;
  s.smoothPresetDuration = 10;
  s.presetDuration = 15;
  s.presetURL = preset_path.toStdString();
  s.shuffleEnabled = true;
  s.easterEgg = 0; // ??
  s.softCutRatingsEnabled = false;

  projectm_.reset(new projectM(s));
  Load();
}

void ProjectMVisualisation::drawBackground(QPainter* p, const QRectF&) {
  p->beginNativePainting();

  if (!projectm_) {
    InitProjectM();
  }

  projectm_->projectM_resetGL(sceneRect().width(), sceneRect().height());
  projectm_->renderFrame();

  p->endNativePainting();
}

void ProjectMVisualisation::SceneRectChanged(const QRectF &rect) {
  if (projectm_)
    projectm_->projectM_resetGL(rect.width(), rect.height());
}

void ProjectMVisualisation::SetTextureSize(int size) {
  texture_size_ = size;

  if (projectm_)
    projectm_->changeTextureSize(texture_size_);
}

void ProjectMVisualisation::ConsumeBuffer(GstBuffer *buffer, GstEnginePipeline*) {
  const int samples_per_channel = GST_BUFFER_SIZE(buffer) / sizeof(short) / 2;
  const short* data = reinterpret_cast<short*>(GST_BUFFER_DATA(buffer));

  if (projectm_)
    projectm_->pcm()->addPCM16Data(data, samples_per_channel);
  gst_buffer_unref(buffer);
}

void ProjectMVisualisation::set_selected(int preset, bool selected) {
  if (selected)
    selected_indices_.insert(preset);
  else
    selected_indices_.remove(preset);

  Save();
}

void ProjectMVisualisation::set_all_selected(bool selected) {
  selected_indices_.clear();
  if (selected) {
    int count = projectm_->getPlaylistSize();
    for (int i=0 ; i<count ; ++i) {
      selected_indices_ << i;
    }
  }
  Save();
}

void ProjectMVisualisation::Load() {
  QSettings s;
  s.beginGroup(VisualisationContainer::kSettingsGroup);
  QVariantList presets(s.value("presets").toList());

  int count = projectm_->getPlaylistSize();
  selected_indices_.clear();

  if (presets.isEmpty()) {
    for (int i=0 ; i<count ; ++i)
      selected_indices_ << i;
  } else {
    foreach (const QVariant& var, presets)
      if (var.toInt() < count)
        selected_indices_ << var.toInt();
  }

  mode_ = s.value("mode", 0).toInt();
}

void ProjectMVisualisation::Save() {
  QVariantList list;

  foreach (int index, selected_indices_.values()) {
    list << index;
  }

  QSettings s;
  s.beginGroup(VisualisationContainer::kSettingsGroup);
  s.setValue("presets", list);
  s.setValue("mode", mode_);
}

void ProjectMVisualisation::set_mode(int mode) {
  mode_ = mode;
  Save();
}

