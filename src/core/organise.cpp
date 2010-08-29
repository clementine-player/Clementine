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
                       transcoder_(new Transcoder(this)),
                       destination_(destination),
                       format_(format),
                       copy_(copy),
                       overwrite_(overwrite),
                       eject_after_(eject_after),
                       task_count_(files.count()),
                       transcode_suffix_(1),
                       started_(false),
                       task_id_(0),
                       progress_(0),
                       song_progress_(0)
{
  original_thread_ = thread();

  foreach (const QString& filename, files) {
    tasks_pending_ << Task(filename);
  }
}

void Organise::Start() {
  if (thread_)
    return;

  task_id_ = task_manager_->StartTask(tr("Organising files"));
  task_manager_->SetTaskBlocksLibraryScans(true);

  thread_ = new QThread;
  connect(thread_, SIGNAL(started()), SLOT(ProcessSomeFiles()));
  connect(transcoder_, SIGNAL(JobComplete(QString,bool)), SLOT(FileTranscoded(QString,bool)));

  moveToThread(thread_);
  thread_->start();
}

void Organise::ProcessSomeFiles() {
  if (!started_) {
    transcode_temp_name_.open();
    supported_filetypes_ = destination_->SupportedFiletypes();

    destination_->StartCopy();
    started_ = true;
  }

  // None left?
  if (tasks_pending_.isEmpty()) {
    if (!tasks_transcoding_.isEmpty()) {
      // Just wait - FileTranscoded will start us off again in a little while
      qDebug() << "Waiting for transcoding jobs";
      return;
    }

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

  // We process files in batches so we can be cancelled part-way through.
  for (int i=0 ; i<kBatchSize ; ++i) {
    SetSongProgress(0);

    if (tasks_pending_.isEmpty())
      break;

    Task task = tasks_pending_.takeFirst();
    qDebug() << "Processing" << task.filename_;

    // Is it a directory?
    if (QFileInfo(task.filename_).isDir()) {
      QDir dir(task.filename_);
      foreach (const QString& entry, dir.entryList(
          QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable)) {
        tasks_pending_ << Task(task.filename_ + "/" + entry);
        task_count_ ++;
      }
      continue;
    }

    // Read metadata from the file
    Song song;
    song.InitFromFile(task.filename_, -1);
    if (!song.is_valid())
      continue;

    // Maybe this file is one that's been transcoded already?
    if (!task.transcoded_filename_.isEmpty()) {
      qDebug() << "This file has already been transcoded";

      // Set the new filetype on the song so the formatter gets it right
      song.set_filetype(task.new_filetype_);

      // Fiddle the filename extension as well to match the new type
      song.set_filename(song.filename().section('.', 0, -2) + "." + task.new_extension_);
      song.set_basefilename(song.basefilename().section('.', 0, -2) + "." + task.new_extension_);

      // Have to set this to the size of the new file or else funny stuff happens
      song.set_filesize(QFileInfo(task.transcoded_filename_).size());
    } else {
      // Figure out if we need to transcode it
      Song::FileType dest_type = CheckTranscode(song.filetype());
      if (dest_type != Song::Type_Unknown) {
        // Get the preset
        TranscoderPreset preset = Transcoder::PresetForFileType(dest_type);
        qDebug() << "Transcoding with" << preset.name_;

        // Get a temporary name for the transcoded file
        task.transcoded_filename_ = transcode_temp_name_.fileName() + "-" +
                                    QString::number(transcode_suffix_++);
        task.new_extension_ = preset.extension_;
        task.new_filetype_ = dest_type;
        tasks_transcoding_[task.filename_] = task;

        qDebug() << "Transcoding to" << task.transcoded_filename_;

        // Start the transcoding - this will happen in the background and
        // FileTranscoded() will get called when it's done.  At that point the
        // task will get re-added to the pending queue with the new filename.
        transcoder_->AddJob(task.filename_, preset, task.transcoded_filename_);
        transcoder_->Start();
        continue;
      }
    }

    MusicStorage::CopyJob job;
    job.source_ = task.transcoded_filename_.isEmpty() ?
                  task.filename_ : task.transcoded_filename_;
    job.destination_ = format_.GetFilenameForSong(song);
    job.metadata_ = song;
    job.overwrite_ = overwrite_;
    job.remove_original_ = !copy_;
    job.progress_ = boost::bind(&Organise::SetSongProgress, this, _1);

    if (!destination_->CopyToStorage(job)) {
      files_with_errors_ << task.filename_;
    }

    progress_++;
  }
  SetSongProgress(0);

  QTimer::singleShot(0, this, SLOT(ProcessSomeFiles()));
}

Song::FileType Organise::CheckTranscode(Song::FileType original_type) const {
  if (original_type == Song::Type_Stream)
    return Song::Type_Unknown;

  const MusicStorage::TranscodeMode mode = destination_->GetTranscodeMode();
  const Song::FileType format = destination_->GetTranscodeFormat();

  switch (mode) {
    case MusicStorage::Transcode_Never:
      return Song::Type_Unknown;

    case MusicStorage::Transcode_Always:
      if (original_type == format)
        return Song::Type_Unknown;
      return format;

    case MusicStorage::Transcode_Unsupported:
      if (supported_filetypes_.isEmpty() || supported_filetypes_.contains(original_type))
        return Song::Type_Unknown;

      if (format != Song::Type_Unknown)
        return format;

      // The user hasn't visited the device properties page yet to set a
      // preferred format for the device, so we have to pick the best
      // available one.
      return Transcoder::PickBestFormat(supported_filetypes_);
  }
  return Song::Type_Unknown;
}

void Organise::SetSongProgress(float progress) {
  song_progress_ = qBound(0, int(progress * 100), 99);
  UpdateProgress();
}

void Organise::UpdateProgress() {
  const int progress = progress_ * 100 + song_progress_;
  const int total = task_count_ * 100;
  task_manager_->SetTaskProgress(task_id_, progress, total);
}

void Organise::FileTranscoded(const QString& filename, bool success) {
  qDebug() << "File finished" << filename << success;

  Task task = tasks_transcoding_.take(filename);
  if (!success) {
    files_with_errors_ << filename;
  } else {
    tasks_pending_ << task;
  }
  QTimer::singleShot(0, this, SLOT(ProcessSomeFiles()));
}
