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

#include "library.h"
#include "librarymodel.h"
#include "librarybackend.h"

const char* Library::kSongsTable = "songs";
const char* Library::kDirsTable = "directories";
const char* Library::kSubdirsTable = "subdirectories";

Library::Library(Database *db, QObject *parent)
  : QObject(parent),
    backend_(new LibraryBackend(db, kSongsTable, kDirsTable, kSubdirsTable, this)),
    model_(new LibraryModel(backend_, parent)),
    watcher_factory_(new BackgroundThreadFactoryImplementation<LibraryWatcher, LibraryWatcher>),
    watcher_(NULL)
{
}

void Library::set_watcher_factory(BackgroundThreadFactory<LibraryWatcher>* factory) {
  watcher_factory_.reset(factory);
}

void Library::Init() {
  watcher_ = watcher_factory_->GetThread(this);
  connect(watcher_, SIGNAL(Initialised()), SLOT(WatcherInitialised()));
}

void Library::StartThreads() {
  Q_ASSERT(watcher_);

  watcher_->set_io_priority(BackgroundThreadBase::IOPRIO_CLASS_IDLE);
  watcher_->set_cpu_priority(QThread::IdlePriority);
  watcher_->Start();

  model_->Init();
}

void Library::WatcherInitialised() {
  LibraryWatcher* watcher = watcher_->Worker().get();
  connect(watcher, SIGNAL(ScanStarted()), SIGNAL(ScanStarted()));
  connect(watcher, SIGNAL(ScanFinished()), SIGNAL(ScanFinished()));

  watcher->SetBackend(backend_);

  connect(backend_, SIGNAL(DirectoryDiscovered(Directory,SubdirectoryList)),
          watcher,  SLOT(AddDirectory(Directory,SubdirectoryList)));
  connect(backend_, SIGNAL(DirectoryDeleted(Directory)),
          watcher,  SLOT(RemoveDirectory(Directory)));
  connect(watcher,  SIGNAL(NewOrUpdatedSongs(SongList)),
          backend_, SLOT(AddOrUpdateSongs(SongList)));
  connect(watcher,  SIGNAL(SongsMTimeUpdated(SongList)),
          backend_, SLOT(UpdateMTimesOnly(SongList)));
  connect(watcher,  SIGNAL(SongsDeleted(SongList)),
          backend_, SLOT(DeleteSongs(SongList)));
  connect(watcher,  SIGNAL(SubdirsDiscovered(SubdirectoryList)),
          backend_, SLOT(AddOrUpdateSubdirs(SubdirectoryList)));
  connect(watcher,  SIGNAL(SubdirsMTimeUpdated(SubdirectoryList)),
          backend_, SLOT(AddOrUpdateSubdirs(SubdirectoryList)));

  // This will start the watcher checking for updates
  backend_->LoadDirectoriesAsync();
}

void Library::IncrementalScan() {
  if (!watcher_->Worker())
    return;

  watcher_->Worker()->IncrementalScanAsync();
}
