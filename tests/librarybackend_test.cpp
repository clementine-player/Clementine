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

#include "test_utils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "librarybackend.h"
#include "song.h"

#include <boost/scoped_ptr.hpp>

#include <QtDebug>
#include <QThread>
#include <QSignalSpy>

using ::testing::_;
using ::testing::AtMost;
using ::testing::Invoke;
using ::testing::Return;

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

class LibraryBackendTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    backend_.reset(new LibraryBackend(NULL, ":memory:"));

    connection_name_ = "thread_" + QString::number(
        reinterpret_cast<quint64>(QThread::currentThread()));
    database_ = QSqlDatabase::database(connection_name_);
  }

  void TearDown() {
    // Make sure Qt does not re-use the connection.
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection_name_);
  }

  Song MakeDummySong(int directory_id) {
    // Returns a valid song with all the required fields set
    Song ret;
    ret.set_directory_id(directory_id);
    ret.set_filename("foo.mp3");
    ret.set_mtime(0);
    ret.set_ctime(0);
    ret.set_filesize(0);
    return ret;
  }

  boost::scoped_ptr<LibraryBackend> backend_;
  QString connection_name_;
  QSqlDatabase database_;
};

#ifdef Q_OS_LINUX

#include <sys/time.h>
#include <time.h>

struct PerfTimer {
  PerfTimer(int iterations) : iterations_(iterations) {
    gettimeofday(&start_time_, NULL);
  }

  ~PerfTimer() {
    gettimeofday(&end_time_, NULL);

    timeval elapsed_time;
    timersub(&end_time_, &start_time_, &elapsed_time);
    int elapsed_us = elapsed_time.tv_usec + elapsed_time.tv_sec * 1000000;

    qDebug() << "Elapsed:" << elapsed_us << "us";
    qDebug() << "Time per iteration:" << float(elapsed_us) / iterations_ << "us";
  }

  timeval start_time_;
  timeval end_time_;
  int iterations_;
};

TEST_F(LibraryBackendTest, LikePerformance) {
  const int iterations = 1000000;

  const char* needle = "foo";
  const char* haystack = "foobarbaz foobarbaz";
  qDebug() << "Simple query";
  {
    PerfTimer perf(iterations);
    for (int i = 0; i < iterations; ++i) {
      backend_->Like(needle, haystack);
    }
  }
}

#endif

TEST_F(LibraryBackendTest, DatabaseInitialises) {
  // Check that these tables exist
  QStringList tables = database_.tables();
  EXPECT_TRUE(tables.contains("songs"));
  EXPECT_TRUE(tables.contains("directories"));
  ASSERT_TRUE(tables.contains("schema_version"));

  // Check the schema version is correct
  QSqlQuery q("SELECT version FROM schema_version", database_);
  ASSERT_TRUE(q.exec());
  ASSERT_TRUE(q.next());
  EXPECT_EQ(LibraryBackend::kSchemaVersion, q.value(0).toInt());
  EXPECT_FALSE(q.next());
}

TEST_F(LibraryBackendTest, EmptyDatabase) {
  // Check the database is empty to start with
  QStringList artists = backend_->GetAllArtists();
  EXPECT_TRUE(artists.isEmpty());

  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  EXPECT_TRUE(albums.isEmpty());
}

TEST_F(LibraryBackendTest, AddDirectory) {
  QSignalSpy spy(backend_.get(), SIGNAL(DirectoriesDiscovered(DirectoryList)));

  backend_->AddDirectory("/test");

  // Check the signal was emitted correctly
  ASSERT_EQ(1, spy.count());
  DirectoryList list = spy[0][0].value<DirectoryList>();
  ASSERT_EQ(1, list.size());
  EXPECT_EQ("/test", list[0].path);
  EXPECT_EQ(1, list[0].id);
}

TEST_F(LibraryBackendTest, RemoveDirectory) {
  // Add a directory
  Directory dir;
  dir.id = 1;
  dir.path = "/test";
  backend_->AddDirectory(dir.path);

  QSignalSpy spy(backend_.get(), SIGNAL(DirectoriesDeleted(DirectoryList)));

  // Remove the directory again
  backend_->RemoveDirectory(dir);

  // Check the signal was emitted correctly
  ASSERT_EQ(1, spy.count());
  DirectoryList list = spy[0][0].value<DirectoryList>();
  ASSERT_EQ(1, list.size());
  EXPECT_EQ("/test", list[0].path);
  EXPECT_EQ(1, list[0].id);
}

