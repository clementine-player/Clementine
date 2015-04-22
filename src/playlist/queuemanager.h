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

#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <QWidget>

class Playlist;
class PlaylistManager;
class Ui_QueueManager;

class QModelIndex;

class QueueManager : public QWidget {
  Q_OBJECT

 public:
  QueueManager(QWidget* parent = nullptr);
  ~QueueManager();

  void SetPlaylistManager(PlaylistManager* manager);

 private slots:
  void CurrentPlaylistChanged(Playlist* playlist);
  void PlaylistDestroyed();
  void UpdateButtonState();

  void MoveUp();
  void MoveDown();
  void Remove();
  void Clear();

 private:
  Ui_QueueManager* ui_;

  PlaylistManager* playlists_;
  Playlist* current_playlist_;
};

#endif  // QUEUEMANAGER_H
