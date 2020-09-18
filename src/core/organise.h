/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
   Copyright 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Andreas <asfa194@gmail.com>
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

#ifndef CORE_ORGANISE_H_
#define CORE_ORGANISE_H_

#include <QBasicTimer>
#include <QFileInfo>
#include <QObject>
#include <QTemporaryFile>
#include <memory>

#include "organiseformat.h"
#include "transcoder/transcoder.h"

class MusicStorage;
class TaskManager;

class Organise : public QObject {
  Q_OBJECT

 public:
  struct NewSongInfo {
    NewSongInfo(const Song& song = Song(),
                const QString& new_filename = QString())
        : song_(song), new_filename_(new_filename) {}
    Song song_;
    QString new_filename_;
  };
  typedef QList<NewSongInfo> NewSongInfoList;

  Organise(TaskManager* task_manager, std::shared_ptr<MusicStorage> destination,
           const OrganiseFormat& format, bool copy, bool overwrite,
           bool mark_as_listened, const NewSongInfoList& songs,
           bool eject_after);

  static const int kBatchSize;
  static const int kTranscodeProgressInterval;

  void Start();

 signals:
  void Finished(const QStringList& files_with_errors);
  void FileCopied(int database_id);
  void SongPathChanged(const Song& song, const QFileInfo& new_file);

 protected:
  void timerEvent(QTimerEvent* e);

 private slots:
  void ProcessSomeFiles();
  void FileTranscoded(const QString& input, const QString& output,
                      bool success);

 private:
  void SetSongProgress(float progress, bool transcoded = false);
  void UpdateProgress();
  Song::FileType CheckTranscode(Song::FileType original_type) const;

 private:
  struct Task {
    explicit Task(const NewSongInfo& song_info = NewSongInfo())
        : song_info_(song_info), transcode_progress_(0.0) {}

    NewSongInfo song_info_;

    float transcode_progress_;
    QString transcoded_filename_;
    QString new_extension_;
    Song::FileType new_filetype_;
  };

  QThread* thread_;
  QThread* original_thread_;
  TaskManager* task_manager_;
  Transcoder* transcoder_;
  std::shared_ptr<MusicStorage> destination_;
  QList<Song::FileType> supported_filetypes_;

  const OrganiseFormat format_;
  const bool copy_;
  const bool overwrite_;
  const bool mark_as_listened_;
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

#endif  // CORE_ORGANISE_H_
