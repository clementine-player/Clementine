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

#ifndef VISUALISATIONOVERLAY_H
#define VISUALISATIONOVERLAY_H

#include <QWidget>
#include <QBasicTimer>

class Ui_VisualisationOverlay;

class QTimeLine;

class VisualisationOverlay : public QWidget {
  Q_OBJECT

 public:
  VisualisationOverlay(QWidget* parent = nullptr);
  ~VisualisationOverlay();

  static const int kFadeDuration;
  static const int kFadeTimeout;

  QGraphicsProxyWidget* title(QGraphicsProxyWidget* proxy) const;

  void SetActions(QAction* previous, QAction* play_pause, QAction* stop,
                  QAction* next);
  void SetSongTitle(const QString& title);

 public slots:
  void SetVisible(bool visible);

signals:
  void OpacityChanged(qreal value);
  void ShowPopupMenu(const QPoint& pos);

 protected:
  // QWidget
  void timerEvent(QTimerEvent*);

 private slots:
  void ShowSettingsMenu();

 private:
  Ui_VisualisationOverlay* ui_;

  QTimeLine* fade_timeline_;
  QBasicTimer fade_out_timeout_;
  bool visible_;
};

#endif  // VISUALISATIONOVERLAY_H