TEST_F(LibraryBackendTest, AddInvalidSong) {
  // Adding a song without certain fields set should fail
  backend_->AddDirectory("/test");
  Song s;
  s.set_directory_id(1);

  QSignalSpy spy(backend_.get(), SIGNAL(Error(QString)));

  backend_->AddOrUpdateSongs(SongList() << s);
  ASSERT_EQ(1, spy.count()); spy.takeFirst();

  s.set_filename("foo");
  backend_->AddOrUpdateSongs(SongList() << s);
  ASSERT_EQ(1, spy.count()); spy.takeFirst();

  s.set_filesize(100);
  backend_->AddOrUpdateSongs(SongList() << s);
  ASSERT_EQ(1, spy.count()); spy.takeFirst();

  s.set_mtime(100);
  backend_->AddOrUpdateSongs(SongList() << s);
  ASSERT_EQ(1, spy.count()); spy.takeFirst();

  s.set_ctime(100);
  backend_->AddOrUpdateSongs(SongList() << s);
  ASSERT_EQ(0, spy.count());
}

TEST_F(LibraryBackendTest, GetAlbumArtNonExistent) {
}

TEST_F(LibraryBackendTest, LikeWorksWithAllAscii) {
  EXPECT_TRUE(backend_->Like("%ar%", "bar"));
  EXPECT_FALSE(backend_->Like("%ar%", "foo"));
}

TEST_F(LibraryBackendTest, LikeWorksWithUnicode) {
  EXPECT_TRUE(backend_->Like("%Снег%", "Снег"));
  EXPECT_FALSE(backend_->Like("%Снег%", "foo"));
}

TEST_F(LibraryBackendTest, LikeAsciiCaseInsensitive) {
  EXPECT_TRUE(backend_->Like("%ar%", "BAR"));
  EXPECT_FALSE(backend_->Like("%ar%", "FOO"));
}

TEST_F(LibraryBackendTest, LikeUnicodeCaseInsensitive) {
  EXPECT_TRUE(backend_->Like("%снег%", "Снег"));
}

TEST_F(LibraryBackendTest, LikeCacheInvalidated) {
  EXPECT_TRUE(backend_->Like("%foo%", "foobar"));
  EXPECT_FALSE(backend_->Like("%baz%", "foobar"));
}

TEST_F(LibraryBackendTest, LikeQuerySplit) {
  EXPECT_TRUE(backend_->Like("%foo bar%", "foobar"));
  EXPECT_TRUE(backend_->Like("%foo bar%", "barbaz"));
  EXPECT_TRUE(backend_->Like("%foo bar%", "foobaz"));
  EXPECT_FALSE(backend_->Like("%foo bar%", "baz"));
}

// Test adding a single song to the database, then getting various information
// back about it.
class SingleSong : public LibraryBackendTest {
 protected:
  virtual void SetUp() {
    LibraryBackendTest::SetUp();

    // Add a directory - this will get ID 1
    backend_->AddDirectory("/test");

    // Make a song in that directory
    song_ = MakeDummySong(1);
    song_.set_title("Title");
    song_.set_artist("Artist");
    song_.set_album("Album");
  }

  void AddDummySong() {
    QSignalSpy added_spy(backend_.get(), SIGNAL(SongsDiscovered(SongList)));
    QSignalSpy deleted_spy(backend_.get(), SIGNAL(SongsDeleted(SongList)));

    // Add the song
    backend_->AddOrUpdateSongs(SongList() << song_);

    // Check the correct signals were emitted
    EXPECT_EQ(0, deleted_spy.count());
    ASSERT_EQ(1, added_spy.count());

    SongList list = added_spy[0][0].value<SongList>();
    ASSERT_EQ(1, list.count());
    EXPECT_EQ(song_.title(), list[0].title());
    EXPECT_EQ(song_.artist(), list[0].artist());
    EXPECT_EQ(song_.album(), list[0].album());
    EXPECT_EQ(1, list[0].id());
    EXPECT_EQ(1, list[0].directory_id());
  }

  Song song_;
};

TEST_F(SingleSong, GetSongWithNoAlbum) {
  song_.set_album("");
  AddDummySong(); if (HasFatalFailure()) return;
  
  EXPECT_EQ(1, backend_->GetAllArtists().size());
  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  EXPECT_EQ(1, albums.size());
  EXPECT_EQ("Artist", albums[0].artist);
  EXPECT_EQ("", albums[0].album_name);
}

TEST_F(SingleSong, GetAllArtists) {
  AddDummySong();  if (HasFatalFailure()) return;

  QStringList artists = backend_->GetAllArtists();
  ASSERT_EQ(1, artists.size());
  EXPECT_EQ(song_.artist(), artists[0]);
}

