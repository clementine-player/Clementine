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

#include "library.h"
#include "backgroundthread.h"
#include "mock_backgroundthread.h"

#include <QtDebug>
#include <QThread>
#include <QSignalSpy>
#include <QSortFilterProxyModel>

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

namespace {

class LibraryTest : public ::testing::Test {
 protected:
  void SetUp() {
    library_ = new Library(
        static_cast<EngineBase*>(NULL), static_cast<QObject*>(NULL));
    library_->set_backend_factory(
        new FakeBackgroundThreadFactory<LibraryBackendInterface, MemoryLibraryBackend>);
    library_->set_watcher_factory(
        new FakeBackgroundThreadFactory<LibraryWatcher, LibraryWatcher>);

    library_->Init();

    added_dir_ = false;
    backend_ = library_->GetBackend().get();
    connection_name_ = "thread_" + QString::number(
        reinterpret_cast<quint64>(QThread::currentThread()));
    database_ = QSqlDatabase::database(connection_name_);

    library_sorted_ = new QSortFilterProxyModel;
    library_sorted_->setSourceModel(library_);
    library_sorted_->setSortRole(Library::Role_SortText);
    library_sorted_->setDynamicSortFilter(true);
    library_sorted_->sort(0);
  }

  void TearDown() {
    // Make sure Qt does not re-use the connection.
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection_name_);

    delete library_;
    delete library_sorted_;
  }

  Song AddSong(Song& song) {
    song.set_directory_id(1);
    if (song.mtime() == -1) song.set_mtime(0);
    if (song.ctime() == -1) song.set_ctime(0);
    if (song.filename().isNull()) song.set_filename("/test/foo");
    if (song.filesize() == -1) song.set_filesize(0);

    if (!added_dir_) {
      backend_->AddDirectory("/test");
      added_dir_ = true;
    }

    backend_->AddOrUpdateSongs(SongList() << song);
    return song;
  }

  Song AddSong(const QString& title, const QString& artist, const QString& album, int length) {
    Song song;
    song.Init(title, artist, album, length);
    return AddSong(song);
  }

  Library* library_;
  LibraryBackendInterface* backend_;
  QSortFilterProxyModel* library_sorted_;

  bool added_dir_;
  QString connection_name_;
  QSqlDatabase database_;
};

TEST_F(LibraryTest, Initialisation) {
  library_->StartThreads();

  EXPECT_EQ(0, library_->rowCount(QModelIndex()));
}

