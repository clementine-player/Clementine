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

#ifndef GPODLOADER_H
#define GPODLOADER_H

#include <QObject>

#include <boost/shared_ptr.hpp>

#include <gpod/itdb.h>

class LibraryBackend;
class TaskManager;

class GPodLoader : public QObject {
  Q_OBJECT

public:
  GPodLoader(const QString& mount_point, TaskManager* task_manager,
             LibraryBackend* backend, QObject* parent = 0);

  void set_music_path_prefix(const QString& prefix) { path_prefix_ = prefix; }

public slots:
  void LoadDatabase();

signals:
  void Error(const QString& message);
  void TaskStarted(int task_id);
  void LoadFinished(Itdb_iTunesDB* db);

private:
  QThread* original_thread_;

  QString mount_point_;
  QString path_prefix_;
  TaskManager* task_manager_;
  LibraryBackend* backend_;
};

#endif // GPODLOADER_H
