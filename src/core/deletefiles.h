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

#ifndef DELETEFILES_H
#define DELETEFILES_H

#include <QObject>

#include "song.h"

class MusicStorage;
class TaskManager;

class DeleteFiles : public QObject {
  Q_OBJECT

public:
  DeleteFiles(TaskManager* task_manager, MusicStorage* storage);

  static const int kBatchSize;

  void Start(const SongList& songs);
  void Start(const QStringList& filenames);

private slots:
  void ProcessSomeFiles();

private:
  QThread* thread_;
  QThread* original_thread_;
  TaskManager* task_manager_;
  MusicStorage* storage_;

  SongList songs_;

  bool started_;

  int task_id_;
  int progress_;
};

#endif // DELETEFILES_H
