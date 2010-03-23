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

#ifndef RADIOVIEW_H
#define RADIOVIEW_H

#include <QTreeView>

class RadioView : public QTreeView {
  Q_OBJECT

 public:
  RadioView(QWidget* parent = 0);

  // QWidget
  void contextMenuEvent(QContextMenuEvent* e);
};

#endif // RADIOVIEW_H
