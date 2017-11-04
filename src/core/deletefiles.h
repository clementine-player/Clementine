/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
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

#ifndef CORE_DELETEFILES_H_
#define CORE_DELETEFILES_H_

#include <memory>

#include <QObject>

#include "song.h"

class MusicStorage;
class TaskManager;

class DeleteFiles : public QObject {
  Q_OBJECT

 public:
  DeleteFiles(TaskManager* task_manager, std::shared_ptr<MusicStorage> storage);
  ~DeleteFiles();

  static const int kBatchSize;

  void Start(const SongList& songs);
  void Start(const QStringList& filenames);
  void Start(const QUrl& url);

 signals:
  void Finished(const SongList& songs_with_errors);

 private slots:
  void ProcessSomeFiles();

 private:
  QThread* thread_;
  QThread* original_thread_;
  TaskManager* task_manager_;
  std::shared_ptr<MusicStorage> storage_;

  SongList songs_;

  bool started_;

  int task_id_;
  int progress_;

  SongList songs_with_errors_;
};

#endif  // CORE_DELETEFILES_H_
