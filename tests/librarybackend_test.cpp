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

#include <memory>

#include "test_utils.h"
#include "gtest/gtest.h"

#include <QFileInfo>
#include <QSignalSpy>
#include <QThread>
#include <QtDebug>

#include "library/librarybackend.h"
#include "library/library.h"
#include "core/song.h"
#include "core/database.h"

namespace {

class LibraryBackendTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    database_.reset(new MemoryDatabase);
    backend_.reset(new LibraryBackend);
    backend_->Init(database_, Library::kSongsTable,
                   Library::kDirsTable, Library::kSubdirsTable,
                   Library::kFtsTable);
  }

  Song MakeDummySong(int directory_id) {
    // Returns a valid song with all the required fields set
    Song ret;
    ret.set_directory_id(directory_id);
    ret.set_url(QUrl::fromLocalFile("foo.mp3"));
    ret.set_mtime(1);
    ret.set_ctime(1);
    ret.set_filesize(1);
    return ret;
  }

  std::shared_ptr<Database> database_;
  std::unique_ptr<LibraryBackend> backend_;
};

TEST_F(LibraryBackendTest, EmptyDatabase) {
  // Check the database is empty to start with
  QStringList artists = backend_->GetAllArtists();
  EXPECT_TRUE(artists.isEmpty());

  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  EXPECT_TRUE(albums.isEmpty());
}

TEST_F(LibraryBackendTest, AddDirectory) {
  QSignalSpy spy(backend_.get(), SIGNAL(DirectoryDiscovered(Directory, SubdirectoryList)));

  backend_->AddDirectory("/tmp");

  // Check the signal was emitted correctly
  ASSERT_EQ(1, spy.count());
  Directory dir = spy[0][0].value<Directory>();
  EXPECT_EQ(QFileInfo("/tmp").canonicalFilePath(), dir.path);
  EXPECT_EQ(1, dir.id);
  EXPECT_EQ(0, spy[0][1].value<SubdirectoryList>().size());
}

TEST_F(LibraryBackendTest, RemoveDirectory) {
  // Add a directory
  Directory dir;
  dir.id = 1;
  dir.path = "/tmp";
  backend_->AddDirectory(dir.path);

  QSignalSpy spy(backend_.get(), SIGNAL(DirectoryDeleted(Directory)));

  // Remove the directory again
  backend_->RemoveDirectory(dir);

  // Check the signal was emitted correctly
  ASSERT_EQ(1, spy.count());
  dir = spy[0][0].value<Directory>();
  EXPECT_EQ("/tmp", dir.path);
  EXPECT_EQ(1, dir.id);
}

TEST_F(LibraryBackendTest, AddInvalidSong) {
  // Adding a song without certain fields set should fail
  backend_->AddDirectory("/tmp");
  Song s;
  s.set_directory_id(1);

  QSignalSpy spy(database_.get(), SIGNAL(Error(QString)));

  backend_->AddOrUpdateSongs(SongList() << s);
  ASSERT_EQ(1, spy.count()); spy.takeFirst();

  s.set_url(QUrl::fromLocalFile("foo"));
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

// Test adding a single song to the database, then getting various information
// back about it.
class SingleSong : public LibraryBackendTest {
 protected:
  virtual void SetUp() {
    LibraryBackendTest::SetUp();

    // Add a directory - this will get ID 1
    backend_->AddDirectory("/tmp");

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

    SongList list = *(reinterpret_cast<SongList*>(added_spy[0][0].data()));
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

  SongList songs_added = *(reinterpret_cast<SongList*>(added_spy[0][0].data()));
  SongList songs_deleted = *(reinterpret_cast<SongList*>(deleted_spy[0][0].data()));
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

  SongList songs_deleted = *(reinterpret_cast<SongList*>(deleted_spy[0][0].data()));
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

TEST_F(SingleSong, MarkSongsUnavailable) {
  AddDummySong();  if (HasFatalFailure()) return;

  Song new_song(song_);
  new_song.set_id(1);

  QSignalSpy deleted_spy(backend_.get(), SIGNAL(SongsDeleted(SongList)));

  backend_->MarkSongsUnavailable(SongList() << new_song);

  ASSERT_EQ(1, deleted_spy.size());

  SongList songs_deleted = *(reinterpret_cast<SongList*>(deleted_spy[0][0].data()));
  ASSERT_EQ(1, songs_deleted.size());
  EXPECT_EQ("Title", songs_deleted[0].title());
  EXPECT_EQ(1, songs_deleted[0].id());

  // Check the song is marked as deleted.
  Song song = backend_->GetSongById(1);
  EXPECT_TRUE(song.is_valid());
  EXPECT_TRUE(song.is_unavailable());

  // And the artist or album shouldn't show up either
  QStringList artists = backend_->GetAllArtists();
  EXPECT_EQ(0, artists.size());

  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  EXPECT_EQ(0, albums.size());
}

} // namespace
