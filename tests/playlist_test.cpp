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

#include "library/libraryplaylistitem.h"
#include "playlist/playlist.h"
#include "mock_settingsprovider.h"
#include "mock_playlistitem.h"

#include <QtDebug>
#include <QUndoStack>

using std::shared_ptr;
using ::testing::Return;

namespace {

class PlaylistTest : public ::testing::Test {
 protected:
  PlaylistTest()
    : playlist_(nullptr, nullptr, nullptr, 1),
      sequence_(nullptr, new DummySettingsProvider)
  {
  }

  void SetUp() {
    playlist_.set_sequence(&sequence_);
  }

  MockPlaylistItem* MakeMockItem(const QString& title,
                                 const QString& artist = QString(),
                                 const QString& album = QString(),
                                 int length = 123) const {
    Song metadata;
    metadata.Init(title, artist, album, length);

    MockPlaylistItem* ret = new MockPlaylistItem;
    EXPECT_CALL(*ret, Metadata())
        .WillRepeatedly(Return(metadata));

    return ret;
  }

  shared_ptr<PlaylistItem> MakeMockItemP(
      const QString& title, const QString& artist = QString(),
      const QString& album = QString(), int length = 123) const {
    return shared_ptr<PlaylistItem>(MakeMockItem(title, artist, album, length));
  }

  Playlist playlist_;
  PlaylistSequence sequence_;
};

TEST_F(PlaylistTest, Basic) {
  EXPECT_EQ(0, playlist_.rowCount(QModelIndex()));
}

TEST_F(PlaylistTest, InsertItems) {
  MockPlaylistItem* item = MakeMockItem("Title", "Artist", "Album", 123);
  shared_ptr<PlaylistItem> item_ptr(item);

  // Insert the item
  EXPECT_EQ(0, playlist_.rowCount(QModelIndex()));
  playlist_.InsertItems(PlaylistItemList() << item_ptr, -1);
  ASSERT_EQ(1, playlist_.rowCount(QModelIndex()));

  // Get the metadata
  EXPECT_EQ("Title", playlist_.data(playlist_.index(0, Playlist::Column_Title)));
  EXPECT_EQ("Artist", playlist_.data(playlist_.index(0, Playlist::Column_Artist)));
  EXPECT_EQ("Album", playlist_.data(playlist_.index(0, Playlist::Column_Album)));
  EXPECT_EQ(123, playlist_.data(playlist_.index(0, Playlist::Column_Length)));
}

TEST_F(PlaylistTest, Indexes) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  // Start "playing" track 1
  playlist_.set_current_row(0);
  EXPECT_EQ(0, playlist_.current_row());
  EXPECT_EQ("One", playlist_.current_item()->Metadata().title());
  EXPECT_EQ(-1, playlist_.previous_row());
  EXPECT_EQ(1, playlist_.next_row());

  // Stop playing
  EXPECT_EQ(0, playlist_.last_played_row());
  playlist_.set_current_row(-1);
  EXPECT_EQ(0, playlist_.last_played_row());
  EXPECT_EQ(-1, playlist_.current_row());

  // Play track 2
  playlist_.set_current_row(1);
  EXPECT_EQ(1, playlist_.current_row());
  EXPECT_EQ("Two", playlist_.current_item()->Metadata().title());
  EXPECT_EQ(0, playlist_.previous_row());
  EXPECT_EQ(2, playlist_.next_row());

  // Play track 3
  playlist_.set_current_row(2);
  EXPECT_EQ(2, playlist_.current_row());
  EXPECT_EQ("Three", playlist_.current_item()->Metadata().title());
  EXPECT_EQ(1, playlist_.previous_row());
  EXPECT_EQ(-1, playlist_.next_row());
}

TEST_F(PlaylistTest, RepeatPlaylist) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  playlist_.sequence()->SetRepeatMode(PlaylistSequence::Repeat_Playlist);

  playlist_.set_current_row(0);
  EXPECT_EQ(1, playlist_.next_row());

  playlist_.set_current_row(1);
  EXPECT_EQ(2, playlist_.next_row());

  playlist_.set_current_row(2);
  EXPECT_EQ(0, playlist_.next_row());
}

TEST_F(PlaylistTest, RepeatTrack) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  playlist_.sequence()->SetRepeatMode(PlaylistSequence::Repeat_Track);

  playlist_.set_current_row(0);
  EXPECT_EQ(0, playlist_.next_row());
}

