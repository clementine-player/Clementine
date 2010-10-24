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

#ifndef SMARTPLAYLISTVIEW_H
#define SMARTPLAYLISTVIEW_H

#include "widgets/autoexpandingtreeview.h"

class QAction;
class QMenu;

class SmartPlaylistView : public AutoExpandingTreeView {
  Q_OBJECT

public:
  SmartPlaylistView(QWidget* parent = 0);

signals:
  void Play(const QModelIndex& index, bool as_new, bool clear);

protected:
  void contextMenuEvent(QContextMenuEvent*);

private slots:
  void Load();
  void AddToPlaylist();
  void AddAsNewPlaylist();
  void NewSmartPlaylist();
  void NewFolder();
  void Remove();

private:
  QMenu* menu_;
  QAction* load_action_;
  QAction* add_action_;
  QAction* add_as_new_action_;
  QAction* new_smart_action_;
  QAction* new_folder_action_;
  QAction* remove_action_;

  QModelIndex menu_index_;
};

#endif // SMARTPLAYLISTVIEW_H
