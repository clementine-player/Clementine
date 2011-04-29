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

#include "devicelister.h"
#include "wmdmdevice.h"
#include "wmdmlister.h"
#include "wmdmloader.h"
#include "wmdmthread.h"
#include "core/taskmanager.h"
#include "library/librarybackend.h"

#include <boost/scoped_array.hpp>

#include <cwchar>
#include <mswmdm.h>

#include <QUuid>

WmdmLoader::WmdmLoader(TaskManager* task_manager, LibraryBackend* backend,
                       boost::shared_ptr<ConnectedDevice> device)
  : QObject(NULL),
    device_(device),
    task_manager_(task_manager),
    backend_(backend)
{
  original_thread_ = thread();
}

WmdmLoader::~WmdmLoader() {
}

void WmdmLoader::LoadDatabase() {
  int task_id = task_manager_->StartTask(tr("Loading Windows Media device"));
  emit TaskStarted(task_id);

  boost::scoped_ptr<WmdmThread> thread(new WmdmThread);

  // Get the device's canonical name
  boost::shared_ptr<WmdmDevice> connected_device =
      boost::static_pointer_cast<WmdmDevice>(device_);
  WmdmLister* lister = static_cast<WmdmLister*>(connected_device->lister());
  QString canonical_name = lister->DeviceCanonicalName(connected_device->unique_id());

  IWMDMStorage* storage = thread->GetRootStorage(canonical_name);
  QStringList path_components;
  RecursiveExploreStorage(storage, &path_components);
  storage->Release();

  thread.reset();

  // Need to remove all the existing songs in the database first
  backend_->DeleteSongs(backend_->FindSongsInDirectory(1));

  // Add the songs we've just loaded
  backend_->AddOrUpdateSongs(songs_);

  task_manager_->SetTaskFinished(task_id);
  emit LoadFinished();
}

void WmdmLoader::RecursiveExploreStorage(IWMDMStorage* parent, QStringList* path_components) {
  IWMDMEnumStorage* child_it = NULL;
  parent->EnumStorage(&child_it);

  IWMDMStorage* child = NULL;
  ULONG num_retreived = 0;
  while (child_it->Next(1, &child, &num_retreived) == S_OK && num_retreived == 1) {
    const int kMaxLen = 255;
    wchar_t name[kMaxLen];
    child->GetName(name, kMaxLen);

    DWORD attributes = 0;
    _WAVEFORMATEX audio_format;
    child->GetAttributes(&attributes, &audio_format);

    path_components->append(QString::fromWCharArray(name));
    if (attributes & WMDM_FILE_ATTR_FILE) {
      LoadFile(child, path_components);
    } else if (attributes & WMDM_FILE_ATTR_FOLDER) {
      RecursiveExploreStorage(child, path_components);
    }
    path_components->removeLast();

    child->Release();
  }
  child_it->Release();
}

void WmdmLoader::LoadFile(IWMDMStorage* file, const QStringList* path_components) {
  // Convert to a IWMDMStorage3 so we can get metadata
  IWMDMStorage3* storage3 = NULL;
  if (file->QueryInterface(IID_IWMDMStorage3, (void**) &storage3))
    return;

  // Get the metadata interface
  IWMDMMetaData* metadata = NULL;
  if (storage3->GetMetadata(&metadata)) {
    storage3->Release();
    return;
  }
  storage3->Release();

  QUrl url;
  url.setScheme("wmdm");
  url.setPath(path_components->join("/"));

  // Store the metadata in here
  Song song;
  song.InitFromWmdm(metadata);
  song.set_directory_id(1);
  song.set_url(url);

  metadata->Release();

  if (song.is_valid())
    songs_ << song;
}