TEST_F(PlaylistTest, RepeatAlbum) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One", "Album one")
      << MakeMockItemP("Two", "Album two")
      << MakeMockItemP("Three", "Album one"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  playlist_.sequence()->SetRepeatMode(PlaylistSequence::Repeat_Album);

  playlist_.set_current_row(0);
  EXPECT_EQ(2, playlist_.next_row());

  playlist_.set_current_row(2);
  EXPECT_EQ(0, playlist_.next_row());
}

TEST_F(PlaylistTest, RemoveBeforeCurrent) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  // Remove a row before the currently playing track
  playlist_.set_current_row(2);
  EXPECT_EQ(2, playlist_.current_row());
  playlist_.removeRow(1, QModelIndex());
  EXPECT_EQ(1, playlist_.current_row());
  EXPECT_EQ(1, playlist_.last_played_row());
  EXPECT_EQ(0, playlist_.previous_row());
  EXPECT_EQ(-1, playlist_.next_row());
}

TEST_F(PlaylistTest, RemoveAfterCurrent) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  // Remove a row after the currently playing track
  playlist_.set_current_row(0);
  EXPECT_EQ(0, playlist_.current_row());
  playlist_.removeRow(1, QModelIndex());
  EXPECT_EQ(0, playlist_.current_row());
  EXPECT_EQ(0, playlist_.last_played_row());
  EXPECT_EQ(-1, playlist_.previous_row());
  EXPECT_EQ(1, playlist_.next_row());

  playlist_.set_current_row(1);
  EXPECT_EQ(-1, playlist_.next_row());
}

TEST_F(PlaylistTest, RemoveCurrent) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  // Remove the currently playing track's row
  playlist_.set_current_row(1);
  EXPECT_EQ(1, playlist_.current_row());
  playlist_.removeRow(1, QModelIndex());
  EXPECT_EQ(-1, playlist_.current_row());
  EXPECT_EQ(-1, playlist_.last_played_row());
  EXPECT_EQ(-1, playlist_.previous_row());
  EXPECT_EQ(0, playlist_.next_row());
}

TEST_F(PlaylistTest, InsertBeforeCurrent) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  playlist_.set_current_row(1);
  EXPECT_EQ(1, playlist_.current_row());
  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("Four"), 0);
  ASSERT_EQ(4, playlist_.rowCount(QModelIndex()));

  EXPECT_EQ(2, playlist_.current_row());
  EXPECT_EQ(2, playlist_.last_played_row());
  EXPECT_EQ(1, playlist_.previous_row());
  EXPECT_EQ(3, playlist_.next_row());

  EXPECT_EQ("Four", playlist_.data(playlist_.index(0, Playlist::Column_Title)));
  EXPECT_EQ("One", playlist_.data(playlist_.index(1, Playlist::Column_Title)));
}

TEST_F(PlaylistTest, InsertAfterCurrent) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  playlist_.set_current_row(1);
  EXPECT_EQ(1, playlist_.current_row());
  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("Four"), 2);
  ASSERT_EQ(4, playlist_.rowCount(QModelIndex()));

  EXPECT_EQ(1, playlist_.current_row());
  EXPECT_EQ(1, playlist_.last_played_row());
  EXPECT_EQ(0, playlist_.previous_row());
  EXPECT_EQ(2, playlist_.next_row());

  EXPECT_EQ("Two", playlist_.data(playlist_.index(1, Playlist::Column_Title)));
  EXPECT_EQ("Four", playlist_.data(playlist_.index(2, Playlist::Column_Title)));
  EXPECT_EQ("Three", playlist_.data(playlist_.index(3, Playlist::Column_Title)));
}

TEST_F(PlaylistTest, Clear) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  playlist_.set_current_row(1);
  EXPECT_EQ(1, playlist_.current_row());
  playlist_.Clear();

  EXPECT_EQ(0, playlist_.rowCount(QModelIndex()));
  EXPECT_EQ(-1, playlist_.current_row());
  EXPECT_EQ(-1, playlist_.last_played_row());
  EXPECT_EQ(-1, playlist_.previous_row());
  EXPECT_EQ(-1, playlist_.next_row());
}

