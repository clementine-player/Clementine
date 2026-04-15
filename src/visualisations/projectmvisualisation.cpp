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

#include "projectmvisualisation.h"

#include "config.h"
#include "projectmpresetmodel.h"
#include "visualisationcontainer.h"

// GLEW must be included before any other OpenGL headers.
// Required on Windows for projectM 4.x up to 4.1.x.
#ifdef HAVE_GLEW_FOR_PROJECTM
#include <GL/glew.h>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGLWidget>
#include <QGraphicsView>
#include <QMessageBox>
#include <QPaintEngine>
#include <QPainter>
#include <QSettings>
#include <QTimerEvent>
#include <QtDebug>

#ifdef Q_OS_MAC
#include <OpenGL/gl.h>

#include "core/mac_startup.h"
#else
#include <GL/gl.h>
#endif

ProjectMVisualisation::ProjectMVisualisation(VisualisationContainer* container)
    : QGraphicsScene(container),
      projectm_(nullptr),
      playlist_(nullptr),
      preset_model_(nullptr),
      mode_(Random),
      duration_(15),
      texture_size_(512),
      pixel_ratio_(container->devicePixelRatio()),
      container_(container) {
  connect(this, SIGNAL(sceneRectChanged(QRectF)),
          SLOT(SceneRectChanged(QRectF)));
}

ProjectMVisualisation::~ProjectMVisualisation() {
  if (playlist_) {
    projectm_playlist_destroy(playlist_);
    playlist_ = nullptr;
  }
  if (projectm_) {
    projectm_destroy(projectm_);
    projectm_ = nullptr;
  }
}

void ProjectMVisualisation::InitProjectM() {
  // Find the projectM presets
  QStringList paths = QStringList()
#ifdef USE_INSTALL_PREFIX
                      << CMAKE_INSTALL_PREFIX
                      "/share/clementine/projectm-presets"
                      << CMAKE_INSTALL_PREFIX "/share/projectM/presets"
#endif
                      << "/usr/share/clementine/projectm-presets"
                      << "/usr/local/share/clementine/projectm-presets"
                      << "/usr/share/projectM/presets"
                      << "/usr/local/share/projectM/presets";

#if defined(Q_OS_WIN32)
  paths.prepend(QCoreApplication::applicationDirPath() + "/projectm-presets");
#elif defined(Q_OS_MAC)
  paths.prepend(mac::GetResourcesPath() + "/projectm-presets");
#endif

  for (const QString& path : paths) {
    if (!QFile::exists(path)) continue;

    // Don't use empty directories
    if (QDir(path)
            .entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)
            .isEmpty())
      continue;

    preset_path_ = path;
    break;
  }

#ifdef HAVE_GLEW_FOR_PROJECTM
  // projectM 4.1.x and earlier on Windows requires GLEW to be initialised
  // before the projectM instance is created.
  GLenum glew_err = glewInit();
  if (glew_err != GLEW_OK) {
    qWarning("GLEW initialization failed: %s",
             reinterpret_cast<const char*>(glewGetErrorString(glew_err)));
  }
#endif

  // Create projectM instance
  projectm_ = projectm_create();
  if (!projectm_) {
    qWarning("Failed to create projectM instance");
    return;
  }

  // Configure rendering parameters
  projectm_set_mesh_size(projectm_, 32, 24);
  projectm_set_fps(projectm_, 35);
  projectm_set_soft_cut_duration(projectm_, 5.0);
  projectm_set_preset_duration(projectm_, static_cast<double>(duration_));
  projectm_set_hard_cut_enabled(projectm_, false);

  size_t w = static_cast<size_t>(sceneRect().width() * pixel_ratio_);
  size_t h = static_cast<size_t>(sceneRect().height() * pixel_ratio_);
  if (w == 0) w = 512;
  if (h == 0) h = 512;
  projectm_set_window_size(projectm_, w, h);

  // Create playlist manager
  playlist_ = projectm_playlist_create(projectm_);
  projectm_playlist_set_shuffle(playlist_, true);

  preset_model_ = new ProjectMPresetModel(this, this);
  Load();

  // Start at a random preset.
  if (projectm_playlist_size(playlist_) > 0) {
    projectm_playlist_play_next(playlist_, true);
  }

  if (preset_path_.isNull()) {
    qWarning("ProjectM presets could not be found, search path was:\n  %s",
             paths.join("\n  ").toLocal8Bit().constData());
    QMessageBox::warning(
        nullptr, tr("Missing projectM presets"),
        tr("Clementine could not load any projectM visualisations.  Check that "
           "you have installed Clementine properly."));
  }
}

