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
#include "wmdmdevice.h"
#include "wmdmlister.h"
#include "wmdmloader.h"
#include "wmdmprogress.h"
#include "wmdmthread.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <QDir>
#include <QThread>

#include <boost/scoped_array.hpp>

#include <mswmdm.h>

WmdmDevice::WmdmDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, database_id, first_time),
      loader_thread_(new QThread(this)),
      loader_(NULL)
{
}

WmdmDevice::~WmdmDevice() {
}

void WmdmDevice::Init() {
  InitBackendDirectory("/", first_time_, false);
  model_->Init();

  loader_ = new WmdmLoader(manager_->task_manager(), backend_, shared_from_this());
  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished()), SLOT(LoadFinished()));
  connect(loader_thread_, SIGNAL(started()), loader_, SLOT(LoadDatabase()));
  loader_thread_->start();
}

void WmdmDevice::LoadFinished() {
  loader_->deleteLater();
  loader_ = NULL;
}

void WmdmDevice::StartCopy() {
  // Ensure only one "organise files" can be active at any one time
  db_busy_.lock();

  // This initialises COM and gets a connection to the device
  thread_.reset(new WmdmThread);

  // Find a place to put the files.  We default to the root folder for now, but
  // could look for a "Music" folder in the future?
  WmdmLister* wmdm_lister = static_cast<WmdmLister*>(lister());
  QString canonical_name = wmdm_lister->DeviceCanonicalName(unique_id());
  IWMDMStorage* destination = thread_->GetRootStorage(canonical_name);

  // Get the control interface
  destination->QueryInterface(IID_IWMDMStorageControl3, (void**)&storage_control_);

  // Get the storage3 interface for CreateEmptyMetadataObject later
  destination->QueryInterface(IID_IWMDMStorage3, (void**)&storage_);

  destination->Release();
}

bool WmdmDevice::CopyToStorage(
    const QString& source, const QString&, const Song& song,
    bool, bool remove_original)
{
  if (!storage_control_ || !storage_)
    return false;

  // Create the song metadata
  IWMDMMetaData* metadata_iface = NULL;
  storage_->CreateEmptyMetadataObject(&metadata_iface);
  song.ToWmdm(metadata_iface);

  // Convert the filenames to wchars
  ScopedWCharArray source_filename(QDir::toNativeSeparators(source));
  ScopedWCharArray dest_filename(song.basefilename());

  // Create the progress object
  WmdmProgress progress;

  // Copy the file
  IWMDMStorage* new_storage = NULL;
  if (storage_control_->Insert3(
      WMDM_MODE_BLOCK | WMDM_STORAGECONTROL_INSERTINTO |
      WMDM_FILE_CREATE_OVERWRITE | WMDM_CONTENT_FILE,
      WMDM_FILE_ATTR_FOLDER,
      source_filename,
      dest_filename,
      NULL, // operation
      &progress, // progress
      metadata_iface,
      NULL, // data
      &new_storage)) {
    qWarning() << "Couldn't copy file to WMDM device";
    metadata_iface->Release();
    return false;
  }
  metadata_iface->Release();

  if (!new_storage)
    return false;

  // Get the metadata from the newly copied file
  IWMDMStorage3* new_storage3 = NULL;
  IWMDMMetaData* new_metadata = NULL;

  new_storage->QueryInterface(IID_IWMDMStorage3, (void**)&new_storage3);
  new_storage3->GetMetadata(&new_metadata);

  new_storage->Release();
  new_storage3->Release();

  if (!new_metadata)
    return false;

  // Add it to our LibraryModel
  Song new_song;
  new_song.InitFromWmdm(new_metadata);
  new_song.set_directory_id(1);
  songs_to_add_ << new_song;

  new_metadata->Release();

  // Remove the original if requested
  if (remove_original) {
    if (!QFile::remove(source))
      return false;
  }

  return true;
}

void WmdmDevice::FinishCopy(bool success) {
  if (success) {
    if (!songs_to_add_.isEmpty())
      backend_->AddOrUpdateSongs(songs_to_add_);
    if (!songs_to_remove_.isEmpty())
      backend_->DeleteSongs(songs_to_remove_);
  }

  songs_to_add_.clear();
  songs_to_remove_.clear();

  storage_->Release();
  storage_control_->Release();
  thread_.reset();

  db_busy_.unlock();
}

void WmdmDevice::StartDelete() {
  StartCopy();
}

bool WmdmDevice::DeleteFromStorage(const Song& metadata) {
  return false;
}

void WmdmDevice::FinishDelete(bool success) {
  FinishCopy(success);
}
