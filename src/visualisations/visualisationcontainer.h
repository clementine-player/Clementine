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

#ifndef VISUALISATIONCONTAINER_H
#define VISUALISATIONCONTAINER_H

#include <QGraphicsView>
#include <QBasicTimer>

#include "core/song.h"

class GstEngine;
class ProjectMVisualisation;
class VisualisationOverlay;
class VisualisationSelector;

class QMenu;
class QSignalMapper;
class QActionGroup;

class VisualisationContainer : public QGraphicsView {
  Q_OBJECT

 public:
  VisualisationContainer(QWidget* parent = nullptr);

  static const int kLowFramerate;
  static const int kMediumFramerate;
  static const int kHighFramerate;
  static const int kSuperHighFramerate;

  static const char* kSettingsGroup;
  static const int kDefaultWidth;
  static const int kDefaultHeight;
  static const int kDefaultFps;
  static const int kDefaultTextureSize;

  void SetEngine(GstEngine* engine);
  void SetActions(QAction* previous, QAction* play_pause, QAction* stop,
                  QAction* next);

 public slots:
  void SongMetadataChanged(const Song& metadata);
  void Stopped();

 protected:
  // QWidget
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent* e);
  void resizeEvent(QResizeEvent* e);
  void timerEvent(QTimerEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void enterEvent(QEvent* e);
  void leaveEvent(QEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  void contextMenuEvent(QContextMenuEvent* event);
  void keyReleaseEvent(QKeyEvent* event);

 private:
  void Init();

  void SizeChanged();
  void AddMenuItem(const QString& name, int value, int def, QActionGroup* group,
                   QSignalMapper* mapper);

 private slots:
  void ChangeOverlayOpacity(qreal value);
  void ShowPopupMenu(const QPoint& pos);
  void ToggleFullscreen();
  void SetFps(int fps);
  void SetQuality(int size);

 private:
  bool initialised_;

  GstEngine* engine_;
  ProjectMVisualisation* vis_;
  VisualisationOverlay* overlay_;
  QBasicTimer update_timer_;

  VisualisationSelector* selector_;

  QGraphicsProxyWidget* overlay_proxy_;

  QMenu* menu_;

  int fps_;
  int size_;
};

#endif  // VISUALISATIONCONTAINER_H
