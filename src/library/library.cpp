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

#include "library.h"
#include "librarymodel.h"
#include "librarybackend.h"
#include "core/database.h"
#include "smartplaylists/generator.h"
#include "smartplaylists/querygenerator.h"
#include "smartplaylists/search.h"

const char* Library::kSongsTable = "songs";
const char* Library::kDirsTable = "directories";
const char* Library::kSubdirsTable = "subdirectories";
const char* Library::kFtsTable = "songs_fts";

Library::Library(BackgroundThread<Database>* db_thread, TaskManager* task_manager,
                 QObject *parent)
  : QObject(parent),
    task_manager_(task_manager),
    backend_(NULL),
    model_(NULL),
    watcher_factory_(new BackgroundThreadFactoryImplementation<LibraryWatcher, LibraryWatcher>),
    watcher_(NULL)
{
  backend_ = new LibraryBackend;
  backend()->moveToThread(db_thread);

  backend_->Init(db_thread->Worker(), kSongsTable, kDirsTable, kSubdirsTable, kFtsTable);

  using smart_playlists::Generator;
  using smart_playlists::GeneratorPtr;
  using smart_playlists::QueryGenerator;
  using smart_playlists::Search;
  using smart_playlists::SearchTerm;

  model_ = new LibraryModel(backend_, this);
  model_->set_show_smart_playlists(true);
  model_->set_default_smart_playlists(LibraryModel::DefaultGenerators()
    << (LibraryModel::GeneratorList()
      << GeneratorPtr(new QueryGenerator(tr("50 random tracks"), Search(
                  Search::Type_All, Search::TermList(),
                  Search::Sort_Random, SearchTerm::Field_Title, 50)))
      << GeneratorPtr(new QueryGenerator(tr("Ever played"), Search(
                  Search::Type_And, Search::TermList()
                    << SearchTerm(SearchTerm::Field_PlayCount, SearchTerm::Op_GreaterThan, 0),
                  Search::Sort_Random, SearchTerm::Field_Title)))
      << GeneratorPtr(new QueryGenerator(tr("Never played"), Search(
                  Search::Type_And, Search::TermList()
                    << SearchTerm(SearchTerm::Field_PlayCount, SearchTerm::Op_Equals, 0),
                  Search::Sort_Random, SearchTerm::Field_Title)))
      << GeneratorPtr(new QueryGenerator(tr("Last played"), Search(
                  Search::Type_All, Search::TermList(),
                  Search::Sort_FieldDesc, SearchTerm::Field_LastPlayed)))
      << GeneratorPtr(new QueryGenerator(tr("Most played"), Search(
                  Search::Type_All, Search::TermList(),
                  Search::Sort_FieldDesc, SearchTerm::Field_PlayCount)))
      << GeneratorPtr(new QueryGenerator(tr("Favourite tracks"), Search(
                  Search::Type_All, Search::TermList(),
                  Search::Sort_FieldDesc, SearchTerm::Field_Score)))
      << GeneratorPtr(new QueryGenerator(tr("Newest tracks"), Search(
                  Search::Type_All, Search::TermList(),
                  Search::Sort_FieldDesc, SearchTerm::Field_DateCreated)))
    ) << (LibraryModel::GeneratorList()
      << GeneratorPtr(new QueryGenerator(tr("All tracks"), Search(
                  Search::Type_All, Search::TermList(),
                  Search::Sort_FieldAsc, SearchTerm::Field_Artist, -1)))
      << GeneratorPtr(new QueryGenerator(tr("Least favourite tracks"), Search(
                  Search::Type_Or, Search::TermList()
                    << SearchTerm(SearchTerm::Field_Rating, SearchTerm::Op_LessThan, 0.6)
                    << SearchTerm(SearchTerm::Field_SkipCount, SearchTerm::Op_GreaterThan, 4),
                  Search::Sort_FieldDesc, SearchTerm::Field_SkipCount)))
    ) << (LibraryModel::GeneratorList()
      << GeneratorPtr(new QueryGenerator(tr("Dynamic random mix"), Search(
                  Search::Type_All, Search::TermList(),
                  Search::Sort_Random, SearchTerm::Field_Title), true))
    )
  );
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

  watcher->set_backend(backend_);
  watcher->set_task_manager(task_manager_);

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
  connect(watcher, SIGNAL(CompilationsNeedUpdating()),
          backend_, SLOT(UpdateCompilations()));

  // This will start the watcher checking for updates
  backend_->LoadDirectoriesAsync();
}

void Library::IncrementalScan() {
  if (!watcher_->Worker())
    return;

  watcher_->Worker()->IncrementalScanAsync();
}

void Library::PauseWatcher() {
  if (!watcher_->Worker())
    return;

  watcher_->Worker()->SetRescanPausedAsync(true);
}

void Library::ResumeWatcher() {
  if (!watcher_->Worker())
    return;

  watcher_->Worker()->SetRescanPausedAsync(false);
}

void Library::ReloadSettings() {
  if (!watcher_->Worker())
    return;

  watcher_->Worker()->ReloadSettingsAsync();
}
