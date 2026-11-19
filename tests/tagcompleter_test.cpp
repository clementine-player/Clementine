/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include <QCoreApplication>
#include <QLineEdit>
#include <QPointer>
#include <QStringListModel>
#include <QTemporaryDir>
#include <QTest>
#include <QThreadPool>
#include <memory>

#include "core/database.h"
#include "core/song.h"
#include "gtest/gtest.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "playlist/playlist.h"
#include "playlist/playlistdelegates.h"

namespace {

class TagCompleterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_TRUE(dir_.isValid());
    // A file, not a MemoryDatabase: the completer reads the library on
    // another thread, which would get an empty database of its own.
    database_ = std::make_unique<Database>(nullptr, nullptr,
                                           dir_.filePath("clementine.db"));
    backend_ = std::make_unique<LibraryBackend>();
    backend_->Init(database_.get(), Library::kSongsTable, Library::kDirsTable,
                   Library::kSubdirsTable, Library::kFtsTable);
    backend_->AddDirectory("/music");

    Song song;
    song.Init("Title", "Artist", "Album", 100);
    song.set_directory_id(1);
    song.set_url(QUrl::fromLocalFile("/music/song.mp3"));
    song.set_mtime(1);
    song.set_ctime(1);
    song.set_filesize(1);
    backend_->AddOrUpdateSongs(SongList() << song);
  }

  // Finishes the lookups the completers started, and delivers their results.
  void FinishLookups() {
    QThreadPool::globalInstance()->waitForDone();
    QCoreApplication::processEvents();
  }

  QTemporaryDir dir_;
  std::unique_ptr<Database> database_;
  std::unique_ptr<LibraryBackend> backend_;
};

TEST_F(TagCompleterTest, CompletesWithTheLibrarysValues) {
  QLineEdit editor;
  TagCompleter* completer =
      new TagCompleter(backend_.get(), Playlist::Column_Artist, &editor);
  EXPECT_EQ(nullptr, editor.completer());

  FinishLookups();

  ASSERT_EQ(completer, editor.completer());
  QStringListModel* model = qobject_cast<QStringListModel*>(completer->model());
  ASSERT_NE(nullptr, model);
  EXPECT_EQ(QStringList() << "Artist", model->stringList());
  // The completer owns its model, so it goes with the editor.
  EXPECT_EQ(completer, model->parent());
}

// The playlist's editor can be closed, deleting the completer, before the
// library has been read (CLEMENTINE-7).
TEST_F(TagCompleterTest, EditorClosedBeforeTheValuesArrive) {
  QLineEdit* editor = new QLineEdit;
  QPointer<TagCompleter> completer =
      new TagCompleter(backend_.get(), Playlist::Column_Artist, editor);

  delete editor;
  EXPECT_TRUE(completer.isNull());

  FinishLookups();
}

}  // namespace
