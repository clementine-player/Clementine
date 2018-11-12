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

#include "playlist.h"
#include "playlistdelegates.h"
#include "playlistmanager.h"
#include "queue.h"
#include "queuemanager.h"
#include "ui_queuemanager.h"
#include "ui/iconloader.h"

#include <algorithm>

#include <QKeySequence>
#include <QShortcut>

QueueManager::QueueManager(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_QueueManager),
      playlists_(nullptr),
      current_playlist_(nullptr) {
  ui_->setupUi(this);
  ui_->list->setItemDelegate(new QueuedItemDelegate(this, 0));

  // Set icons on buttons
  ui_->move_down->setIcon(IconLoader::Load("go-down", IconLoader::Base));
  ui_->move_up->setIcon(IconLoader::Load("go-up", IconLoader::Base));
  ui_->remove->setIcon(IconLoader::Load("edit-delete", IconLoader::Base));
  ui_->clear->setIcon(IconLoader::Load("edit-clear-list", IconLoader::Base));

  // Set a standard shortcut
  ui_->remove->setShortcut(QKeySequence::Delete);

  // Button connections
  connect(ui_->move_down, SIGNAL(clicked()), SLOT(MoveDown()));
  connect(ui_->move_up, SIGNAL(clicked()), SLOT(MoveUp()));
  connect(ui_->remove, SIGNAL(clicked()), SLOT(Remove()));
  connect(ui_->clear, SIGNAL(clicked()), SLOT(Clear()));

  QShortcut* close = new QShortcut(QKeySequence::Close, this);
  connect(close, SIGNAL(activated()), SLOT(close()));
}

QueueManager::~QueueManager() { delete ui_; }

void QueueManager::SetPlaylistManager(PlaylistManager* manager) {
  playlists_ = manager;

  connect(playlists_, SIGNAL(CurrentChanged(Playlist*)),
          SLOT(CurrentPlaylistChanged(Playlist*)));
  CurrentPlaylistChanged(playlists_->current());
}

void QueueManager::CurrentPlaylistChanged(Playlist* playlist) {
  if (current_playlist_) {
    disconnect(current_playlist_->queue(),
               SIGNAL(rowsInserted(QModelIndex, int, int)), this,
               SLOT(UpdateButtonState()));
    disconnect(current_playlist_->queue(),
               SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
               SLOT(UpdateButtonState()));
    disconnect(current_playlist_->queue(), SIGNAL(layoutChanged()), this,
               SLOT(UpdateButtonState()));
    disconnect(current_playlist_->queue(), SIGNAL(SummaryTextChanged(QString)),
               ui_->queue_summary, SLOT(setText(QString)));
    disconnect(current_playlist_, SIGNAL(destroyed()), this,
               SLOT(PlaylistDestroyed()));
  }

  current_playlist_ = playlist;

  connect(current_playlist_->queue(),
          SIGNAL(rowsInserted(QModelIndex, int, int)), this,
          SLOT(UpdateButtonState()));
  connect(current_playlist_->queue(),
          SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
          SLOT(UpdateButtonState()));
  connect(current_playlist_->queue(), SIGNAL(layoutChanged()), this,
          SLOT(UpdateButtonState()));
  connect(current_playlist_->queue(), SIGNAL(SummaryTextChanged(QString)),
          ui_->queue_summary, SLOT(setText(QString)));
  connect(current_playlist_, SIGNAL(destroyed()), this,
          SLOT(PlaylistDestroyed()));

  ui_->list->setModel(current_playlist_->queue());

  connect(ui_->list->selectionModel(),
          SIGNAL(currentChanged(QModelIndex, QModelIndex)),
          SLOT(UpdateButtonState()));

  QTimer::singleShot(0, current_playlist_->queue(), SLOT(UpdateSummaryText()));
}

void QueueManager::MoveUp() {
  QModelIndexList indexes = ui_->list->selectionModel()->selectedRows();
  std::stable_sort(indexes.begin(), indexes.end());

  if (indexes.isEmpty() || indexes.first().row() == 0) return;

  for (const QModelIndex& index : indexes) {
    current_playlist_->queue()->MoveUp(index.row());
  }
}

void QueueManager::MoveDown() {
  QModelIndexList indexes = ui_->list->selectionModel()->selectedRows();
  std::stable_sort(indexes.begin(), indexes.end());

  if (indexes.isEmpty() ||
      indexes.last().row() == current_playlist_->queue()->rowCount() - 1)
    return;

  for (int i = indexes.count() - 1; i >= 0; --i) {
    current_playlist_->queue()->MoveDown(indexes[i].row());
  }
}

void QueueManager::Clear() { current_playlist_->queue()->Clear(); }

void QueueManager::Remove() {
  // collect the rows to be removed
  QList<int> row_list;
  for (const QModelIndex& index : ui_->list->selectionModel()->selectedRows()) {
    if (index.isValid()) row_list << index.row();
  }

  current_playlist_->queue()->Remove(row_list);
}

void QueueManager::UpdateButtonState() {
  const QModelIndex current = ui_->list->selectionModel()->currentIndex();

  if (current.isValid()) {
    ui_->move_up->setEnabled(current.row() != 0);
    ui_->move_down->setEnabled(current.row() !=
                               current_playlist_->queue()->rowCount() - 1);
    ui_->remove->setEnabled(true);
  } else {
    ui_->move_up->setEnabled(false);
    ui_->move_down->setEnabled(false);
    ui_->remove->setEnabled(false);
  }

  ui_->clear->setEnabled(!current_playlist_->queue()->is_empty());
}

void QueueManager::PlaylistDestroyed() {
  current_playlist_ = nullptr;
  // We'll get another CurrentPlaylistChanged() soon
}
