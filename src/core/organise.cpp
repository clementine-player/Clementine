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

const int Organise::kBatchSize = 10;

Organise::Organise(TaskManager* task_manager, MusicStorage* destination,
                   const OrganiseFormat &format, bool copy, bool overwrite,
                   const QStringList& files)
                     : thread_(NULL),
                       task_manager_(task_manager),
                       destination_(destination),
                       format_(format),
                       copy_(copy),
                       overwrite_(overwrite),
                       files_(files),
                       task_id_(0),
                       progress_(0)
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
  // None left?
  if (progress_ >= files_.count()) {
    task_manager_->SetTaskFinished(task_id_);

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
    task_manager_->SetTaskProgress(task_id_, progress_, files_.count());

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

    destination_->CopyToStorage(filename, format_.GetFilenameForSong(song),
                                song, overwrite_, !copy_);
  }

  QTimer::singleShot(0, this, SLOT(ProcessSomeFiles()));
}
