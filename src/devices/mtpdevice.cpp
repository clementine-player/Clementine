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

#include "devicemanager.h"
#include "mtpconnection.h"
#include "mtpdevice.h"
#include "mtploader.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <libmtp.h>

#include <QFile>

bool MtpDevice::sInitialisedLibMTP = false;

MtpDevice::MtpDevice(const QUrl& url, DeviceLister* lister,
                     const QString& unique_id, DeviceManager* manager,
                     int database_id, bool first_time)
  : ConnectedDevice(url, lister, unique_id, manager, database_id, first_time),
    loader_thread_(new QThread(this)),
    loader_(NULL)
{
  if (!sInitialisedLibMTP) {
    LIBMTP_Init();
    sInitialisedLibMTP = true;
  }
}

MtpDevice::~MtpDevice() {
}

void MtpDevice::Init() {
  InitBackendDirectory("/", first_time_, false);
  model_->Init();

  loader_ = new MtpLoader(url_.host(), manager_->task_manager(), backend_,
                          shared_from_this());
  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished()), SLOT(LoadFinished()));
  connect(loader_thread_, SIGNAL(started()), loader_, SLOT(LoadDatabase()));
  loader_thread_->start();
}

void MtpDevice::LoadFinished() {
  loader_->deleteLater();
  loader_ = NULL;
}

void MtpDevice::StartCopy() {
  // Ensure only one "organise files" can be active at any one time
  db_busy_.lock();

  // Connect to the device
  connection_.reset(new MtpConnection(url_.host()));
}

bool MtpDevice::CopyToStorage(
    const QString& source, const QString&, const Song& metadata,
    bool, bool remove_original)
{
  // Convert metadata
  LIBMTP_track_t track;
  metadata.ToMTP(&track);

  // Send the file
  int ret = LIBMTP_Send_Track_From_File(
      connection_->device(), source.toUtf8().constData(), &track, NULL, NULL);
  if (ret != 0)
    return false;

  // Add it to our LibraryModel
  Song metadata_on_device;
  metadata_on_device.InitFromMTP(&track);
  metadata_on_device.set_directory_id(1);
  songs_to_add_ << metadata_on_device;

  // Remove the original if requested
  if (remove_original) {
    if (!QFile::remove(source))
      return false;
  }

  return true;
}

void MtpDevice::FinishCopy(bool success) {
  if (success) {
    if (!songs_to_add_.isEmpty())
      backend_->AddOrUpdateSongs(songs_to_add_);
  }

  songs_to_add_.clear();
  songs_to_remove_.clear();

  connection_.reset();

  db_busy_.unlock();
}

void MtpDevice::StartDelete() {

}

bool MtpDevice::DeleteFromStorage(const Song& metadata) {
  return false;
}

void MtpDevice::FinishDelete(bool success) {

}
