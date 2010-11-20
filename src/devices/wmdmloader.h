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

#ifndef WMDMLOADER_H
#define WMDMLOADER_H

#include <QObject>

#include <boost/shared_ptr.hpp>

#include "core/song.h"

class ConnectedDevice;
class LibraryBackend;
class TaskManager;

struct IWMDMStorage;

class WmdmLoader : public QObject {
  Q_OBJECT

public:
  WmdmLoader(TaskManager* task_manager, LibraryBackend* backend,
             boost::shared_ptr<ConnectedDevice> device);
  ~WmdmLoader();

public slots:
  void LoadDatabase();

signals:
  void Error(const QString& message);
  void TaskStarted(int task_id);
  void LoadFinished();

private:
  void RecursiveExploreStorage(IWMDMStorage* parent, QStringList* path_components);
  void LoadFile(IWMDMStorage* file, const QStringList* path_components);

private:
  boost::shared_ptr<ConnectedDevice> device_;
  QThread* original_thread_;

  TaskManager* task_manager_;
  LibraryBackend* backend_;

  SongList songs_;
};

#endif // WMDMLOADER_H