TEST_F(SingleSong, GetAllAlbums) {
  AddDummySong();  if (HasFatalFailure()) return;

  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  ASSERT_EQ(1, albums.size());
  EXPECT_EQ(song_.album(), albums[0].album_name);
  EXPECT_EQ(song_.artist(), albums[0].artist);
}

TEST_F(SingleSong, GetAlbumsByArtist) {
  AddDummySong();  if (HasFatalFailure()) return;

  LibraryBackend::AlbumList albums = backend_->GetAlbumsByArtist("Artist");
  ASSERT_EQ(1, albums.size());
  EXPECT_EQ(song_.album(), albums[0].album_name);
  EXPECT_EQ(song_.artist(), albums[0].artist);
}

TEST_F(SingleSong, GetAlbumArt) {
  AddDummySong();  if (HasFatalFailure()) return;

  LibraryBackend::Album album = backend_->GetAlbumArt("Artist", "Album");
  EXPECT_EQ(song_.album(), album.album_name);
  EXPECT_EQ(song_.artist(), album.artist);
}

TEST_F(SingleSong, GetSongs) {
  AddDummySong();  if (HasFatalFailure()) return;

  SongList songs = backend_->GetSongs("Artist", "Album");
  ASSERT_EQ(1, songs.size());
  EXPECT_EQ(song_.album(), songs[0].album());
  EXPECT_EQ(song_.artist(), songs[0].artist());
  EXPECT_EQ(song_.title(), songs[0].title());
  EXPECT_EQ(1, songs[0].id());
}

TEST_F(SingleSong, GetSongById) {
  AddDummySong();  if (HasFatalFailure()) return;

  Song song = backend_->GetSongById(1);
  EXPECT_EQ(song_.album(), song.album());
  EXPECT_EQ(song_.artist(), song.artist());
  EXPECT_EQ(song_.title(), song.title());
  EXPECT_EQ(1, song.id());
}

TEST_F(SingleSong, FindSongsInDirectory) {
  AddDummySong();  if (HasFatalFailure()) return;

  SongList songs = backend_->FindSongsInDirectory(1);
  ASSERT_EQ(1, songs.size());
  EXPECT_EQ(song_.album(), songs[0].album());
  EXPECT_EQ(song_.artist(), songs[0].artist());
  EXPECT_EQ(song_.title(), songs[0].title());
  EXPECT_EQ(1, songs[0].id());
}

TEST_F(SingleSong, UpdateSong) {
  AddDummySong();  if (HasFatalFailure()) return;

  Song new_song(song_);
  new_song.set_id(1);
  new_song.set_title("A different title");

  QSignalSpy deleted_spy(backend_.get(), SIGNAL(SongsDeleted(SongList)));
  QSignalSpy added_spy(backend_.get(), SIGNAL(SongsDiscovered(SongList)));

  backend_->AddOrUpdateSongs(SongList() << new_song);

  ASSERT_EQ(1, added_spy.size());
  ASSERT_EQ(1, deleted_spy.size());

  SongList songs_added = added_spy[0][0].value<SongList>();
  SongList songs_deleted = deleted_spy[0][0].value<SongList>();
  ASSERT_EQ(1, songs_added.size());
  ASSERT_EQ(1, songs_deleted.size());
  EXPECT_EQ("Title", songs_deleted[0].title());
  EXPECT_EQ("A different title", songs_added[0].title());
  EXPECT_EQ(1, songs_deleted[0].id());
  EXPECT_EQ(1, songs_added[0].id());
}

TEST_F(SingleSong, DeleteSongs) {
  AddDummySong();  if (HasFatalFailure()) return;

  Song new_song(song_);
  new_song.set_id(1);

  QSignalSpy deleted_spy(backend_.get(), SIGNAL(SongsDeleted(SongList)));

  backend_->DeleteSongs(SongList() << new_song);

  ASSERT_EQ(1, deleted_spy.size());

  SongList songs_deleted = deleted_spy[0][0].value<SongList>();
  ASSERT_EQ(1, songs_deleted.size());
  EXPECT_EQ("Title", songs_deleted[0].title());
  EXPECT_EQ(1, songs_deleted[0].id());

  // Check we can't retreive that song any more
  Song song = backend_->GetSongById(1);
  EXPECT_FALSE(song.is_valid());
  EXPECT_EQ(-1, song.id());

  // And the artist or album shouldn't show up either
  QStringList artists = backend_->GetAllArtists();
  EXPECT_EQ(0, artists.size());

  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  EXPECT_EQ(0, albums.size());
}
