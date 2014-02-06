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

#include "core/database.h"
#include "library/librarymodel.h"
#include "library/librarybackend.h"
#include "library/library.h"

#include <QtDebug>
#include <QThread>
#include <QSignalSpy>
#include <QSortFilterProxyModel>

namespace {

class LibraryModelTest : public ::testing::Test {
 protected:
  void SetUp() {
    database_.reset(new MemoryDatabase);
    backend_.reset(new LibraryBackend);
    backend_->Init(database_, Library::kSongsTable,
                   Library::kDirsTable, Library::kSubdirsTable, Library::kFtsTable);
    model_.reset(new LibraryModel(backend_.get(), NULL));

    added_dir_ = false;

    model_sorted_.reset(new QSortFilterProxyModel);
    model_sorted_->setSourceModel(model_.get());
    model_sorted_->setSortRole(LibraryModel::Role_SortText);
    model_sorted_->setDynamicSortFilter(true);
    model_sorted_->sort(0);
  }

  Song AddSong(Song& song) {
    song.set_directory_id(1);
    if (song.mtime() == -1) song.set_mtime(1);
    if (song.ctime() == -1) song.set_ctime(1);
    if (song.url().isEmpty()) song.set_url(QUrl("file:///tmp/foo"));
    if (song.filesize() == -1) song.set_filesize(1);

    if (!added_dir_) {
      backend_->AddDirectory("/tmp");
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

  std::shared_ptr<Database> database_;
  std::unique_ptr<LibraryBackend> backend_;
  std::unique_ptr<LibraryModel> model_;
  std::unique_ptr<QSortFilterProxyModel> model_sorted_;

  bool added_dir_;
};

TEST_F(LibraryModelTest, Initialisation) {
  EXPECT_EQ(0, model_->rowCount(QModelIndex()));
}

TEST_F(LibraryModelTest, WithInitialArtists) {
  AddSong("Title", "Artist 1", "Album", 123);
  AddSong("Title", "Artist 2", "Album", 123);
  AddSong("Title", "Foo", "Album", 123);
  model_->Init(false);

  ASSERT_EQ(5, model_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("A", model_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 1", model_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 2", model_sorted_->index(2, 0, QModelIndex()).data().toString());
  EXPECT_EQ("F", model_sorted_->index(3, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Foo", model_sorted_->index(4, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryModelTest, CompilationAlbums) {
  Song song;
  song.Init("Title", "Artist", "Album", 123);
  song.set_compilation(true);

  AddSong(song);
  model_->Init(false);
  model_->fetchMore(model_->index(0, 0));

  ASSERT_EQ(1, model_->rowCount(QModelIndex()));

  QModelIndex va_index = model_->index(0, 0, QModelIndex());
  EXPECT_EQ("Various artists", va_index.data().toString());
  EXPECT_TRUE(model_->hasChildren(va_index));
  ASSERT_EQ(model_->rowCount(va_index), 1);

  QModelIndex album_index = model_->index(0, 0, va_index);
  EXPECT_EQ(model_->data(album_index).toString(), "Album");
  EXPECT_TRUE(model_->hasChildren(album_index));
}

TEST_F(LibraryModelTest, NumericHeaders) {
  AddSong("Title", "1artist", "Album", 123);
  AddSong("Title", "2artist", "Album", 123);
  AddSong("Title", "0artist", "Album", 123);
  AddSong("Title", "zartist", "Album", 123);
  model_->Init(false);

  ASSERT_EQ(6, model_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("0-9", model_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("0artist", model_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("1artist", model_sorted_->index(2, 0, QModelIndex()).data().toString());
  EXPECT_EQ("2artist", model_sorted_->index(3, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Z", model_sorted_->index(4, 0, QModelIndex()).data().toString());
  EXPECT_EQ("zartist", model_sorted_->index(5, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryModelTest, MixedCaseHeaders) {
  AddSong("Title", "Artist", "Album", 123);
  AddSong("Title", "artist", "Album", 123);
  model_->Init(false);

  ASSERT_EQ(3, model_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("A", model_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist", model_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("artist", model_sorted_->index(2, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryModelTest, UnknownArtists) {
  AddSong("Title", "", "Album", 123);
  model_->Init(false);
  model_->fetchMore(model_->index(0, 0));

  ASSERT_EQ(1, model_->rowCount(QModelIndex()));
  QModelIndex unknown_index = model_->index(0, 0, QModelIndex());
  EXPECT_EQ("Unknown", unknown_index.data().toString());

  ASSERT_EQ(1, model_->rowCount(unknown_index));
  EXPECT_EQ("Album", model_->index(0, 0, unknown_index).data().toString());
}

TEST_F(LibraryModelTest, UnknownAlbums) {
  AddSong("Title", "Artist", "", 123);
  AddSong("Title", "Artist", "Album", 123);
  model_->Init(false);
  model_->fetchMore(model_->index(0, 0));

  QModelIndex artist_index = model_->index(0, 0, QModelIndex());
  ASSERT_EQ(2, model_->rowCount(artist_index));

  QModelIndex unknown_album_index = model_->index(0, 0, artist_index);
  QModelIndex real_album_index = model_->index(1, 0, artist_index);

  EXPECT_EQ("Unknown", unknown_album_index.data().toString());
  EXPECT_EQ("Album", real_album_index.data().toString());
}

TEST_F(LibraryModelTest, VariousArtistSongs) {
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
  model_->Init(false);

  QModelIndex artist_index = model_->index(0, 0, QModelIndex());
  model_->fetchMore(artist_index);
  ASSERT_EQ(1, model_->rowCount(artist_index));

  QModelIndex album_index = model_->index(0, 0, artist_index);
  model_->fetchMore(album_index);
  ASSERT_EQ(4, model_->rowCount(album_index));

  EXPECT_EQ("Artist 1 - Title 1", model_->index(0, 0, album_index).data().toString());
  EXPECT_EQ("Artist 2 - Title 2", model_->index(1, 0, album_index).data().toString());
  EXPECT_EQ("Artist 3 - Title 3", model_->index(2, 0, album_index).data().toString());
  EXPECT_EQ("Title 4", model_->index(3, 0, album_index).data().toString());
}

TEST_F(LibraryModelTest, RemoveSongsLazyLoaded) {
  Song one = AddSong("Title 1", "Artist", "Album", 123); one.set_id(1);
  Song two = AddSong("Title 2", "Artist", "Album", 123); two.set_id(2);
  AddSong("Title 3", "Artist", "Album", 123);
  model_->Init(false);

  // Lazy load the items
  QModelIndex artist_index = model_->index(0, 0, QModelIndex());
  model_->fetchMore(artist_index);
  ASSERT_EQ(1, model_->rowCount(artist_index));
  QModelIndex album_index = model_->index(0, 0, artist_index);
  model_->fetchMore(album_index);
  ASSERT_EQ(3, model_->rowCount(album_index));

  // Remove the first two songs
  QSignalSpy spy_preremove(model_.get(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy spy_remove(model_.get(), SIGNAL(rowsRemoved(QModelIndex,int,int)));
  QSignalSpy spy_reset(model_.get(), SIGNAL(modelReset()));

  backend_->DeleteSongs(SongList() << one << two);

  ASSERT_EQ(2, spy_preremove.count());
  ASSERT_EQ(2, spy_remove.count());
  ASSERT_EQ(0, spy_reset.count());

  artist_index = model_->index(0, 0, QModelIndex());
  ASSERT_EQ(1, model_->rowCount(artist_index));
  album_index = model_->index(0, 0, artist_index);
  ASSERT_EQ(1, model_->rowCount(album_index));
  EXPECT_EQ("Title 3", model_->index(0, 0, album_index).data().toString());
}

TEST_F(LibraryModelTest, RemoveSongsNotLazyLoaded) {
  Song one = AddSong("Title 1", "Artist", "Album", 123); one.set_id(1);
  Song two = AddSong("Title 2", "Artist", "Album", 123); two.set_id(2);
  model_->Init(false);

  // Remove the first two songs
  QSignalSpy spy_preremove(model_.get(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
  QSignalSpy spy_remove(model_.get(), SIGNAL(rowsRemoved(QModelIndex,int,int)));
  QSignalSpy spy_reset(model_.get(), SIGNAL(modelReset()));

  backend_->DeleteSongs(SongList() << one << two);

  ASSERT_EQ(0, spy_preremove.count());
  ASSERT_EQ(0, spy_remove.count());
  ASSERT_EQ(1, spy_reset.count());
}

TEST_F(LibraryModelTest, RemoveEmptyAlbums) {
  Song one = AddSong("Title 1", "Artist", "Album 1", 123); one.set_id(1);
  Song two = AddSong("Title 2", "Artist", "Album 2", 123); two.set_id(2);
  Song three = AddSong("Title 3", "Artist", "Album 2", 123); three.set_id(3);
  model_->Init(false);

  QModelIndex artist_index = model_->index(0, 0, QModelIndex());
  model_->fetchMore(artist_index);
  ASSERT_EQ(2, model_->rowCount(artist_index));

  // Remove one song from each album
  backend_->DeleteSongs(SongList() << one << two);

  // Check the model
  artist_index = model_->index(0, 0, QModelIndex());
  model_->fetchMore(artist_index);
  ASSERT_EQ(1, model_->rowCount(artist_index));
  QModelIndex album_index = model_->index(0, 0, artist_index);
  model_->fetchMore(album_index);
  EXPECT_EQ("Album 2", album_index.data().toString());

  ASSERT_EQ(1, model_->rowCount(album_index));
  EXPECT_EQ("Title 3", model_->index(0, 0, album_index).data().toString());
}

TEST_F(LibraryModelTest, RemoveEmptyArtists) {
  Song one = AddSong("Title", "Artist", "Album", 123); one.set_id(1);
  model_->Init(false);

  // Lazy load the items
  QModelIndex artist_index = model_->index(0, 0, QModelIndex());
  model_->fetchMore(artist_index);
  ASSERT_EQ(1, model_->rowCount(artist_index));
  QModelIndex album_index = model_->index(0, 0, artist_index);
  model_->fetchMore(album_index);
  ASSERT_EQ(1, model_->rowCount(album_index));

  // The artist header is there too right?
  ASSERT_EQ(2, model_->rowCount(QModelIndex()));

  // Remove the song
  backend_->DeleteSongs(SongList() << one);

  // Everything should be gone - even the artist header
  ASSERT_EQ(0, model_->rowCount(QModelIndex()));
}

} // namespace