void ProjectMVisualisation::drawBackground(QPainter* p, const QRectF&) {
  p->beginNativePainting();

  if (!projectm_) {
    InitProjectM();
  }

  if (projectm_) {
    projectm_opengl_render_frame(projectm_);
  }

  p->endNativePainting();
}

void ProjectMVisualisation::SceneRectChanged(const QRectF& rect) {
  pixel_ratio_ = container_->devicePixelRatio();

  if (projectm_) {
    projectm_set_window_size(projectm_,
                             static_cast<size_t>(rect.width() * pixel_ratio_),
                             static_cast<size_t>(rect.height() * pixel_ratio_));
  }
}

void ProjectMVisualisation::SetTextureSize(int size) {
  texture_size_ = size;
  // projectM 4.x manages texture sizes internally
}

void ProjectMVisualisation::SetDuration(int seconds) {
  duration_ = seconds;

  if (projectm_)
    projectm_set_preset_duration(projectm_, static_cast<double>(duration_));

  Save();
}

void ProjectMVisualisation::ConsumeBuffer(GstBuffer* buffer, int) {
  GstMapInfo map;
  gst_buffer_map(buffer, &map, GST_MAP_READ);
  const unsigned int samples_per_channel = map.size / sizeof(int16_t) / 2;
  const int16_t* data = reinterpret_cast<const int16_t*>(map.data);

  if (projectm_) {
    projectm_pcm_add_int16(projectm_, data, samples_per_channel,
                           PROJECTM_STEREO);
  }

  gst_buffer_unmap(buffer, &map);
  gst_buffer_unref(buffer);
}

void ProjectMVisualisation::SetSelected(const QStringList& paths,
                                        bool selected) {
  if (!playlist_) return;

  for (const QString& path : paths) {
    int index = IndexOfPreset(path);
    if (selected && index == -1) {
      projectm_playlist_add_preset(playlist_, path.toUtf8().constData(), false);
    } else if (!selected && index != -1) {
      projectm_playlist_remove_preset(playlist_, static_cast<uint32_t>(index));
    }
  }

  Save();
}

void ProjectMVisualisation::ClearSelected() {
  if (playlist_) projectm_playlist_clear(playlist_);
  Save();
}

int ProjectMVisualisation::IndexOfPreset(const QString& path) const {
  if (!playlist_) return -1;

  uint32_t size = projectm_playlist_size(playlist_);
  for (uint32_t i = 0; i < size; ++i) {
    char* item = projectm_playlist_item(playlist_, i);
    if (item) {
      bool match = (QString::fromUtf8(item) == path);
      projectm_playlist_free_string(item);
      if (match) return static_cast<int>(i);
    }
  }
  return -1;
}

void ProjectMVisualisation::Load() {
  QSettings s;
  s.beginGroup(VisualisationContainer::kSettingsGroup);
  mode_ = Mode(s.value("mode", 0).toInt());
  duration_ = s.value("duration", duration_).toInt();

  if (projectm_)
    projectm_set_preset_duration(projectm_, static_cast<double>(duration_));

  if (playlist_) {
    projectm_playlist_clear(playlist_);
    switch (mode_) {
      case Random:
        for (int i = 0; i < preset_model_->all_presets_.count(); ++i) {
          projectm_playlist_add_preset(
              playlist_,
              preset_model_->all_presets_[i].path_.toUtf8().constData(), false);
          preset_model_->all_presets_[i].selected_ = true;
        }
        break;

      case FromList: {
        QStringList paths(s.value("preset_paths").toStringList());
        for (const QString& path : paths) {
          projectm_playlist_add_preset(playlist_, path.toUtf8().constData(),
                                       false);
          preset_model_->MarkSelected(path, true);
        }
      }
    }

    projectm_playlist_set_shuffle(playlist_, true);
  }
}

void ProjectMVisualisation::Save() {
  QStringList paths;

  for (const ProjectMPresetModel::Preset& preset :
       preset_model_->all_presets_) {
    if (preset.selected_) paths << preset.path_;
  }

  QSettings s;
  s.beginGroup(VisualisationContainer::kSettingsGroup);
  s.setValue("preset_paths", paths);
  s.setValue("mode", mode_);
  s.setValue("duration", duration_);
}

void ProjectMVisualisation::SetMode(Mode mode) {
  mode_ = mode;
  Save();
}

QString ProjectMVisualisation::preset_url() const { return preset_path_; }

void ProjectMVisualisation::SetImmediatePreset(const QString& path) {
  if (!projectm_) return;
  projectm_load_preset_file(projectm_, path.toUtf8().constData(), true);
}

void ProjectMVisualisation::Lock(bool lock) {
  if (!projectm_) return;
  projectm_set_preset_locked(projectm_, lock);

  if (!lock) Load();
}