TEST_F(PlaylistTest, UndoAdd) {
  EXPECT_FALSE(playlist_.undo_stack()->canUndo());
  EXPECT_FALSE(playlist_.undo_stack()->canRedo());

  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("Title"));
  EXPECT_EQ(1, playlist_.rowCount(QModelIndex()));
  EXPECT_FALSE(playlist_.undo_stack()->canRedo());
  ASSERT_TRUE(playlist_.undo_stack()->canUndo());

  playlist_.undo_stack()->undo();
  EXPECT_EQ(0, playlist_.rowCount(QModelIndex()));
  EXPECT_FALSE(playlist_.undo_stack()->canUndo());
  ASSERT_TRUE(playlist_.undo_stack()->canRedo());

  playlist_.undo_stack()->redo();
  EXPECT_EQ(1, playlist_.rowCount(QModelIndex()));
  EXPECT_FALSE(playlist_.undo_stack()->canRedo());
  EXPECT_TRUE(playlist_.undo_stack()->canUndo());

  EXPECT_EQ("Title", playlist_.data(playlist_.index(0, Playlist::Column_Title)));
}

TEST_F(PlaylistTest, UndoMultiAdd) {
  // Add 1 item
  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("One"));

  // Add 2 items
  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("Two") << MakeMockItemP("Three"));

  // Undo adding 2 items
  ASSERT_TRUE(playlist_.undo_stack()->canUndo());
  EXPECT_EQ("add 2 songs", playlist_.undo_stack()->undoText());
  playlist_.undo_stack()->undo();

  // Undo adding 1 item
  ASSERT_TRUE(playlist_.undo_stack()->canUndo());
  EXPECT_EQ("add 1 songs", playlist_.undo_stack()->undoText());
  playlist_.undo_stack()->undo();

  EXPECT_FALSE(playlist_.undo_stack()->canUndo());
}

TEST_F(PlaylistTest, UndoRemove) {
  EXPECT_FALSE(playlist_.undo_stack()->canUndo());
  EXPECT_FALSE(playlist_.undo_stack()->canRedo());

  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("Title"));
  playlist_.removeRow(0);

  EXPECT_EQ(0, playlist_.rowCount(QModelIndex()));
  EXPECT_FALSE(playlist_.undo_stack()->canRedo());
  ASSERT_TRUE(playlist_.undo_stack()->canUndo());

  playlist_.undo_stack()->undo();
  EXPECT_EQ(1, playlist_.rowCount(QModelIndex()));
  ASSERT_TRUE(playlist_.undo_stack()->canRedo());

  EXPECT_EQ("Title", playlist_.data(playlist_.index(0, Playlist::Column_Title)));

  playlist_.undo_stack()->redo();
  EXPECT_EQ(0, playlist_.rowCount(QModelIndex()));
  EXPECT_FALSE(playlist_.undo_stack()->canRedo());
  EXPECT_TRUE(playlist_.undo_stack()->canUndo());
}

TEST_F(PlaylistTest, UndoMultiRemove) {
  // Add 3 items
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  // Remove 1 item
  playlist_.removeRow(1); // Item "Two"

  // Remove 2 items
  playlist_.removeRows(0, 2); // "One" and "Three"

  ASSERT_EQ(0, playlist_.rowCount(QModelIndex()));

  // Undo removing all 3 items
  ASSERT_TRUE(playlist_.undo_stack()->canUndo());
  EXPECT_EQ("remove 3 songs", playlist_.undo_stack()->undoText());
  playlist_.undo_stack()->undo();
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));
}

TEST_F(PlaylistTest, UndoClear) {
  playlist_.InsertItems(PlaylistItemList()
      << MakeMockItemP("One") << MakeMockItemP("Two") << MakeMockItemP("Three"));
  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));

  playlist_.Clear();
  ASSERT_EQ(0, playlist_.rowCount(QModelIndex()));
  ASSERT_TRUE(playlist_.undo_stack()->canUndo());
  EXPECT_EQ("remove 3 songs", playlist_.undo_stack()->undoText());
  playlist_.undo_stack()->undo();

  ASSERT_EQ(3, playlist_.rowCount(QModelIndex()));
}

TEST_F(PlaylistTest, UndoRemoveCurrent) {
  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("Title"));
  playlist_.set_current_row(0);
  EXPECT_EQ(0, playlist_.current_row());
  EXPECT_EQ(0, playlist_.last_played_row());

  playlist_.removeRow(0);
  EXPECT_EQ(-1, playlist_.current_row());
  EXPECT_EQ(-1, playlist_.last_played_row());

  playlist_.undo_stack()->undo();
  EXPECT_EQ(0, playlist_.current_row());
  EXPECT_EQ(0, playlist_.last_played_row());
}

