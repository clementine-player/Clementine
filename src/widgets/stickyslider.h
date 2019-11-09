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

#ifndef STICKYSLIDER_H
#define STICKYSLIDER_H

#include <QSlider>

class StickySlider : public QSlider {
  Q_OBJECT
  Q_PROPERTY(int sticky_center READ sticky_center WRITE set_sticky_center)
  Q_PROPERTY(int sticky_threshold READ sticky_threshold WRITE
                 set_sticky_threshold)

 public:
  StickySlider(QWidget* parent = nullptr);

  int sticky_center() const { return sticky_center_; }
  int sticky_threshold() const { return sticky_threshold_; }
  void set_sticky_center(int center) { sticky_center_ = center; }
  void set_sticky_threshold(int threshold) { sticky_threshold_ = threshold; }

 protected:
  void mouseMoveEvent(QMouseEvent* e);

 private:
  int sticky_center_;
  int sticky_threshold_;
};

#endif  // STICKYSLIDER_H
