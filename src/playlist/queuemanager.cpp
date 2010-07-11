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

#include "playlist.h"
#include "playlistdelegates.h"
#include "playlistmanager.h"
#include "queue.h"
#include "queuemanager.h"
#include "ui_queuemanager.h"

QueueManager::QueueManager(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_QueueManager),
    playlists_(NULL),
    current_playlist_(NULL)
{
  ui_->setupUi(this);

  ui_->list->setItemDelegate(new QueuedItemDelegate(this));
}

QueueManager::~QueueManager() {
  delete ui_;
}

void QueueManager::SetPlaylistManager(PlaylistManager* manager) {
  playlists_ = manager;

  connect(playlists_, SIGNAL(CurrentChanged(Playlist*)), SLOT(CurrentPlaylistChanged(Playlist*)));
}

void QueueManager::CurrentPlaylistChanged(Playlist* playlist) {
  current_playlist_ = playlist;
  ui_->list->setModel(current_playlist_->queue());
  ui_->list->setModelColumn(Queue::Column_CombinedArtistTitle);
}