TEST_F(PlaylistTest, UndoRemoveOldCurrent) {
  playlist_.InsertItems(PlaylistItemList() << MakeMockItemP("Title"));
  playlist_.set_current_row(0);
  EXPECT_EQ(0, playlist_.current_row());
  EXPECT_EQ(0, playlist_.last_played_row());

  playlist_.removeRow(0);
  EXPECT_EQ(-1, playlist_.current_row());
  EXPECT_EQ(-1, playlist_.last_played_row());

  playlist_.set_current_row(-1);

  playlist_.undo_stack()->undo();
  EXPECT_EQ(0, playlist_.current_row());
  EXPECT_EQ(0, playlist_.last_played_row());
}

TEST_F(PlaylistTest, ShuffleThenNext) {
  // Add 100 items
  PlaylistItemList items;
  for (int i=0 ; i<100 ; ++i)
    items << MakeMockItemP("Item " + QString::number(i));
  playlist_.InsertItems(items);

  playlist_.set_current_row(0);

  // Shuffle until the current index is not at the end
  forever {
    playlist_.Shuffle();
    if (playlist_.current_row() != items.count()-1)
      break;
  }

  int index = playlist_.current_row();
  EXPECT_EQ("Item 0", playlist_.current_item()->Metadata().title());
  EXPECT_EQ("Item 0", playlist_.data(playlist_.index(index, Playlist::Column_Title)));
  EXPECT_EQ(index, playlist_.last_played_row());
  EXPECT_EQ(index + 1, playlist_.next_row());

  // Shuffle until the current index *is* at the end
  forever {
    playlist_.Shuffle();
    if (playlist_.current_row() == items.count()-1)
      break;
  }

  index = playlist_.current_row();
  EXPECT_EQ("Item 0", playlist_.current_item()->Metadata().title());
  EXPECT_EQ("Item 0", playlist_.data(playlist_.index(index, Playlist::Column_Title)));
  EXPECT_EQ(index, playlist_.last_played_row());
  EXPECT_EQ(-1, playlist_.next_row());
  EXPECT_EQ(index-1, playlist_.previous_row());
}

TEST_F(PlaylistTest, LibraryIdMapSingle) {
  Song song;
  song.Init("title", "artist", "album", 123);
  song.set_id(1);

  PlaylistItemPtr item(new LibraryPlaylistItem(song));
  playlist_.InsertItems(PlaylistItemList() << item);

  EXPECT_EQ(0, playlist_.library_items_by_id(-1).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(0).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(2).count());
  ASSERT_EQ(1, playlist_.library_items_by_id(1).count());
  EXPECT_EQ(song.title(), playlist_.library_items_by_id(1)[0]->Metadata().title());

  playlist_.Clear();

  EXPECT_EQ(0, playlist_.library_items_by_id(1).count());
}

TEST_F(PlaylistTest, LibraryIdMapInvalid) {
  Song invalid;
  invalid.Init("title", "artist", "album", 123);
  ASSERT_EQ(-1, invalid.id());

  PlaylistItemPtr item(new LibraryPlaylistItem(invalid));
  playlist_.InsertItems(PlaylistItemList() << item);

  EXPECT_EQ(0, playlist_.library_items_by_id(-1).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(0).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(1).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(2).count());
}

TEST_F(PlaylistTest, LibraryIdMapMulti) {
  Song one;
  one.Init("title", "artist", "album", 123);
  one.set_id(1);

  Song two;
  two.Init("title 2", "artist 2", "album 2", 123);
  two.set_id(2);

  PlaylistItemPtr item_one(new LibraryPlaylistItem(one));
  PlaylistItemPtr item_two(new LibraryPlaylistItem(two));
  PlaylistItemPtr item_three(new LibraryPlaylistItem(one));
  playlist_.InsertItems(PlaylistItemList() << item_one << item_two << item_three);

  EXPECT_EQ(2, playlist_.library_items_by_id(1).count());
  EXPECT_EQ(1, playlist_.library_items_by_id(2).count());

  playlist_.removeRow(1); // item_two
  EXPECT_EQ(2, playlist_.library_items_by_id(1).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(2).count());

  playlist_.removeRow(1); // item_three
  EXPECT_EQ(1, playlist_.library_items_by_id(1).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(2).count());

  playlist_.removeRow(0); // item_one
  EXPECT_EQ(0, playlist_.library_items_by_id(1).count());
  EXPECT_EQ(0, playlist_.library_items_by_id(2).count());
}


} // namespace
