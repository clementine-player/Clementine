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

#include "connecteddevice.h"
#include "mtploader.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "library/librarybackend.h"

#include <libmtp.h>

MtpLoader::MtpLoader(const QString& hostname, TaskManager* task_manager,
                     LibraryBackend* backend, boost::shared_ptr<ConnectedDevice> device)
  : QObject(NULL),
    device_(device),
    hostname_(hostname),
    task_manager_(task_manager),
    backend_(backend)
{
  original_thread_ = thread();
}

MtpLoader::~MtpLoader() {
}

void MtpLoader::LoadDatabase() {
  int task_id = task_manager_->StartTask(tr("Loading MTP device"));
  emit TaskStarted(task_id);

  TryLoad();

  moveToThread(original_thread_);

  task_manager_->SetTaskFinished(task_id);
  emit LoadFinished();
}

bool MtpLoader::TryLoad() {
  // Parse the URL
  QRegExp host_re("^usb-(\\d+)-(\\d+)$");

  if (host_re.indexIn(hostname_) == -1) {
    emit Error(tr("Invalid MTP device: %1").arg(hostname_));
    return false;
  }

  const unsigned int bus_location = host_re.cap(1).toInt();
  const unsigned int device_num = host_re.cap(2).toInt();

  // Get a list of devices from libmtp and figure out which one is ours
  int count = 0;
  LIBMTP_raw_device_t* raw_devices = NULL;
  LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&raw_devices, &count);
  if (err != LIBMTP_ERROR_NONE) {
    emit Error(tr("Error connecting MTP device"));
    qWarning() << "MTP error:" << err;
    return false;
  }

  LIBMTP_raw_device_t* raw_device = NULL;
  for (int i=0 ; i<count ; ++i) {
    if (raw_devices[i].bus_location == bus_location &&
        raw_devices[i].devnum == device_num) {
      raw_device = &raw_devices[i];
      break;
    }
  }

  if (!raw_device) {
    emit Error(tr("MTP device not found"));
    free(raw_devices);
    return false;
  }

  // Connect to the device
  LIBMTP_mtpdevice_t* device = LIBMTP_Open_Raw_Device(raw_device);

  // Load the list of songs on the device
  SongList songs;
  LIBMTP_track_t* tracks = LIBMTP_Get_Tracklisting_With_Callback(device, NULL, NULL);
  while (tracks) {
    LIBMTP_track_t* track = tracks;

    Song song;
    song.InitFromMTP(track);
    song.set_directory_id(1);
    songs << song;

    tracks = tracks->next;
    LIBMTP_destroy_track_t(track);
  }

  // Need to remove all the existing songs in the database first
  backend_->DeleteSongs(backend_->FindSongsInDirectory(1));

  // Add the songs we've just loaded
  backend_->AddOrUpdateSongs(songs);

  free(raw_devices);
  return true;
}
