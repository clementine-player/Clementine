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

#include "playlistundocommands.h"
#include "playlist.h"

namespace PlaylistUndoCommands {

Base::Base(Playlist *playlist)
  : QUndoCommand(0),
    playlist_(playlist)
{
}


InsertItems::InsertItems(Playlist *playlist, const PlaylistItemList &items, int pos)
  : Base(playlist),
    items_(items),
    pos_(pos)
{
  setText(tr("add %n songs", "", items_.count()));
}

void InsertItems::redo() {
  playlist_->InsertItemsWithoutUndo(items_, pos_);
}

void InsertItems::undo() {
  const int start = pos_ == -1 ?
                    playlist_->rowCount() - items_.count() : pos_;
  playlist_->RemoveItemsWithoutUndo(start, items_.count());
}


RemoveItems::RemoveItems(Playlist *playlist, int pos, int count)
  : Base(playlist),
    pos_(pos),
    count_(count)
{
  setText(tr("remove %n songs", "", count_));
}

void RemoveItems::redo() {
  items_ = playlist_->RemoveItemsWithoutUndo(pos_, count_);
}

void RemoveItems::undo() {
  playlist_->InsertItemsWithoutUndo(items_, pos_);
}


MoveItems::MoveItems(Playlist *playlist, const QList<int> &source_rows, int pos)
  : Base(playlist),
    source_rows_(source_rows),
    pos_(pos)
{
  setText(tr("move songs", "", source_rows.count()));
}

void MoveItems::redo() {
  playlist_->MoveItemsWithoutUndo(source_rows_, pos_);
}

void MoveItems::undo() {
  playlist_->MoveItemsWithoutUndo(pos_, source_rows_);
}

} // namespace
