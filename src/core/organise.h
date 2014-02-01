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

#ifndef ORGANISE_H
#define ORGANISE_H

#include <QBasicTimer>
#include <QObject>
#include <QTemporaryFile>

#include <boost/shared_ptr.hpp>

#include "organiseformat.h"
#include "transcoder/transcoder.h"

class MusicStorage;
class TaskManager;

class Organise : public QObject {
  Q_OBJECT

public:
  Organise(TaskManager* task_manager,
           boost::shared_ptr<MusicStorage> destination,
           const OrganiseFormat& format, bool copy, bool overwrite,
           const SongList& songs, bool eject_after);

  static const int kBatchSize;
  static const int kTranscodeProgressInterval;

  void Start();

signals:
  void Finished(const QStringList& files_with_errors);

protected:
  void timerEvent(QTimerEvent* e);

private slots:
  void ProcessSomeFiles();
  void FileTranscoded(const QString& filename, bool success);

private:
  void SetSongProgress(float progress, bool transcoded = false);
  void UpdateProgress();
  Song::FileType CheckTranscode(Song::FileType original_type) const;

private:
  struct Task {
    explicit Task(const Song& song = Song())
      : song_(song), transcode_progress_(0.0) {}

    Song song_;

    float transcode_progress_;
    QString transcoded_filename_;
    QString new_extension_;
    Song::FileType new_filetype_;
  };

  QThread* thread_;
  QThread* original_thread_;
  TaskManager* task_manager_;
  Transcoder* transcoder_;
  boost::shared_ptr<MusicStorage> destination_;
  QList<Song::FileType> supported_filetypes_;

  const OrganiseFormat format_;
  const bool copy_;
  const bool overwrite_;
  const bool eject_after_;
  int task_count_;

  QBasicTimer transcode_progress_timer_;
  QTemporaryFile transcode_temp_name_;
  int transcode_suffix_;

  QList<Task> tasks_pending_;
  QMap<QString, Task> tasks_transcoding_;
  int tasks_complete_;

  bool started_;

  int task_id_;
  int current_copy_progress_;

  QStringList files_with_errors_;
};

#endif // ORGANISE_H
