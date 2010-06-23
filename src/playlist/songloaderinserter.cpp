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
#include "songloaderinserter.h"
#include "core/songloader.h"
#include "core/taskmanager.h"

SongLoaderInserter::SongLoaderInserter(TaskManager* task_manager, QObject *parent)
  : QObject(parent),
    task_manager_(task_manager),
    destination_(NULL),
    row_(-1),
    play_now_(true),
    async_load_id_(0),
    async_progress_(0)
{
}

SongLoaderInserter::~SongLoaderInserter() {
  qDeleteAll(pending_);
}

void SongLoaderInserter::Load(Playlist *destination, int row, bool play_now,
                              const QList<QUrl> &urls) {
  destination_ = destination;
  row_ = row;
  play_now_ = play_now;

  foreach (const QUrl& url, urls) {
    SongLoader* loader = new SongLoader(this);
    SongLoader::Result ret = loader->Load(url);

    if (ret == SongLoader::WillLoadAsync) {
      pending_.insert(loader);
      connect(loader, SIGNAL(LoadFinished(bool)), SLOT(PendingLoadFinished(bool)));
      continue;
    }

    if (ret == SongLoader::Success)
      songs_ << loader->songs();
    else
      emit Error(tr("Error loading %1").arg(url.toString()));
    delete loader;
  }

  if (pending_.isEmpty())
    Finished();
  else {
    async_progress_ = 0;
    async_load_id_ = task_manager_->StartTask(tr("Loading tracks"));
    task_manager_->SetTaskProgress(async_load_id_, async_progress_, pending_.count());
  }
}

void SongLoaderInserter::PendingLoadFinished(bool success) {
  SongLoader* loader = qobject_cast<SongLoader*>(sender());
  if (!loader || !pending_.contains(loader))
    return;
  pending_.remove(loader);

  if (success)
    songs_ << loader->songs();
  else
    emit Error(tr("Error loading %1").arg(loader->url().toString()));

  loader->deleteLater();

  task_manager_->SetTaskProgress(async_load_id_, ++async_progress_);
  if (pending_.isEmpty()) {
    task_manager_->SetTaskFinished(async_load_id_);
    Finished();
  }
}

void SongLoaderInserter::Finished() {
  QModelIndex index = destination_->InsertSongs(songs_, row_);
  if (play_now_)
    emit PlayRequested(index);

  deleteLater();
}
