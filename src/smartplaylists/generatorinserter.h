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

#ifndef PLAYLISTGENERATORINSERTER_H
#define PLAYLISTGENERATORINSERTER_H

#include "generator_fwd.h"

#include <QFuture>
#include <QObject>

#include "playlist/playlist.h"

class LibraryBackend;
class Playlist;
class TaskManager;

class QModelIndex;

namespace smart_playlists {

class GeneratorInserter : public QObject {
  Q_OBJECT

 public:
  GeneratorInserter(TaskManager* task_manager, LibraryBackend* library,
                    QObject* parent);

  void Load(Playlist* destination, int row, bool play_now, bool enqueue, bool enqueue_next,
            GeneratorPtr generator, int dynamic_count = 0);

signals:
  void Error(const QString& message);
  void PlayRequested(const QModelIndex& index);

 private slots:
  void Finished(QFuture<PlaylistItemList> future);

 private:
  TaskManager* task_manager_;
  LibraryBackend* library_;
  int task_id_;

  Playlist* destination_;
  int row_;
  bool play_now_;
  bool enqueue_;
  bool enqueue_next_;
  bool is_dynamic_;
};

}  // namespace

#endif  // PLAYLISTGENERATORINSERTER_H
