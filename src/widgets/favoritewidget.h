/* This file is part of Clementine.
   Copyright 2013, David Sansome <me@davidsansome.com>

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

#include <QWidget>

class QPaintEvent;
class QMouseEvent;

class FavoriteWidget : public QWidget {
  Q_OBJECT

 public:
  FavoriteWidget(int tab_id, bool favorite = false, QWidget* parent = nullptr);

  // Change the value if different from the current one and then update display
  // and emit FavoriteStateChanged signal
  void SetFavorite(bool favorite);

  QSize sizeHint() const;

signals:
  void FavoriteStateChanged(int, bool);

 protected:
  void paintEvent(QPaintEvent* e);
  void resizeEvent(QResizeEvent * e);
  void mouseReleaseEvent(QMouseEvent* e);

 private:
  static const int kStarSize;

  // The playlist's id this widget belongs to
  int tab_index_;
  bool favorite_;
  QPixmap on_;
  QPixmap off_;
  QRect rect_;
};
