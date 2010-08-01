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

#include "afcfile.h"
#include "afctransfer.h"
#include "imobiledeviceconnection.h"
#include "core/taskmanager.h"

#include <QDir>
#include <QtDebug>

AfcTransfer::AfcTransfer(const QString& uuid, const QString& local_destination,
                         TaskManager* task_manager, QObject* parent)
  : QObject(parent),
    task_manager_(task_manager),
    uuid_(uuid),
    local_destination_(local_destination)
{
  original_thread_ = thread();
}

void AfcTransfer::CopyFromDevice() {
  int task_id = task_manager_->StartTask(tr("Copying iPod database"));
  emit TaskStarted(task_id);

  // Connect to the device
  iMobileDeviceConnection c(uuid_);

  CopyDirFromDevice(&c, "/iTunes_Control/Device");
  CopyDirFromDevice(&c, "/iTunes_Control/iTunes");

  moveToThread(original_thread_);
  task_manager_->SetTaskFinished(task_id);
  emit CopyFinished();
}

void AfcTransfer::CopyDirFromDevice(iMobileDeviceConnection* c, const QString& path) {
  foreach (const QString& filename, c->ReadDirectory(path, QDir::Files | QDir::NoDotAndDotDot)) {
    CopyFileFromDevice(c, path + "/" + filename);
  }

  foreach (const QString& dir, c->ReadDirectory(path, QDir::Dirs | QDir::NoDotAndDotDot)) {
    CopyDirFromDevice(c, path + "/" + dir);
  }
}

void AfcTransfer::CopyFileFromDevice(iMobileDeviceConnection *c, const QString &path) {
  QString local_filename = local_destination_ + path;
  QString local_dir = local_filename.section('/', 0, -2);

  QDir d;
  d.mkpath(local_dir);

  QFile dest(local_filename);
  AfcFile source(c, path);

  dest.open(QIODevice::WriteOnly);
  source.open(QIODevice::ReadOnly);

  dest.write(source.readAll());
}

void AfcTransfer::CopyToDevice() {

}
