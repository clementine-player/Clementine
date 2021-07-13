/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "deletefiles.h"

#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QUrl>

#include "musicstorage.h"
#include "taskmanager.h"

const int DeleteFiles::kBatchSize = 50;

DeleteFiles::DeleteFiles(TaskManager* task_manager,
                         std::shared_ptr<MusicStorage> storage)
    : thread_(nullptr),
      task_manager_(task_manager),
      storage_(storage),
      started_(false),
      task_id_(0),
      progress_(0) {
  original_thread_ = thread();
}

DeleteFiles::~DeleteFiles() {}

void DeleteFiles::Start(const SongList& songs) {
  if (thread_) return;

  songs_ = songs;

  task_id_ = task_manager_->StartTask(tr("Deleting files"));
  task_manager_->SetTaskBlocksLibraryScans(task_id_);

  thread_ = new QThread;
  connect(thread_, SIGNAL(started()), SLOT(ProcessSomeFiles()));

  moveToThread(thread_);
  thread_->start();
}

void DeleteFiles::Start(const QUrl& url) {
  SongList songs;
  Song song;
  song.set_url(url);
  songs << song;

  Start(songs);
}

void DeleteFiles::Start(const QStringList& filenames) {
  SongList songs;
  for (const QString& filename : filenames) {
    Song song;
    song.set_url(QUrl::fromLocalFile(filename));
    songs << song;
  }

  Start(songs);
}

void DeleteFiles::ProcessSomeFiles() {
  if (!started_) {
    storage_->StartDelete();
    started_ = true;
  }

  // None left?
  if (progress_ >= songs_.count()) {
    task_manager_->SetTaskProgress(task_id_, progress_, songs_.count());

    storage_->FinishCopy(songs_with_errors_.isEmpty());

    task_manager_->SetTaskFinished(task_id_);

    emit Finished(songs_with_errors_);

    // Move back to the original thread so deleteLater() can get called in
    // the main thread's event loop
    moveToThread(original_thread_);
    deleteLater();

    // Stop this thread
    thread_->quit();
    return;
  }

  // We process files in batches so we can be cancelled part-way through.

  const int n = qMin(songs_.count(), progress_ + kBatchSize);
  for (; progress_ < n; ++progress_) {
    task_manager_->SetTaskProgress(task_id_, progress_, songs_.count());

    const Song& song = songs_[progress_];

    MusicStorage::DeleteJob job;
    job.metadata_ = song;

    if (!storage_->DeleteFromStorage(job)) {
      songs_with_errors_ << song;
    }
  }

  QTimer::singleShot(0, this, SLOT(ProcessSomeFiles()));
}
