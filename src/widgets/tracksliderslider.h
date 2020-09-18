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

#ifndef TRACKSLIDERSLIDER_H
#define TRACKSLIDERSLIDER_H

#include <QSlider>

class TrackSliderPopup;

// It's the slider inside the TrackSliderSlider
class TrackSliderSlider : public QSlider {
  Q_OBJECT

 public:
  TrackSliderSlider(QWidget* parent = nullptr);

 signals:
  void SeekForward();
  void SeekBackward();
  void Previous();
  void Next();

 protected:
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void wheelEvent(QWheelEvent* e);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void keyPressEvent(QKeyEvent* event);

 private slots:
  void UpdateDeltaTime();

 private:
  TrackSliderPopup* popup_;

  int mouse_hover_seconds_;
};

#endif  // TRACKSLIDERSLIDER_H
