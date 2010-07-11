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
#include "ui/iconloader.h"

QueueManager::QueueManager(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_QueueManager),
    playlists_(NULL),
    current_playlist_(NULL)
{
  ui_->setupUi(this);
  ui_->list->setItemDelegate(new QueuedItemDelegate(this));

  // Set icons on buttons
  ui_->move_down->setIcon(IconLoader::Load("go-down"));
  ui_->move_up->setIcon(IconLoader::Load("go-up"));
  ui_->clear->setIcon(IconLoader::Load("edit-clear-list"));

  // Button connections
  connect(ui_->move_down, SIGNAL(clicked()), SLOT(MoveDown()));
  connect(ui_->move_up, SIGNAL(clicked()), SLOT(MoveUp()));
  connect(ui_->clear, SIGNAL(clicked()), SLOT(Clear()));
}

QueueManager::~QueueManager() {
  delete ui_;
}

void QueueManager::SetPlaylistManager(PlaylistManager* manager) {
  playlists_ = manager;

  connect(playlists_, SIGNAL(CurrentChanged(Playlist*)), SLOT(CurrentPlaylistChanged(Playlist*)));
}

void QueueManager::CurrentPlaylistChanged(Playlist* playlist) {
  if (current_playlist_) {
    disconnect(current_playlist_->queue(), SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(UpdateButtonState()));
    disconnect(current_playlist_->queue(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
               this, SLOT(UpdateButtonState()));
  }

  current_playlist_ = playlist;

  connect(current_playlist_->queue(), SIGNAL(rowsInserted(QModelIndex,int,int)),
          this, SLOT(UpdateButtonState()));
  connect(current_playlist_->queue(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
          this, SLOT(UpdateButtonState()));

  ui_->list->setModel(current_playlist_->queue());
  ui_->list->setModelColumn(Queue::Column_CombinedArtistTitle);

  connect(ui_->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(UpdateButtonState()));
}

void QueueManager::MoveUp() {

}

void QueueManager::MoveDown() {

}

void QueueManager::Clear() {
  current_playlist_->queue()->Clear();
}

void QueueManager::UpdateButtonState() {
  const QModelIndex current = ui_->list->selectionModel()->currentIndex();

  if (current.isValid()) {
    ui_->move_up->setEnabled(current.row() != 0);
    ui_->move_down->setEnabled(current.row() != current_playlist_->queue()->rowCount()-1);
  } else {
    ui_->move_up->setEnabled(false);
    ui_->move_down->setEnabled(false);
  }

  ui_->clear->setEnabled(!current_playlist_->queue()->is_empty());
}
