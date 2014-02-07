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

#include "generator.h"
#include "generatorinserter.h"
#include "core/taskmanager.h"
#include "playlist/playlist.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

namespace smart_playlists {

typedef QFuture<PlaylistItemList> Future;
typedef QFutureWatcher<PlaylistItemList> FutureWatcher;

GeneratorInserter::GeneratorInserter(TaskManager* task_manager,
                                     LibraryBackend* library, QObject* parent)
    : QObject(parent),
      task_manager_(task_manager),
      library_(library),
      task_id_(-1),
      is_dynamic_(false) {}

static PlaylistItemList Generate(GeneratorPtr generator, int dynamic_count) {
  if (dynamic_count) {
    return generator->GenerateMore(dynamic_count);
  } else {
    return generator->Generate();
  }
}

void GeneratorInserter::Load(Playlist* destination, int row, bool play_now,
                             bool enqueue, GeneratorPtr generator,
                             int dynamic_count) {
  task_id_ = task_manager_->StartTask(tr("Loading smart playlist"));

  destination_ = destination;
  row_ = row;
  play_now_ = play_now;
  enqueue_ = enqueue;
  is_dynamic_ = generator->is_dynamic();

  connect(generator.get(), SIGNAL(Error(QString)), SIGNAL(Error(QString)));

  Future future = QtConcurrent::run(Generate, generator, dynamic_count);
  FutureWatcher* watcher = new FutureWatcher(this);
  watcher->setFuture(future);

  connect(watcher, SIGNAL(finished()), SLOT(Finished()));
}

void GeneratorInserter::Finished() {
  FutureWatcher* watcher = static_cast<FutureWatcher*>(sender());
  watcher->deleteLater();

  PlaylistItemList items = watcher->result();

  if (items.isEmpty()) {
    if (is_dynamic_) {
      destination_->TurnOffDynamicPlaylist();
    }
  } else {
    destination_->InsertItems(items, row_, play_now_, enqueue_);
  }

  task_manager_->SetTaskFinished(task_id_);

  deleteLater();
}

}  // namespace
