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

#ifndef PLAYLISTHEADER_H
#define PLAYLISTHEADER_H

#include "widgets/stretchheaderview.h"

class PlaylistView;

class QMenu;
class QSignalMapper;

class PlaylistHeader : public StretchHeaderView {
  Q_OBJECT

 public:
  PlaylistHeader(Qt::Orientation orientation, PlaylistView* view,
                 QWidget* parent = nullptr);

  // QWidget
  void contextMenuEvent(QContextMenuEvent* e);
  void enterEvent(QEvent*);

signals:
  void SectionVisibilityChanged(int logical, bool visible);
  void SectionRatingLockStatusChanged(bool state);
  void MouseEntered();

 private slots:
  void HideCurrent();
  void ToggleVisible(int section);
  void ToggleRatingEditStatus();
  void SetColumnAlignment(QAction* action);

 private:
  void AddColumnAction(int index);

 private:
  PlaylistView* view_;

  int menu_section_{};
  QMenu* menu_;
  QAction* hide_action_;
  QAction* rating_lock_;
  QAction* stretch_action_;
  QAction* align_left_action_;
  QAction* align_center_action_;
  QAction* align_right_action_;
  QList<QAction*> show_actions_;

  QSignalMapper* show_mapper_;
};

#endif  // PLAYLISTHEADER_H
