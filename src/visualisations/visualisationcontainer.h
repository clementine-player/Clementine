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

#ifndef VISUALISATIONCONTAINER_H
#define VISUALISATIONCONTAINER_H

#include <QGraphicsView>
#include <QBasicTimer>

#include "core/song.h"

class GstEngine;
class ProjectMVisualisation;
class VisualisationOverlay;

class VisualisationContainer : public QGraphicsView {
  Q_OBJECT

public:
  VisualisationContainer(QWidget* parent = 0);

  static const char* kSettingsGroup;
  static const int kDefaultWidth;
  static const int kDefaultHeight;
  static const int kDefaultFps;

  void SetEngine(GstEngine* engine);
  void SetActions(QAction* previous, QAction* play_pause,
                  QAction* stop, QAction* next);

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

private:
  void SizeChanged();

private slots:
  void ChangeOverlayOpacity(qreal value);

private:
  GstEngine* engine_;
  ProjectMVisualisation* vis_;
  VisualisationOverlay* overlay_;
  QBasicTimer update_timer_;

  QGraphicsProxyWidget* overlay_proxy_;

  int fps_;
};

#endif // VISUALISATIONCONTAINER_H
