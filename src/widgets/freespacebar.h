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

#ifndef FREESPACEBAR_H
#define FREESPACEBAR_H

#include <QWidget>

class FreeSpaceBar : public QWidget {
  Q_OBJECT
  Q_PROPERTY(quint64 free READ free_bytes WRITE set_free_bytes);
  Q_PROPERTY(quint64 total READ total_bytes WRITE set_total_bytes);

public:
  FreeSpaceBar(QWidget* parent = 0);

  static const int kBarHeight;
  static const int kBarBorderRadius;
  static const int kMarkerSpacing;

  static const QRgb kColorBg1;
  static const QRgb kColorBg2;
  static const QRgb kColorBar1;
  static const QRgb kColorBar2;
  static const QRgb kColorBorder;

  quint64 free_bytes() const { return free_; }
  quint64 total_bytes() const { return total_; }
  void set_free_bytes(quint64 bytes);
  void set_total_bytes(quint64 bytes);

  QSize sizeHint() const;

protected:
  void paintEvent(QPaintEvent*);

private:
  quint64 free_;
  quint64 total_;
};

#endif // FREESPACEBAR_H
