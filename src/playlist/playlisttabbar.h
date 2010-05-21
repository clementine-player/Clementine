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

#ifndef PLAYLISTTABBAR_H
#define PLAYLISTTABBAR_H

#include <QTabBar>

class QMenu;

class PlaylistTabBar : public QTabBar {
  Q_OBJECT

public:
  PlaylistTabBar(QWidget *parent = 0);

  void SetActions(QAction* new_playlist, QAction* save_playlist,
                  QAction* load_playlist);

  // We use IDs to refer to tabs so the tabs can be moved around (and their
  // indexes change).
  int index_of(int id) const;
  int current_id() const;

  // Utility functions that use IDs rather than indexes
  void set_current_id(int id);
  void set_icon_by_id(int id, const QIcon& icon);
  void set_text_by_id(int id, const QString& text);

  void RemoveTab(int id);
  void InsertTab(int id, int index, const QString& text);

signals:
  void CurrentIdChanged(int id);
  void Rename(int id, const QString& name);
  void Remove(int id);
  void PlaylistOrderChanged(const QList<int>& ids);

private slots:
  void CurrentIndexChanged(int index);
  void Rename();
  void Remove();
  void TabMoved();

protected:
  void contextMenuEvent(QContextMenuEvent *);

private:
  QMenu* menu_;
  int menu_index_;
  QAction* rename_;
  QAction* remove_;

  bool suppress_current_changed_;
};

#endif // PLAYLISTTABBAR_H
