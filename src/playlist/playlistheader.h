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

#ifndef PLAYLISTHEADER_H
#define PLAYLISTHEADER_H

#include "widgets/stretchheaderview.h"

class QMenu;
class QSignalMapper;

class PlaylistHeader : public StretchHeaderView {
  Q_OBJECT

 public:
  PlaylistHeader(Qt::Orientation orientation, QWidget* parent = 0);

  // QWidget
  void contextMenuEvent(QContextMenuEvent* e);
  void enterEvent(QEvent*);

 signals:
  void SectionVisibilityChanged(int logical, bool visible);
  void MouseEntered();

 private slots:
  void HideCurrent();
  void ToggleVisible(int section);

 private:
  void AddColumnAction(int index);

 private:
  int menu_section_;
  QMenu* menu_;
  QAction* hide_action_;
  QAction* stretch_action_;
  QList<QAction*> show_actions_;

  QSignalMapper* show_mapper_;
};

#endif // PLAYLISTHEADER_H
