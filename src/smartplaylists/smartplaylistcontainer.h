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

#ifndef SMARTPLAYLISTCONTAINER_H
#define SMARTPLAYLISTCONTAINER_H

#include "playlistgenerator_fwd.h"

#include <QWidget>

class LibraryBackend;
class PlaylistManager;
class SmartPlaylistModel;
class Ui_SmartPlaylistContainer;

class QModelIndex;

class SmartPlaylistContainer : public QWidget {
  Q_OBJECT

public:
  SmartPlaylistContainer(QWidget* parent);
  ~SmartPlaylistContainer();

  void set_library(LibraryBackend* library) { library_ = library; }
  void set_playlists(PlaylistManager* playlist_manager) { playlist_manager_ = playlist_manager; }

protected:
  void showEvent(QShowEvent*);

private slots:
  void Play(const QModelIndex& index, bool as_new, bool clear);

private:
  Ui_SmartPlaylistContainer* ui_;
  bool first_show_;

  LibraryBackend* library_;
  SmartPlaylistModel* model_;
  PlaylistManager* playlist_manager_;
};

#endif // SMARTPLAYLISTCONTAINER_H
