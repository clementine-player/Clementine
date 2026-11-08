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

// Loads local files, playlists and directories, with real tag reader workers
// and an in-memory library. Loading remote URLs, which needs GStreamer, is
// covered by the remote playback smoke tests in tools/remote-cli.

#include "core/songloader.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QUrl>
#include <memory>

#include "core/database.h"
#include "gtest/gtest.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "tagreader_env.h"
#include "test_utils.h"

namespace {

::testing::Environment* const kTagReader =
    ::testing::AddGlobalTestEnvironment(new TagReaderEnvironment);

class SongLoaderTest : public ::testing::Test {
 protected:
  void SetUp() {
    database_.reset(new MemoryDatabase(nullptr));
    library_.reset(new LibraryBackend);
    library_->Init(database_.get(), Library::kSongsTable, Library::kDirsTable,
                   Library::kSubdirsTable, Library::kFtsTable);
    loader_.reset(new SongLoader(library_.get(), nullptr));
  }

  // Loads |url| as SongLoaderInserter does, blocking if the loader needs to.
  SongLoader::Result Load(const QUrl& url) {
    SongLoader::Result result = loader_->Load(url);
    if (result == SongLoader::BlockingLoadRequired) {
      result = loader_->LoadFilenamesBlocking();
      loader_->LoadMetadataBlocking();
    }
    return result;
  }

  std::unique_ptr<Database> database_;
  std::unique_ptr<LibraryBackend> library_;
  std::unique_ptr<SongLoader> loader_;
};

TEST_F(SongLoaderTest, LoadsLocalMp3) {
  TemporaryResource file(":/testdata/beep.mp3");
  ASSERT_EQ(SongLoader::Success, Load(QUrl::fromLocalFile(file.fileName())));

  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_TRUE(loader_->songs()[0].is_valid());
  EXPECT_EQ("Beep mp3", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadsLocalMp3FromLibrary) {
  TemporaryResource file(":/testdata/beep.mp3");
  const QUrl url = QUrl::fromLocalFile(file.fileName());

  library_->AddDirectory(QDir::tempPath());
  Song song;
  song.set_directory_id(1);
  song.set_url(url);
  song.set_mtime(1);
  song.set_ctime(1);
  song.set_filesize(1);
  song.set_title("Title in the library");
  library_->AddOrUpdateSongs(SongList() << song);

  // Found in the library, so there's nothing to wait for.
  ASSERT_EQ(SongLoader::Success, loader_->Load(url));
  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_EQ("Title in the library", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadsLocalPls) {
  TemporaryResource file(":/testdata/pls_somafm.pls");
  ASSERT_EQ(SongLoader::Success, Load(QUrl::fromLocalFile(file.fileName())));

  ASSERT_EQ(4, loader_->songs().count());
  EXPECT_EQ(
      "SomaFM: Groove Salad (#3 128k mp3): A nicely chilled plate of ambient "
      "beats and grooves.",
      loader_->songs()[2].title());
  EXPECT_EQ(QUrl("http://ice.somafm.com/groovesalad"),
            loader_->songs()[3].url());
}

TEST_F(SongLoaderTest, LoadsLocalM3U) {
  TemporaryResource file(":/testdata/test.m3u");
  ASSERT_EQ(SongLoader::Success, Load(QUrl::fromLocalFile(file.fileName())));

  EXPECT_EQ(239, loader_->songs().count());
}

TEST_F(SongLoaderTest, LoadsLocalXSPF) {
  TemporaryResource file(":/testdata/test.xspf");
  ASSERT_EQ(SongLoader::Success, Load(QUrl::fromLocalFile(file.fileName())));

  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_EQ("Foo", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadsLocalASX) {
  TemporaryResource file(":/testdata/test.asx");
  ASSERT_EQ(SongLoader::Success, Load(QUrl::fromLocalFile(file.fileName())));

  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_EQ("Foo", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadsLocalDirectory) {
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());

  QFile resource(":/testdata/beep.mp3");
  ASSERT_TRUE(resource.open(QIODevice::ReadOnly));
  const QByteArray data(resource.readAll());
  for (int i = 0; i < 3; ++i) {
    QFile mp3(dir.filePath(QString("%1.mp3").arg(i)));
    ASSERT_TRUE(mp3.open(QIODevice::WriteOnly));
    mp3.write(data);
  }

  // And one file that isn't music.
  QFile something_else(dir.filePath("somethingelse.foo"));
  ASSERT_TRUE(something_else.open(QIODevice::WriteOnly));
  something_else.write("I'm not an MP3!");
  something_else.close();

  ASSERT_EQ(SongLoader::Success, Load(QUrl::fromLocalFile(dir.path())));

  ASSERT_EQ(3, loader_->songs().count());
  for (const Song& song : loader_->songs()) {
    EXPECT_EQ("Beep mp3", song.title());
  }
}

}  // namespace
