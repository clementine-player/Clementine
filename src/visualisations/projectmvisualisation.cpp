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

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGLWidget>
#include <QGraphicsView>
#include <QMessageBox>
#include <QPaintEngine>
#include <QPainter>
#include <QSettings>
#include <QTemporaryFile>
#include <QTimerEvent>
#include <QtDebug>

#include "config.h"
#include "projectmpresetmodel.h"
#include "visualisationcontainer.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

#ifdef USE_SYSTEM_PROJECTM
#include <libprojectM/projectM.hpp>
#else
#include "projectM.hpp"
#endif

#ifdef Q_OS_MAC
#include <OpenGL/gl.h>

#include "core/mac_startup.h"
#else
#include <GL/gl.h>
#endif

ProjectMVisualisation::ProjectMVisualisation(VisualisationContainer* container)
    : QGraphicsScene(container),
      preset_model_(nullptr),
      mode_(Random),
      duration_(15),
      texture_size_(512),
      pixel_ratio_(container->devicePixelRatio()),
      container_(container) {
  connect(this, SIGNAL(sceneRectChanged(QRectF)),
          SLOT(SceneRectChanged(QRectF)));

  for (int i = 0; i < TOTAL_RATING_TYPES; ++i)
    default_rating_list_.push_back(3);
}

ProjectMVisualisation::~ProjectMVisualisation() {}

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

  QString preset_path;
  for (const QString& path : paths) {
    if (!QFile::exists(path)) continue;

    // Don't use empty directories
    if (QDir(path).entryList(QDir::Files | QDir::NoDotAndDotDot).isEmpty())
      continue;

    preset_path = path;
    break;
  }

  // Write an empty font out to a temporary directory.  libprojectM dies if it's
  // compiled with FTGL support and you pass it an empty font URL, so we have
  // to give it a dummy font even though we won't use it.
  temporary_font_.reset(QTemporaryFile::createLocalFile(":blank.ttf"));
  const QString font_path = temporary_font_->fileName();

  // Create projectM settings
  projectM::Settings s;
  s.meshX = 32;
  s.meshY = 24;
  s.textureSize = texture_size_;
  s.fps = 35;
  s.windowWidth = 512;
  s.windowHeight = 512;
  s.smoothPresetDuration = 5;
  s.presetDuration = duration_;
  s.presetURL = preset_path.toStdString();
  s.shuffleEnabled = true;
  s.easterEgg = 0;  // ??
  s.softCutRatingsEnabled = false;
  s.menuFontURL = font_path.toStdString();
  s.titleFontURL = font_path.toStdString();

  projectm_.reset(new projectM(s));
  preset_model_ = new ProjectMPresetModel(this, this);
  Load();

  // Start at a random preset.
  if (projectm_->getPlaylistSize() > 0) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    int selection = qrand() % projectm_->getPlaylistSize();
#else
    int selection =
        QRandomGenerator::global()->bounded(projectm_->getPlaylistSize());
#endif
    projectm_->selectPreset(selection, true);
  }

  if (font_path.isNull()) {
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

  projectm_->projectM_resetGL(sceneRect().width() * pixel_ratio_,
                              sceneRect().height() * pixel_ratio_);
  projectm_->renderFrame();

  p->endNativePainting();
}

void ProjectMVisualisation::SceneRectChanged(const QRectF& rect) {
  // NOTE: This should be updated on a QScreen dpi change signal. Accessing the
  // QScreen becomes a lot easier in Qt 5.14 with QWidget::screen().
  pixel_ratio_ = container_->devicePixelRatio();

  if (projectm_)
    projectm_->projectM_resetGL(rect.width() * pixel_ratio_,
                                rect.height() * pixel_ratio_);
}

void ProjectMVisualisation::SetTextureSize(int size) {
  texture_size_ = size;

  if (projectm_) projectm_->changeTextureSize(texture_size_);
}

void ProjectMVisualisation::SetDuration(int seconds) {
  duration_ = seconds;

  if (projectm_) projectm_->changePresetDuration(duration_);

  Save();
}

void ProjectMVisualisation::ConsumeBuffer(GstBuffer* buffer, int) {
  GstMapInfo map;
  gst_buffer_map(buffer, &map, GST_MAP_READ);
  const int samples_per_channel = map.size / sizeof(short) / 2;
  const short* data = reinterpret_cast<short*>(map.data);

  if (projectm_) {
    projectm_->pcm()->addPCM16Data(data, samples_per_channel);
  }

  gst_buffer_unmap(buffer, &map);
  gst_buffer_unref(buffer);
}

void ProjectMVisualisation::SetSelected(const QStringList& paths,
                                        bool selected) {
  for (const QString& path : paths) {
    int index = IndexOfPreset(path);
    if (selected && index == -1) {
      projectm_->addPresetURL(path.toStdString(), std::string(),
                              default_rating_list_);
    } else if (!selected && index != -1) {
      projectm_->removePreset(index);
    }
  }

  Save();
}

void ProjectMVisualisation::ClearSelected() {
  projectm_->clearPlaylist();
  Save();
}

int ProjectMVisualisation::IndexOfPreset(const QString& path) const {
  for (uint i = 0; i < projectm_->getPlaylistSize(); ++i) {
    if (QString::fromStdString(projectm_->getPresetURL(i)) == path) return i;
  }
  return -1;
}

void ProjectMVisualisation::Load() {
  QSettings s;
  s.beginGroup(VisualisationContainer::kSettingsGroup);
  mode_ = Mode(s.value("mode", 0).toInt());
  duration_ = s.value("duration", duration_).toInt();

  projectm_->changePresetDuration(duration_);
  projectm_->clearPlaylist();
  switch (mode_) {
    case Random:
      for (int i = 0; i < preset_model_->all_presets_.count(); ++i) {
        projectm_->addPresetURL(
            preset_model_->all_presets_[i].path_.toStdString(), std::string(),
            default_rating_list_);
        preset_model_->all_presets_[i].selected_ = true;
      }
      break;

    case FromList: {
      QStringList paths(s.value("preset_paths").toStringList());
      for (const QString& path : paths) {
        projectm_->addPresetURL(path.toStdString(), std::string(),
                                default_rating_list_);
        preset_model_->MarkSelected(path, true);
      }
    }
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

QString ProjectMVisualisation::preset_url() const {
  return QString::fromStdString(projectm_->settings().presetURL);
}

void ProjectMVisualisation::SetImmediatePreset(const QString& path) {
  int index = IndexOfPreset(path);
  if (index == -1) {
    index = projectm_->addPresetURL(path.toStdString(), std::string(),
                                    default_rating_list_);
  }

  projectm_->selectPreset(index, true);
}

void ProjectMVisualisation::Lock(bool lock) {
  projectm_->setPresetLock(lock);

  if (!lock) Load();
}
