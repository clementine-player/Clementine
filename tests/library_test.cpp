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

#include "library.h"
#include "backgroundthread.h"
#include "mock_backgroundthread.h"
#include "mock_librarybackend.h"

#include <QtDebug>
#include <QThread>
#include <QSignalSpy>
#include <QSortFilterProxyModel>

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

namespace {

class LibraryTest : public ::testing::Test {
 protected:
  void SetUp() {
    library_ = new StrictMock<Library>(
        static_cast<EngineBase*>(NULL), static_cast<QObject*>(NULL));
    library_->set_backend_factory(
        new FakeBackgroundThreadFactory<LibraryBackendInterface, MockLibraryBackend>);
    library_->set_watcher_factory(
        new FakeBackgroundThreadFactory<LibraryWatcher, LibraryWatcher>);

    library_->Init();

    backend_ = static_cast<MockLibraryBackend*>(library_->GetBackend().get());

    library_sorted_ = new QSortFilterProxyModel;
    library_sorted_->setSourceModel(library_);
    library_sorted_->setSortRole(Library::Role_SortText);
    library_sorted_->setDynamicSortFilter(true);
    library_sorted_->sort(0);
  }

  void TearDown() {
    EXPECT_CALL(*backend_, Die());
    delete library_;
    delete library_sorted_;
  }

