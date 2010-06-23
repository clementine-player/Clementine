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

#include "taskmanager.h"

#include <QtDebug>

TaskManager::TaskManager(QObject *parent)
  : QObject(parent),
    next_task_id_(1)
{
}

int TaskManager::StartTask(const QString& name) {
  Task t;
  t.name = name;
  t.progress = 0;
  t.progress_max = 0;

  {
    QMutexLocker l(&mutex_);
    t.id = next_task_id_ ++;
    tasks_[t.id] = t;
  }

  qDebug() << name << "started";
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

void TaskManager::SetTaskProgress(int id, int progress, int max) {
  {
    QMutexLocker l(&mutex_);
    if (!tasks_.contains(id))
      return;

    Task& t = tasks_[id];
    t.progress = progress;
    if (max != -1)
      t.progress_max = max;

    qDebug() << t.name << "progress" << t.progress << "/" << t.progress_max;
  }

  emit TasksChanged();
}

void TaskManager::SetTaskFinished(int id) {
  {
    QMutexLocker l(&mutex_);
    if (!tasks_.contains(id))
      return;
    qDebug() << tasks_[id].name << "finished";
    tasks_.remove(id);
  }
  emit TasksChanged();
}
