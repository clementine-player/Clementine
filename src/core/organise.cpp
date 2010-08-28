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

#include "musicstorage.h"
#include "organise.h"
#include "taskmanager.h"

#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QThread>

#include <boost/bind.hpp>

const int Organise::kBatchSize = 10;

Organise::Organise(TaskManager* task_manager,
                   boost::shared_ptr<MusicStorage> destination,
                   const OrganiseFormat &format, bool copy, bool overwrite,
                   const QStringList& files, bool eject_after)
                     : thread_(NULL),
                       task_manager_(task_manager),
                       destination_(destination),
                       format_(format),
                       copy_(copy),
                       overwrite_(overwrite),
                       files_(files),
                       eject_after_(eject_after),
                       started_(false),
                       task_id_(0),
                       progress_(0),
                       song_progress_(0)
{
  original_thread_ = thread();
}

void Organise::Start() {
  if (thread_)
    return;

  task_id_ = task_manager_->StartTask(tr("Organising files"));
  task_manager_->SetTaskBlocksLibraryScans(true);

  thread_ = new QThread;
  connect(thread_, SIGNAL(started()), SLOT(ProcessSomeFiles()));

  moveToThread(thread_);
  thread_->start();
}

void Organise::ProcessSomeFiles() {
  if (!started_) {
    destination_->StartCopy();
    started_ = true;
  }

  // None left?
  if (progress_ >= files_.count()) {
    UpdateProgress();

    destination_->FinishCopy(files_with_errors_.isEmpty());
    if (eject_after_)
      destination_->Eject();

    task_manager_->SetTaskFinished(task_id_);

    emit Finished(files_with_errors_);

    // Move back to the original thread so deleteLater() can get called in
    // the main thread's event loop
    moveToThread(original_thread_);
    deleteLater();

    // Stop this thread
    thread_->quit();
    return;
  }

  QDir dir;

  // We process files in batches so we can be cancelled part-way through.

  const int n = qMin(files_.count(), progress_ + kBatchSize);
  for ( ; progress_<n ; ++progress_) {
    SetSongProgress(0);

    const QString filename = files_[progress_];

    // Is it a directory?
    if (QFileInfo(filename).isDir()) {
      QDir dir(filename);
      foreach (const QString& entry, dir.entryList(
          QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable)) {
        files_ << filename + "/" + entry;
      }
      continue;
    }

    // Read metadata from the file
    Song song;
    song.InitFromFile(filename, -1);
    if (!song.is_valid())
      continue;

    MusicStorage::CopyJob job;
    job.source_ = filename;
    job.destination_ = format_.GetFilenameForSong(song);
    job.metadata_ = song;
    job.overwrite_ = overwrite_;
    job.remove_original_ = !copy_;
    job.progress_ = boost::bind(&Organise::SetSongProgress, this, _1);

    if (!destination_->CopyToStorage(job)) {
      files_with_errors_ << filename;
    }
  }
  SetSongProgress(0);

  QTimer::singleShot(0, this, SLOT(ProcessSomeFiles()));
}

void Organise::SetSongProgress(float progress) {
  song_progress_ = qBound(0, int(progress * 100), 99);
  UpdateProgress();
}

void Organise::UpdateProgress() {
  const int progress = progress_ * 100 + song_progress_;
  const int total = files_.count() * 100;
  task_manager_->SetTaskProgress(task_id_, progress, total);
}
