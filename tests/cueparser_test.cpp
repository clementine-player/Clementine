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


#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "test_utils.h"
#include "mock_taglib.h"

#include "playlistparsers/cueparser.h"

class CueParserTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    testing::DefaultValue<TagLib::String>::Set("foobarbaz");
  }

  CueParserTest()
      : parser_(NULL) {
  }

  CueParser parser_;
  MockFileRefFactory taglib_;
};

TEST_F(CueParserTest, ParsesASong) {
  QFile file(":testdata/onesong.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, QDir(""));

  // one song
  ASSERT_EQ(1, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  ASSERT_EQ("Un soffio caldo", first_song.title());
  ASSERT_EQ("Zucchero", first_song.artist());
  ASSERT_EQ("Zucchero himself", first_song.albumartist());
  ASSERT_EQ("", first_song.album());
  ASSERT_EQ(1, first_song.beginning());
}

TEST_F(CueParserTest, ParsesTwoSongs) {
  QFile file(":testdata/twosongs.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, QDir(""));

  // two songs
  ASSERT_EQ(2, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  ASSERT_EQ("Un soffio caldo", first_song.title());
  ASSERT_EQ("Chocabeck", first_song.album());
  ASSERT_EQ("Zucchero himself", first_song.artist());
  ASSERT_EQ("Zucchero himself", first_song.albumartist());
  ASSERT_EQ(1, first_song.beginning());
  ASSERT_EQ((5 * 60 + 3) - 1, first_song.length());

  Song second_song = song_list.at(1);
  ASSERT_EQ("Somewon Else's Tears", second_song.title());
  ASSERT_EQ("Chocabeck", second_song.album());
  ASSERT_EQ("Zucchero himself", second_song.artist());
  ASSERT_EQ("Zucchero himself", second_song.albumartist());
  ASSERT_EQ(5 * 60 + 3, second_song.beginning());
}

TEST_F(CueParserTest, SkipsBrokenSongs) {
  QFile file(":testdata/brokensong.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, QDir(""));

  // two songs (the broken one is not in the list)
  ASSERT_EQ(2, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  ASSERT_EQ("Un soffio caldo", first_song.title());
  ASSERT_EQ("Chocabeck", first_song.album());
  ASSERT_EQ("Zucchero himself", first_song.artist());
  ASSERT_EQ("Zucchero himself", first_song.albumartist());
  ASSERT_EQ(1, first_song.beginning());
  // includes the broken song too; this entry will span from it's 
  // INDEX (beginning) to the end of the next correct song
  ASSERT_EQ((5 * 60) - 1, first_song.length());

  Song second_song = song_list.at(1);
  ASSERT_EQ("Somewon Else's Tears", second_song.title());
  ASSERT_EQ("Chocabeck", second_song.album());
  ASSERT_EQ("Zucchero himself", second_song.artist());
  ASSERT_EQ("Zucchero himself", second_song.albumartist());
  ASSERT_EQ(5 * 60, second_song.beginning());
}
