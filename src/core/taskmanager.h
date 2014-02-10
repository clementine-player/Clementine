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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QMap>
#include <QMutex>
#include <QObject>

class TaskManager : public QObject {
  Q_OBJECT

 public:
  TaskManager(QObject* parent = nullptr);

  struct Task {
    int id;
    QString name;
    int progress;
    int progress_max;
    bool blocks_library_scans;
  };

  class ScopedTask {
   public:
    ScopedTask(const int task_id, TaskManager* task_manager)
        : task_id_(task_id), task_manager_(task_manager) {}

    ~ScopedTask() { task_manager_->SetTaskFinished(task_id_); }

   private:
    const int task_id_;
    TaskManager* task_manager_;

    Q_DISABLE_COPY(ScopedTask);
  };

  // Everything here is thread safe
  QList<Task> GetTasks();

  int StartTask(const QString& name);
  void SetTaskBlocksLibraryScans(int id);
  void SetTaskProgress(int id, int progress, int max = 0);
  void IncreaseTaskProgress(int id, int progress, int max = 0);
  void SetTaskFinished(int id);
  int GetTaskProgress(int id);

signals:
  void TasksChanged();

  void PauseLibraryWatchers();
  void ResumeLibraryWatchers();

 private:
  QMutex mutex_;
  QMap<int, Task> tasks_;
  int next_task_id_;

  Q_DISABLE_COPY(TaskManager);
};

#endif  // TASKMANAGER_H
