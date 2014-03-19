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

#include "core/timeconstants.h"
#include "playlistparsers/cueparser.h"

#include <QUrl>

class CueParserTest : public ::testing::Test {
 protected:
  CueParserTest()
      : parser_(nullptr) {
  }

  // We believe CUE - all songs with proper CUE entries should be valid.
  bool validate_songs(SongList songs) {
    foreach(const Song& song, songs) {
      if(!song.is_valid()) {
        return false;
      }
    }

    return true;
  }

  qlonglong to_nanosec(int minutes, int seconds, int frames) {
    return (minutes * 60 * 75 + seconds * 75 + frames) * kNsecPerSec / 75;
  }

  CueParser parser_;
};

TEST_F(CueParserTest, ParsesASong) {
  QFile file(":testdata/onesong.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, "CUEPATH", QDir(""));

  // one song
  ASSERT_EQ(1, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  ASSERT_EQ("Un soffio caldo", first_song.title());
  ASSERT_EQ("Zucchero", first_song.artist());
  ASSERT_EQ("Zucchero himself", first_song.albumartist());
  ASSERT_EQ("", first_song.album());
  ASSERT_EQ(to_nanosec(0, 1, 15), first_song.beginning_nanosec());
  ASSERT_EQ(1, first_song.track());
  ASSERT_EQ("CUEPATH", first_song.cue_path());

  validate_songs(song_list);
}

TEST_F(CueParserTest, ParsesTwoSongs) {
  QFile file(":testdata/twosongs.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, "", QDir(""));

  // two songs
  ASSERT_EQ(2, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  Song second_song = song_list.at(1);

  ASSERT_EQ("Un soffio caldo", first_song.title());
  ASSERT_EQ("Chocabeck", first_song.album());
  ASSERT_EQ("Zucchero himself", first_song.artist());
  ASSERT_EQ("Zucchero himself", first_song.albumartist());
  ASSERT_EQ(to_nanosec(0, 1, 0), first_song.beginning_nanosec());
  ASSERT_EQ(second_song.beginning_nanosec() - first_song.beginning_nanosec(), first_song.length_nanosec());
  ASSERT_EQ(1, first_song.track());

  ASSERT_EQ("Somewon Else's Tears", second_song.title());
  ASSERT_EQ("Chocabeck", second_song.album());
  ASSERT_EQ("Zucchero himself", second_song.artist());
  ASSERT_EQ("Zucchero himself", second_song.albumartist());
  ASSERT_EQ(to_nanosec(5, 3, 68), second_song.beginning_nanosec());
  ASSERT_EQ(2, second_song.track());

  validate_songs(song_list);
}

TEST_F(CueParserTest, SkipsBrokenSongs) {
  QFile file(":testdata/brokensong.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, "", QDir(""));

  // two songs (the broken one is not in the list)
  ASSERT_EQ(2, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  Song second_song = song_list.at(1);

  ASSERT_EQ("Un soffio caldo", first_song.title());
  ASSERT_EQ("Chocabeck", first_song.album());
  ASSERT_EQ("Zucchero himself", first_song.artist());
  ASSERT_EQ("Zucchero himself", first_song.albumartist());
  ASSERT_EQ(to_nanosec(0, 1, 0), first_song.beginning_nanosec());
  // includes the broken song too; this entry will span from it's 
  // INDEX (beginning) to the end of the next correct song
  ASSERT_EQ(second_song.beginning_nanosec() - first_song.beginning_nanosec(), first_song.length_nanosec());
  ASSERT_EQ(1, first_song.track());

  ASSERT_EQ("Somewon Else's Tears", second_song.title());
  ASSERT_EQ("Chocabeck", second_song.album());
  ASSERT_EQ("Zucchero himself", second_song.artist());
  ASSERT_EQ("Zucchero himself", second_song.albumartist());
  ASSERT_EQ(to_nanosec(5, 0, 0), second_song.beginning_nanosec());
  ASSERT_EQ(2, second_song.track());

  validate_songs(song_list);
}

TEST_F(CueParserTest, UsesAllMetadataInformation) {
  QFile file(":testdata/fullmetadata.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, "", QDir(""));

  // two songs
  ASSERT_EQ(2, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  Song second_song = song_list.at(1);

  ASSERT_TRUE(first_song.url().toString().endsWith("a_file.mp3"));
  ASSERT_EQ("Un soffio caldo", first_song.title());
  ASSERT_EQ("Album", first_song.album());
  ASSERT_EQ("Zucchero", first_song.artist());
  ASSERT_EQ("Zucchero himself", first_song.albumartist());
  ASSERT_EQ("Some guy", first_song.composer());
  ASSERT_EQ(to_nanosec(0, 1, 0), first_song.beginning_nanosec());
  ASSERT_EQ(second_song.beginning_nanosec() - first_song.beginning_nanosec(), first_song.length_nanosec());
  ASSERT_EQ(1, first_song.track());

  ASSERT_TRUE(second_song.url().toString().endsWith("a_file.mp3"));
  ASSERT_EQ("Hey you!", second_song.title());
  ASSERT_EQ("Album", second_song.album());
  ASSERT_EQ("Zucchero himself", second_song.artist());
  ASSERT_EQ("Zucchero himself", second_song.albumartist());
  ASSERT_EQ("Some other guy", second_song.composer());
  ASSERT_EQ(to_nanosec(0, 2, 0), second_song.beginning_nanosec());
  ASSERT_EQ(2, second_song.track());

  validate_songs(song_list);
}

TEST_F(CueParserTest, AcceptsMultipleFileBasedCues) {
  QFile file(":testdata/manyfiles.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, "CUEPATH", QDir(""));

  // five songs
  ASSERT_EQ(5, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  Song second_song = song_list.at(1);
  Song third_song = song_list.at(2);
  Song fourth_song = song_list.at(3);
  Song fifth_song = song_list.at(4);

  ASSERT_TRUE(first_song.url().toString().endsWith("files/longer_one.mp3"));
  ASSERT_EQ("A1Song1", first_song.title());
  ASSERT_EQ("Artist One Album", first_song.album());
  ASSERT_EQ("Artist One", first_song.artist());
  ASSERT_EQ("Artist One", first_song.albumartist());
  ASSERT_EQ(to_nanosec(0, 1, 0), first_song.beginning_nanosec());
  ASSERT_EQ(second_song.beginning_nanosec() - first_song.beginning_nanosec(), first_song.length_nanosec());
  ASSERT_EQ(-1, first_song.track());
  ASSERT_EQ("CUEPATH", first_song.cue_path());

  ASSERT_TRUE(second_song.url().toString().endsWith("files/longer_one.mp3"));
  ASSERT_EQ("A1Song2", second_song.title());
  ASSERT_EQ("Artist One Album", second_song.album());
  ASSERT_EQ("Artist One", second_song.artist());
  ASSERT_EQ("Artist One", second_song.albumartist());
  ASSERT_EQ(to_nanosec(5, 3, 68), second_song.beginning_nanosec());
  ASSERT_EQ(-1, second_song.track());

  ASSERT_TRUE(third_song.url().toString().endsWith("files/longer_two_p1.mp3"));
  ASSERT_EQ("A2P1Song1", third_song.title());
  ASSERT_EQ("Artist Two Album", third_song.album());
  ASSERT_EQ("Artist X", third_song.artist());
  ASSERT_EQ("Artist Two", third_song.albumartist());
  ASSERT_EQ(to_nanosec(0, 0, 12), third_song.beginning_nanosec());
  ASSERT_EQ(fourth_song.beginning_nanosec() - third_song.beginning_nanosec(), third_song.length_nanosec());
  ASSERT_EQ(-1, third_song.track());
  ASSERT_EQ("CUEPATH", third_song.cue_path());

  ASSERT_TRUE(fourth_song.url().toString().endsWith("files/longer_two_p1.mp3"));
  ASSERT_EQ("A2P1Song2", fourth_song.title());
  ASSERT_EQ("Artist Two Album", fourth_song.album());
  ASSERT_EQ("Artist Two", fourth_song.artist());
  ASSERT_EQ("Artist Two", fourth_song.albumartist());
  ASSERT_EQ(to_nanosec(4, 0, 13), fourth_song.beginning_nanosec());
  ASSERT_EQ(-1, fourth_song.track());

  ASSERT_TRUE(fifth_song.url().toString().endsWith("files/longer_two_p2.mp3"));
  ASSERT_EQ("A2P2Song1", fifth_song.title());
  ASSERT_EQ("Artist Two Album", fifth_song.album());
  ASSERT_EQ("Artist Two", fifth_song.artist());
  ASSERT_EQ("Artist Two", fifth_song.albumartist());
  ASSERT_EQ(to_nanosec(0, 1, 0), fifth_song.beginning_nanosec());
  ASSERT_EQ(-1, fifth_song.track());
  ASSERT_EQ("CUEPATH", fifth_song.cue_path());

  validate_songs(song_list);
}

TEST_F(CueParserTest, SkipsBrokenSongsInMultipleFileBasedCues) {
  QFile file(":testdata/manyfilesbroken.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, "", QDir(""));

  // four songs
  ASSERT_EQ(4, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  Song second_song = song_list.at(1);
  Song third_song = song_list.at(2);
  Song fourth_song = song_list.at(3);

  // A* - broken song in the middle
  ASSERT_TRUE(first_song.url().toString().endsWith("file1.mp3"));
  ASSERT_EQ("Artist One", first_song.artist());
  ASSERT_EQ("Artist One Album", first_song.album());
  ASSERT_EQ("A1", first_song.title());
  ASSERT_EQ(to_nanosec(0, 1, 15), first_song.beginning_nanosec());
  ASSERT_EQ(second_song.beginning_nanosec() - first_song.beginning_nanosec(), first_song.length_nanosec());
  ASSERT_EQ(-1, first_song.track());

  ASSERT_TRUE(second_song.url().toString().endsWith("file1.mp3"));
  ASSERT_EQ("Artist One", second_song.artist());
  ASSERT_EQ("Artist One Album", second_song.album());
  ASSERT_EQ("A3", second_song.title());
  ASSERT_EQ(to_nanosec(1, 0, 16), second_song.beginning_nanosec());
  ASSERT_EQ(-1, second_song.track());

  // all B* songs are broken

  // C* - broken song at the end
  ASSERT_TRUE(third_song.url().toString().endsWith("file3.mp3"));
  ASSERT_EQ("Artist Three", third_song.artist());
  ASSERT_EQ("Artist Three Album", third_song.album());
  ASSERT_EQ("C1", third_song.title());
  ASSERT_EQ(to_nanosec(0, 1, 0), third_song.beginning_nanosec());
  ASSERT_EQ(-1, third_song.track());

  // D* - broken song at the beginning
  ASSERT_TRUE(fourth_song.url().toString().endsWith("file4.mp3"));
  ASSERT_EQ("Artist Four", fourth_song.artist());
  ASSERT_EQ("Artist Four Album", fourth_song.album());
  ASSERT_EQ("D2", fourth_song.title());
  ASSERT_EQ(to_nanosec(15, 55, 66), fourth_song.beginning_nanosec());
  ASSERT_EQ(-1, fourth_song.track());

  validate_songs(song_list);
}

TEST_F(CueParserTest, SkipsDataFiles) {
  QFile file(":testdata/withdatafiles.cue");
  file.open(QIODevice::ReadOnly);

  SongList song_list = parser_.Load(&file, "", QDir(""));

  // two songs
  ASSERT_EQ(2, song_list.size());

  // with the specified metadata
  Song first_song = song_list.at(0);
  Song second_song = song_list.at(1);

  ASSERT_TRUE(first_song.url().toString().endsWith("file1.mp3"));
  ASSERT_EQ("Artist One", first_song.artist());
  ASSERT_EQ("Artist One Album", first_song.album());
  ASSERT_EQ("A1", first_song.title());
  ASSERT_EQ(to_nanosec(0, 1, 0), first_song.beginning_nanosec());
  ASSERT_EQ(-1, first_song.track());

  ASSERT_TRUE(second_song.url().toString().endsWith("file4.mp3"));
  ASSERT_EQ("Artist Four", second_song.artist());
  ASSERT_EQ("Artist Four Album", second_song.album());
  ASSERT_EQ("D1", second_song.title());
  ASSERT_EQ(to_nanosec(1, 1, 0), second_song.beginning_nanosec());
  ASSERT_EQ(-1, second_song.track());

  validate_songs(song_list);
}