TEST_F(LibraryTest, WithInitialArtists) {
  AddSong("Title", "Artist 1", "Album", 123);
  AddSong("Title", "Artist 2", "Album", 123);
  AddSong("Title", "Foo", "Album", 123);
  library_->StartThreads();

  ASSERT_EQ(5, library_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("A", library_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 1", library_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 2", library_sorted_->index(2, 0, QModelIndex()).data().toString());
  EXPECT_EQ("F", library_sorted_->index(3, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Foo", library_sorted_->index(4, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryTest, CompilationAlbums) {
  Song song;
  song.Init("Title", "Artist", "Album", 123);
  song.set_compilation(true);

  AddSong(song);
  library_->StartThreads();

  ASSERT_EQ(1, library_->rowCount(QModelIndex()));

  QModelIndex va_index = library_->index(0, 0, QModelIndex());
  EXPECT_EQ("Various Artists", va_index.data().toString());
  EXPECT_TRUE(library_->hasChildren(va_index));
  ASSERT_EQ(library_->rowCount(va_index), 1);

  QModelIndex album_index = library_->index(0, 0, va_index);
  EXPECT_EQ(library_->data(album_index).toString(), "Album");
  EXPECT_TRUE(library_->hasChildren(album_index));
}

TEST_F(LibraryTest, NumericHeaders) {
  AddSong("Title", "1artist", "Album", 123);
  AddSong("Title", "2artist", "Album", 123);
  AddSong("Title", "0artist", "Album", 123);
  AddSong("Title", "zartist", "Album", 123);
  library_->StartThreads();

  ASSERT_EQ(6, library_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("0-9", library_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("0artist", library_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("1artist", library_sorted_->index(2, 0, QModelIndex()).data().toString());
  EXPECT_EQ("2artist", library_sorted_->index(3, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Z", library_sorted_->index(4, 0, QModelIndex()).data().toString());
  EXPECT_EQ("zartist", library_sorted_->index(5, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryTest, MixedCaseHeaders) {
  AddSong("Title", "Artist", "Album", 123);
  AddSong("Title", "artist", "Album", 123);
  library_->StartThreads();

  ASSERT_EQ(3, library_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("A", library_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist", library_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("artist", library_sorted_->index(2, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryTest, UnknownArtists) {
  AddSong("Title", "", "Album", 123);
  library_->StartThreads();

  ASSERT_EQ(1, library_->rowCount(QModelIndex()));
  QModelIndex unknown_index = library_->index(0, 0, QModelIndex());
  EXPECT_EQ("Unknown", unknown_index.data().toString());

  ASSERT_EQ(1, library_->rowCount(unknown_index));
  EXPECT_EQ("Album", library_->index(0, 0, unknown_index).data().toString());
}

TEST_F(LibraryTest, UnknownAlbums) {
  AddSong("Title", "Artist", "", 123);
  AddSong("Title", "Artist", "Album", 123);
  library_->StartThreads();

  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(2, library_->rowCount(artist_index));

  QModelIndex unknown_album_index = library_->index(0, 0, artist_index);
  QModelIndex real_album_index = library_->index(1, 0, artist_index);

  EXPECT_EQ("Unknown", unknown_album_index.data().toString());
  EXPECT_EQ("Album", real_album_index.data().toString());
}

TEST_F(LibraryTest, VariousArtistSongs) {
  SongList songs;
  for (int i=0 ; i<4 ; ++i) {
    QString n = QString::number(i+1);
    Song song;
    song.Init("Title " + n, "Artist " + n, "Album", 0);
    songs << song;
  }

  // Different ways of putting songs in "Various Artist".  Make sure they all work
  songs[0].set_sampler(true);
  songs[1].set_compilation(true);
  songs[2].set_forced_compilation_on(true);
  songs[3].set_sampler(true); songs[3].set_artist("Various Artists");

  for (int i=0 ; i<4 ; ++i)
    AddSong(songs[i]);
  library_->StartThreads();

  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, library_->rowCount(artist_index));

  QModelIndex album_index = library_->index(0, 0, artist_index);
  ASSERT_EQ(4, library_->rowCount(album_index));

  EXPECT_EQ("Artist 1 - Title 1", library_->index(0, 0, album_index).data().toString());
  EXPECT_EQ("Artist 2 - Title 2", library_->index(1, 0, album_index).data().toString());
  EXPECT_EQ("Artist 3 - Title 3", library_->index(2, 0, album_index).data().toString());
  EXPECT_EQ("Title 4", library_->index(3, 0, album_index).data().toString());
}

TEST_F(LibraryTest, RemoveSongsLazyLoaded) {
  Song one = AddSong("Title 1", "Artist", "Album", 123); one.set_id(1);
  Song two = AddSong("Title 2", "Artist", "Album", 123); two.set_id(2);
  AddSong("Title 3", "Artist", "Album", 123);
  library_->StartThreads();

  // Lazy load the items
  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, library_->rowCount(artist_index));
  QModelIndex album_index = library_->index(0, 0, artist_index);
  ASSERT_EQ(3, library_->rowCount(album_index));

  // Remove the first two songs
  QSignalSpy spy_preremove(library_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy spy_remove(library_, SIGNAL(rowsRemoved(QModelIndex,int,int)));
  QSignalSpy spy_reset(library_, SIGNAL(modelReset()));

  backend_->DeleteSongs(SongList() << one << two);

  ASSERT_EQ(2, spy_preremove.count());
  ASSERT_EQ(2, spy_remove.count());
  ASSERT_EQ(0, spy_reset.count());

  artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, library_->rowCount(artist_index));
  album_index = library_->index(0, 0, artist_index);
  ASSERT_EQ(1, library_->rowCount(album_index));
  EXPECT_EQ("Title 3", library_->index(0, 0, album_index).data().toString());
}

TEST_F(LibraryTest, RemoveSongsNotLazyLoaded) {
  Song one = AddSong("Title 1", "Artist", "Album", 123); one.set_id(1);
  Song two = AddSong("Title 2", "Artist", "Album", 123); two.set_id(2);
  library_->StartThreads();

  // Remove the first two songs
  QSignalSpy spy_preremove(library_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy spy_remove(library_, SIGNAL(rowsRemoved(QModelIndex,int,int)));
  QSignalSpy spy_reset(library_, SIGNAL(modelReset()));

  backend_->DeleteSongs(SongList() << one << two);

  ASSERT_EQ(0, spy_preremove.count());
  ASSERT_EQ(0, spy_remove.count());
  ASSERT_EQ(1, spy_reset.count());
}

TEST_F(LibraryTest, RemoveEmptyAlbums) {
  Song one = AddSong("Title 1", "Artist", "Album 1", 123); one.set_id(1);
  Song two = AddSong("Title 2", "Artist", "Album 2", 123); two.set_id(2);
  Song three = AddSong("Title 3", "Artist", "Album 2", 123); three.set_id(3);
  library_->StartThreads();

  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(2, library_->rowCount(artist_index));

  // Remove one song from each album
  backend_->DeleteSongs(SongList() << one << two);

  // Check the model
  artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, library_->rowCount(artist_index));
  QModelIndex album_index = library_->index(0, 0, artist_index);
  EXPECT_EQ("Album 2", album_index.data().toString());

  ASSERT_EQ(1, library_->rowCount(album_index));
  EXPECT_EQ("Title 3", library_->index(0, 0, album_index).data().toString());
}

TEST_F(LibraryTest, RemoveEmptyArtists) {
  Song one = AddSong("Title", "Artist", "Album", 123); one.set_id(1);
  library_->StartThreads();

  // Lazy load the items
  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, library_->rowCount(artist_index));
  QModelIndex album_index = library_->index(0, 0, artist_index);
  ASSERT_EQ(1, library_->rowCount(album_index));

  // The artist header is there too right?
  ASSERT_EQ(2, library_->rowCount(QModelIndex()));

  // Remove the song
  backend_->DeleteSongs(SongList() << one);

  // Everything should be gone - even the artist header
  ASSERT_EQ(0, library_->rowCount(QModelIndex()));
}

} // namespace
