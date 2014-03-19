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
#include "core/application.h"
#include "core/database.h"
#include "core/tagreaderclient.h"
#include "core/taskmanager.h"
#include "smartplaylists/generator.h"
#include "smartplaylists/querygenerator.h"
#include "smartplaylists/search.h"

#include <QThread>

const char* Library::kSongsTable = "songs";
const char* Library::kDirsTable = "directories";
const char* Library::kSubdirsTable = "subdirectories";
const char* Library::kFtsTable = "songs_fts";

Library::Library(Application* app, QObject* parent)
    : QObject(parent),
      app_(app),
      backend_(nullptr),
      model_(nullptr),
      watcher_(nullptr),
      watcher_thread_(nullptr) {
  backend_ = new LibraryBackend;
  backend()->moveToThread(app->database()->thread());

  backend_->Init(app->database(), kSongsTable, kDirsTable, kSubdirsTable,
                 kFtsTable);

  using smart_playlists::Generator;
  using smart_playlists::GeneratorPtr;
  using smart_playlists::QueryGenerator;
  using smart_playlists::Search;
  using smart_playlists::SearchTerm;

  model_ = new LibraryModel(backend_, app_, this);
  model_->set_show_smart_playlists(true);
  model_->set_default_smart_playlists(
      LibraryModel::DefaultGenerators()
      << (LibraryModel::GeneratorList()
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "50 random tracks"),
                 Search(Search::Type_All, Search::TermList(),
                        Search::Sort_Random, SearchTerm::Field_Title, 50)))
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Ever played"),
                 Search(Search::Type_And,
                        Search::TermList()
                            << SearchTerm(SearchTerm::Field_PlayCount,
                                          SearchTerm::Op_GreaterThan, 0),
                        Search::Sort_Random, SearchTerm::Field_Title)))
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Never played"),
                 Search(Search::Type_And,
                        Search::TermList()
                            << SearchTerm(SearchTerm::Field_PlayCount,
                                          SearchTerm::Op_Equals, 0),
                        Search::Sort_Random, SearchTerm::Field_Title)))
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Last played"),
                 Search(Search::Type_All, Search::TermList(),
                        Search::Sort_FieldDesc, SearchTerm::Field_LastPlayed)))
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Most played"),
                 Search(Search::Type_All, Search::TermList(),
                        Search::Sort_FieldDesc, SearchTerm::Field_PlayCount)))
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Favourite tracks"),
                 Search(Search::Type_All, Search::TermList(),
                        Search::Sort_FieldDesc, SearchTerm::Field_Score)))
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Newest tracks"),
                 Search(Search::Type_All, Search::TermList(),
                        Search::Sort_FieldDesc,
                        SearchTerm::Field_DateCreated))))
      << (LibraryModel::GeneratorList()
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "All tracks"),
                 Search(Search::Type_All, Search::TermList(),
                        Search::Sort_FieldAsc, SearchTerm::Field_Artist, -1)))
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Least favourite tracks"),
                 Search(Search::Type_Or,
                        Search::TermList()
                            << SearchTerm(SearchTerm::Field_Rating,
                                          SearchTerm::Op_LessThan, 0.6)
                            << SearchTerm(SearchTerm::Field_SkipCount,
                                          SearchTerm::Op_GreaterThan, 4),
                        Search::Sort_FieldDesc, SearchTerm::Field_SkipCount))))
      << (LibraryModel::GeneratorList()
          << GeneratorPtr(new QueryGenerator(
                 QT_TRANSLATE_NOOP("Library", "Dynamic random mix"),
                 Search(Search::Type_All, Search::TermList(),
                        Search::Sort_Random, SearchTerm::Field_Title),
                 true))));

  // full rescan revisions
  full_rescan_revisions_[26] = tr("CUE sheet support");
}

Library::~Library() {
  watcher_->deleteLater();
  watcher_thread_->exit();
  watcher_thread_->wait(5000 /* five seconds */);
}

void Library::Init() {
  watcher_ = new LibraryWatcher;
  watcher_thread_ = new QThread(this);

  watcher_->moveToThread(watcher_thread_);
  watcher_thread_->start(QThread::IdlePriority);

  watcher_->set_backend(backend_);
  watcher_->set_task_manager(app_->task_manager());

  connect(backend_, SIGNAL(DirectoryDiscovered(Directory, SubdirectoryList)),
          watcher_, SLOT(AddDirectory(Directory, SubdirectoryList)));
  connect(backend_, SIGNAL(DirectoryDeleted(Directory)), watcher_,
          SLOT(RemoveDirectory(Directory)));
  connect(watcher_, SIGNAL(NewOrUpdatedSongs(SongList)), backend_,
          SLOT(AddOrUpdateSongs(SongList)));
  connect(watcher_, SIGNAL(SongsMTimeUpdated(SongList)), backend_,
          SLOT(UpdateMTimesOnly(SongList)));
  connect(watcher_, SIGNAL(SongsDeleted(SongList)), backend_,
          SLOT(MarkSongsUnavailable(SongList)));
  connect(watcher_, SIGNAL(SongsReadded(SongList, bool)), backend_,
          SLOT(MarkSongsUnavailable(SongList, bool)));
  connect(watcher_, SIGNAL(SubdirsDiscovered(SubdirectoryList)), backend_,
          SLOT(AddOrUpdateSubdirs(SubdirectoryList)));
  connect(watcher_, SIGNAL(SubdirsMTimeUpdated(SubdirectoryList)), backend_,
          SLOT(AddOrUpdateSubdirs(SubdirectoryList)));
  connect(watcher_, SIGNAL(CompilationsNeedUpdating()), backend_,
          SLOT(UpdateCompilations()));

  // This will start the watcher checking for updates
  backend_->LoadDirectoriesAsync();
}

void Library::IncrementalScan() { watcher_->IncrementalScanAsync(); }

void Library::FullScan() { watcher_->FullScanAsync(); }

void Library::PauseWatcher() { watcher_->SetRescanPausedAsync(true); }

void Library::ResumeWatcher() { watcher_->SetRescanPausedAsync(false); }

void Library::ReloadSettings() {
  backend_->ReloadSettingsAsync();
  watcher_->ReloadSettingsAsync();
}

void Library::WriteAllSongsStatisticsToFiles() {
  const SongList all_songs = backend_->GetAllSongs();

  const int task_id = app_->task_manager()->StartTask(
      tr("Saving songs statistics into songs files"));
  app_->task_manager()->SetTaskBlocksLibraryScans(task_id);

  const int nb_songs = all_songs.size();
  int i = 0;
  for (const Song& song : all_songs) {
    TagReaderClient::Instance()->UpdateSongStatisticsBlocking(song);
    TagReaderClient::Instance()->UpdateSongRatingBlocking(song);
    app_->task_manager()->SetTaskProgress(task_id, ++i, nb_songs);
  }
  app_->task_manager()->SetTaskFinished(task_id);
}
