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

#include <boost/scoped_ptr.hpp>

AfcTransfer::AfcTransfer(const QString& uuid, const QString& local_destination,
                         TaskManager* task_manager, QObject* parent)
  : QObject(parent),
    task_manager_(task_manager),
    uuid_(uuid),
    local_destination_(local_destination)
{
  original_thread_ = thread();

  important_directories_ << "/iTunes_Control/Artwork";
  important_directories_ << "/iTunes_Control/Device";
  important_directories_ << "/iTunes_Control/iTunes";
}

void AfcTransfer::CopyFromDevice() {
  int task_id = 0;
  if (task_manager_) {
    task_id = task_manager_->StartTask(tr("Copying iPod database"));
    emit TaskStarted(task_id);
  }

  // Connect to the device
  iMobileDeviceConnection c(uuid_);

  // Copy directories.  If one fails we stop.
  bool success = true;
  foreach (const QString& dir, important_directories_) {
    if (!CopyDirFromDevice(&c, dir)) {
      success = false;
      break;
    }
  }

  if (task_manager_) {
    moveToThread(original_thread_);
    task_manager_->SetTaskFinished(task_id);
    emit CopyFinished(success);
  }
}

bool AfcTransfer::CopyToDevice() {
  // Connect to the device
  iMobileDeviceConnection c(uuid_);

  foreach (const QString& dir, important_directories_)
    if (!CopyDirToDevice(&c, dir))
      return false;

  return true;
}

bool AfcTransfer::CopyDirFromDevice(iMobileDeviceConnection* c, const QString& path) {
  foreach (const QString& filename, c->ReadDirectory(path, QDir::Files | QDir::NoDotAndDotDot)) {
    if (!CopyFileFromDevice(c, path + "/" + filename))
      return false;
  }

  foreach (const QString& dir, c->ReadDirectory(path, QDir::Dirs | QDir::NoDotAndDotDot)) {
    if (!CopyDirFromDevice(c, path + "/" + dir))
      return false;
  }
  return true;
}

bool AfcTransfer::CopyDirToDevice(iMobileDeviceConnection* c, const QString& path) {
  QDir dir(local_destination_ + path);

  foreach (const QString& filename, dir.entryList(QDir::Files | QDir::NoDotAndDotDot)) {
    if (!CopyFileToDevice(c, path + "/" + filename))
      return false;
  }

  foreach (const QString& dir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    if (!CopyDirToDevice(c, path + "/" + dir))
      return false;
  }
  return true;
}

bool AfcTransfer::CopyFileFromDevice(iMobileDeviceConnection *c, const QString &path) {
  QString local_filename = local_destination_ + path;
  QString local_dir = local_filename.section('/', 0, -2);

  QDir d;
  d.mkpath(local_dir);

  QFile dest(local_filename);
  AfcFile source(c, path);

  return Copy(&source, &dest);
}

bool AfcTransfer::CopyFileToDevice(iMobileDeviceConnection *c, const QString &path) {
  QFile source(local_destination_ + path);
  AfcFile dest(c, path);

  return Copy(&source, &dest);
}

bool AfcTransfer::Copy(QIODevice* source, QIODevice* destination) {
  if (!source->open(QIODevice::ReadOnly))
    return false;

  if (!destination->open(QIODevice::WriteOnly))
    return false;

  const qint64 bytes = source->size();
  char* data = new char[bytes];
  qint64 pos = 0;

  forever {
    const qint64 bytes_read = source->read(data + pos, bytes - pos);
    if (bytes_read == -1) {
      delete[] data;
      return false;
    }

    pos += bytes_read;
    if (bytes_read == 0 || pos == bytes)
      break;
  }

  pos = 0;
  forever {
    const qint64 bytes_written = destination->write(data + pos, bytes - pos);
    if (bytes_written == -1) {
      delete[] data;
      return false;
    }

    pos += bytes_written;
    if (bytes_written == 0 || pos == bytes)
      break;
  }

  delete[] data;
  return true;
}
