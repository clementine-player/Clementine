/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <me@davidsansome.com>
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
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

#include "taskmanager.h"

TaskManager::TaskManager(QObject* parent) : QObject(parent), next_task_id_(1) {}

int TaskManager::StartTask(const QString& name) {
  Task t;
  t.name = name;
  t.progress = 0;
  t.progress_max = 0;
  t.blocks_library_scans = false;

  {
    QMutexLocker l(&mutex_);
    t.id = next_task_id_++;
    tasks_[t.id] = t;
  }

  emit TasksChanged();
  return t.id;
}

QList<TaskManager::Task> TaskManager::GetTasks() {
  QList<TaskManager::Task> ret;

  {
    QMutexLocker l(&mutex_);
    ret = tasks_.values();
  }

  return ret;
}

void TaskManager::SetTaskBlocksLibraryScans(int id) {
  {
    QMutexLocker l(&mutex_);
    if (!tasks_.contains(id)) return;

    Task& t = tasks_[id];
    t.blocks_library_scans = true;
  }

  emit TasksChanged();
  emit PauseLibraryWatchers();
}

void TaskManager::SetTaskProgress(int id, int progress, int max) {
  {
    QMutexLocker l(&mutex_);
    if (!tasks_.contains(id)) return;

    Task& t = tasks_[id];
    t.progress = progress;
    if (max) t.progress_max = max;
  }

  emit TasksChanged();
}

void TaskManager::IncreaseTaskProgress(int id, int progress, int max) {
  {
    QMutexLocker l(&mutex_);
    if (!tasks_.contains(id)) return;

    Task& t = tasks_[id];
    t.progress += progress;
    if (max) t.progress_max = max;
  }

  emit TasksChanged();
}

void TaskManager::SetTaskFinished(int id) {
  bool resume_library_watchers = false;

  {
    QMutexLocker l(&mutex_);
    if (!tasks_.contains(id)) return;

    if (tasks_[id].blocks_library_scans) {
      resume_library_watchers = true;
      for (const Task& task : tasks_.values()) {
        if (task.id != id && task.blocks_library_scans) {
          resume_library_watchers = false;
          break;
        }
      }
    }

    tasks_.remove(id);
  }

  emit TasksChanged();
  if (resume_library_watchers) emit ResumeLibraryWatchers();
}

int TaskManager::GetTaskProgress(int id) {
  {
    QMutexLocker l(&mutex_);
    if (!tasks_.contains(id)) return 0;
    return tasks_[id].progress;
  }
}
