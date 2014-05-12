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

#ifndef SONGPLAYSTATS_H
#define SONGPLAYSTATS_H

#include <QIcon>
#include <QWidget>

class SongPlayStats : public QWidget {
  Q_OBJECT

 public:
  SongPlayStats(QWidget* parent = nullptr);

  static const int kIconSize;
  static const int kLineSpacing;
  static const int kIconTextSpacing;
  static const int kMargin;

  void AddItem(const QIcon& icon, const QString& text);

  QSize sizeHint() const;

 protected:
  void paintEvent(QPaintEvent*);

 private:
  struct Item {
    Item(const QIcon& icon, const QString& text) : icon_(icon), text_(text) {}
    QIcon icon_;
    QString text_;
  };

  QList<Item> items_;
};

#endif  // SONGPLAYSTATS_H