  Library* library_;
  MockLibraryBackend* backend_;
  QSortFilterProxyModel* library_sorted_;
};

TEST_F(LibraryTest, Initialisation) {
  backend_->ExpectSetup();
  library_->StartThreads();

  EXPECT_EQ(0, library_->rowCount(QModelIndex()));
}

TEST_F(LibraryTest, WithInitialCompilations) {
  backend_->ExpectSetup(true);
  library_->StartThreads();

  ASSERT_EQ(1, library_->rowCount(QModelIndex()));

  QModelIndex va_index = library_->index(0, 0, QModelIndex());
  EXPECT_EQ("Various Artists", va_index.data().toString());
  EXPECT_TRUE(library_->hasChildren(va_index));
}

TEST_F(LibraryTest, WithInitialArtists) {
  backend_->ExpectSetup(false, QStringList() << "Artist 1" << "Artist 2" << "Foo");
  library_->StartThreads();

  ASSERT_EQ(5, library_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("a", library_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 1", library_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 2", library_sorted_->index(2, 0, QModelIndex()).data().toString());
  EXPECT_EQ("f", library_sorted_->index(3, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Foo", library_sorted_->index(4, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryTest, CompilationAlbums) {
  backend_->ExpectSetup(true);
  library_->StartThreads();

  QModelIndex va_index = library_->index(0, 0, QModelIndex());

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("Artist", "Album", "", "");
  EXPECT_CALL(*backend_, GetCompilationAlbums(_))
      .WillOnce(Return(albums));

  ASSERT_EQ(library_->rowCount(va_index), 1);
  QModelIndex album_index = library_->index(0, 0, va_index);
  EXPECT_EQ(library_->data(album_index).toString(), "Album");
  EXPECT_TRUE(library_->hasChildren(album_index));
}

TEST_F(LibraryTest, NumericHeaders) {
  backend_->ExpectSetup(false, QStringList() << "1artist" << "2artist" << "0artist" << "zartist");
  library_->StartThreads();

  ASSERT_EQ(6, library_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("0-9", library_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("0artist", library_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("1artist", library_sorted_->index(2, 0, QModelIndex()).data().toString());
  EXPECT_EQ("2artist", library_sorted_->index(3, 0, QModelIndex()).data().toString());
  EXPECT_EQ("z", library_sorted_->index(4, 0, QModelIndex()).data().toString());
  EXPECT_EQ("zartist", library_sorted_->index(5, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryTest, MixedCaseHeaders) {
  backend_->ExpectSetup(false, QStringList() << "Artist" << "artist");
  library_->StartThreads();

  ASSERT_EQ(3, library_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("a", library_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist", library_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("artist", library_sorted_->index(2, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryTest, UnknownArtists) {
  backend_->ExpectSetup(false, QStringList() << "");
  library_->StartThreads();

  ASSERT_EQ(1, library_->rowCount(QModelIndex()));
  QModelIndex unknown_index = library_->index(0, 0, QModelIndex());
  EXPECT_EQ("Unknown", unknown_index.data().toString());

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("", "Album", "", "");
  EXPECT_CALL(*backend_, GetAlbumsByArtist(QString(""), _))
      .WillOnce(Return(albums));

  ASSERT_EQ(1, library_->rowCount(unknown_index));
  EXPECT_EQ("Album", library_->index(0, 0, unknown_index).data().toString());
}

TEST_F(LibraryTest, UnknownAlbums) {
  backend_->ExpectSetup(false, QStringList() << "Artist");
  library_->StartThreads();

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("Artist", "", "", "");
  albums << LibraryBackendInterface::Album("Artist", "Album", "", "");
  EXPECT_CALL(*backend_, GetAlbumsByArtist(QString("Artist"), _))
      .WillOnce(Return(albums));

  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(2, library_->rowCount(artist_index));

  QModelIndex unknown_album_index = library_->index(0, 0, artist_index);
  QModelIndex real_album_index = library_->index(1, 0, artist_index);

  EXPECT_EQ("Unknown", unknown_album_index.data().toString());
  EXPECT_EQ("Album", real_album_index.data().toString());
}

TEST_F(LibraryTest, VariousArtistSongs) {
  backend_->ExpectSetup(true);
  library_->StartThreads();

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("", "Album", "", "");

  SongList songs;
  songs << Song() << Song() << Song() << Song();
  songs[0].Init("Title 1", "Artist 1", "Album", 0);
  songs[1].Init("Title 2", "Artist 2", "Album", 0);
  songs[2].Init("Title 3", "Artist 3", "Album", 0);
  songs[3].Init("Title 4", "Various Artists", "Album", 0);

  // Different ways of putting songs in "Various Artist".  Make sure they all work
  songs[0].set_sampler(true);
  songs[1].set_compilation(true);
  songs[2].set_forced_compilation_on(true);
  songs[3].set_sampler(true);

  EXPECT_CALL(*backend_, GetCompilationAlbums(_))
      .WillOnce(Return(albums));
  EXPECT_CALL(*backend_, GetCompilationSongs(QString("Album"), _))
      .WillOnce(Return(songs));

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
  backend_->ExpectSetup(false, QStringList() << "Artist");
  library_->StartThreads();

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("Artist", "Album", "", "");

  SongList songs;
  songs << Song() << Song() << Song();
  songs[0].Init("Title 1", "Artist", "Album", 0); songs[0].set_id(0);
  songs[1].Init("Title 2", "Artist", "Album", 0); songs[1].set_id(1);
  songs[2].Init("Title 3", "Artist", "Album", 0); songs[2].set_id(2);

  EXPECT_CALL(*backend_, GetAlbumsByArtist(QString("Artist"), _))
      .WillOnce(Return(albums));
  EXPECT_CALL(*backend_, GetSongs(QString("Artist"), QString("Album"), _))
      .WillOnce(Return(songs));

  // Lazy load the items
  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, library_->rowCount(artist_index));
  QModelIndex album_index = library_->index(0, 0, artist_index);
  ASSERT_EQ(3, library_->rowCount(album_index));

  // Remove the first two songs
  QSignalSpy spy1(library_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy spy2(library_, SIGNAL(rowsRemoved(QModelIndex,int,int)));

  SongList songs_to_delete = SongList() << songs[0] << songs[1];
  backend_->EmitSongsDeleted(songs_to_delete);

  ASSERT_EQ(2, spy1.count());
  ASSERT_EQ(2, spy2.count());
  for (int call=0 ; call<=1 ; ++call) {
    for (int arg=1 ; arg<=2 ; ++arg) {
      EXPECT_EQ(0, spy1[call][arg].toInt()) << "Call " << call << " arg " << arg;
      EXPECT_EQ(0, spy2[call][arg].toInt()) << "Call " << call << " arg " << arg;
    }
  }

  ASSERT_EQ(1, library_->rowCount(album_index));
  EXPECT_EQ("Title 3", library_->index(0, 0, album_index).data().toString());
}

TEST_F(LibraryTest, RemoveSongsNotLazyLoaded) {
  backend_->ExpectSetup(false, QStringList() << "Artist");
  library_->StartThreads();

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("Artist", "Album", "", "");

  SongList songs;
  songs << Song() << Song() << Song();
  songs[0].Init("Title 1", "Artist", "Album", 0); songs[0].set_id(0);
  songs[1].Init("Title 2", "Artist", "Album", 0); songs[1].set_id(1);

  EXPECT_CALL(*backend_, GetAlbumsByArtist(QString("Artist"), _))
      .WillOnce(Return(albums));

  // Remove the first two songs
  QSignalSpy spy1(library_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy spy2(library_, SIGNAL(rowsRemoved(QModelIndex,int,int)));

  backend_->EmitSongsDeleted(songs);

  ASSERT_EQ(0, spy1.count());
  ASSERT_EQ(0, spy2.count());
}

TEST_F(LibraryTest, RemoveEmptyAlbums) {
  backend_->ExpectSetup(false, QStringList() << "Artist");
  library_->StartThreads();

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("Artist", "Album 1", "", "");
  albums << LibraryBackendInterface::Album("Artist", "Album 2", "", "");

  SongList songs_one; songs_one << Song();
  SongList songs_two; songs_two << Song() << Song();
  songs_one[0].Init("Title 1", "Artist", "Album 1", 0); songs_one[0].set_id(0);
  songs_two[0].Init("Title 2", "Artist", "Album 2", 0); songs_two[0].set_id(1);
  songs_two[1].Init("Title 3", "Artist", "Album 2", 0); songs_two[1].set_id(2);

  // Lazy load the album
  EXPECT_CALL(*backend_, GetAlbumsByArtist(QString("Artist"), _))
      .WillOnce(Return(albums));

  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(2, library_->rowCount(artist_index));

  // Remove one song from each album
  SongList songs_to_delete;
  songs_to_delete << songs_one.takeFirst() << songs_two.takeFirst();

  EXPECT_CALL(*backend_, GetSongs(QString("Artist"), QString("Album 1"), _))
      .WillOnce(Return(songs_one));
  EXPECT_CALL(*backend_, GetSongs(QString("Artist"), QString("Album 2"), _))
      .WillOnce(Return(songs_two));

  backend_->EmitSongsDeleted(songs_to_delete);

  // Check the model
  ASSERT_EQ(1, library_->rowCount(artist_index));
  QModelIndex album_index = library_->index(0, 0, artist_index);
  EXPECT_EQ("Album 2", album_index.data().toString());

  ASSERT_EQ(1, library_->rowCount(album_index));
  EXPECT_EQ("Title 3", library_->index(0, 0, album_index).data().toString());
}

TEST_F(LibraryTest, RemoveEmptyArtists) {
  backend_->ExpectSetup(false, QStringList() << "Artist");
  library_->StartThreads();

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("Artist", "Album", "", "");

  SongList songs = SongList() << Song();
  songs[0].Init("Title 1", "Artist", "Album", 0); songs[0].set_id(0);

  EXPECT_CALL(*backend_, GetAlbumsByArtist(QString("Artist"), _))
      .WillOnce(Return(albums));
  EXPECT_CALL(*backend_, GetSongs(QString("Artist"), QString("Album"), _))
      .WillOnce(Return(songs));

  // Lazy load the items
  QModelIndex artist_index = library_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, library_->rowCount(artist_index));
  QModelIndex album_index = library_->index(0, 0, artist_index);
  ASSERT_EQ(1, library_->rowCount(album_index));

  // The artist header is there too right?
  ASSERT_EQ(2, library_->rowCount(QModelIndex()));

  // Remove the song
  backend_->EmitSongsDeleted(songs);

  // Everything should be gone - even the artist header
  ASSERT_EQ(0, library_->rowCount(QModelIndex()));
}


} // namespace
