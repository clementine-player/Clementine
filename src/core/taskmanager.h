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

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QMap>
#include <QMutex>
#include <QObject>

class TaskManager : public QObject {
  Q_OBJECT

public:
  TaskManager(QObject* parent = 0);

  struct Task {
    int id;
    QString name;
    int progress;
    int progress_max;
  };

  // Thread-safe
  int StartTask(const QString& name);
  QList<Task> GetTasks();

public slots:
  // Thread-safe
  void SetTaskProgress(int id, int progress, int max = -1);
  void SetTaskFinished(int id);

signals:
  void TasksChanged();

private:
  QMutex mutex_;
  QMap<int, Task> tasks_;
  int next_task_id_;
};

#endif // TASKMANAGER_H
