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

#ifndef PLAYLISTUNDOCOMMANDS_H
#define PLAYLISTUNDOCOMMANDS_H

#include <QUndoCommand>
#include <QCoreApplication>

#include "playlistitem.h"

class Playlist;

namespace PlaylistUndoCommands {
enum Types { Type_RemoveItems = 0, };

class Base : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS(PlaylistUndoCommands)

 public:
  Base(Playlist* playlist);

 protected:
  Playlist* playlist_;
};

class InsertItems : public Base {
 public:
  InsertItems(Playlist* playlist, const PlaylistItemList& items, int pos,
              bool enqueue = false, bool enqueue_next = false);

  void undo();
  void redo();
  // When load is async, items have already been pushed, so we need to update
  // them.
  // This function try to find the equivalent item, and replace it with the
  // new (completely loaded) one.
  // return true if the was found (and updated), false otherwise
  bool UpdateItem(const PlaylistItemPtr& updated_item);

 private:
  PlaylistItemList items_;
  int pos_;
  bool enqueue_;
  bool enqueue_next_;
};

class RemoveItems : public Base {
 public:
  RemoveItems(Playlist* playlist, int pos, int count);

  int id() const { return Type_RemoveItems; }

  void undo();
  void redo();
  bool mergeWith(const QUndoCommand* other);

 private:
  struct Range {
    Range(int pos, int count) : pos_(pos), count_(count) {}
    int pos_;
    int count_;
    PlaylistItemList items_;
  };

  QList<Range> ranges_;
};

class MoveItems : public Base {
 public:
  MoveItems(Playlist* playlist, const QList<int>& source_rows, int pos);

  void undo();
  void redo();

 private:
  QList<int> source_rows_;
  int pos_;
};

class ReOrderItems : public Base {
 public:
  ReOrderItems(Playlist* playlist, const PlaylistItemList& new_items);

  void undo();
  void redo();

 private:
  PlaylistItemList old_items_;
  PlaylistItemList new_items_;
};

class SortItems : public ReOrderItems {
 public:
  SortItems(Playlist* playlist, int column, Qt::SortOrder order,
            const PlaylistItemList& new_items);

 private:
  int column_;
  Qt::SortOrder order_;
};

class ShuffleItems : public ReOrderItems {
 public:
  ShuffleItems(Playlist* playlist, const PlaylistItemList& new_items);
};
}  // namespace

#endif  // PLAYLISTUNDOCOMMANDS_H
