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

#include "gtest/gtest.h"
#include "test_utils.h"

#include "ui/organiseformat.h"

class OrganiseFormatTest : public ::testing::Test {
protected:
  OrganiseFormat format_;
  Song song_;
};


TEST_F(OrganiseFormatTest, BasicReplace) {
  song_.set_album("album");
  song_.set_albumartist("albumartist");
  song_.set_artist("artist");
  song_.set_bitrate(123);
  song_.set_bpm(4.56);
  song_.set_comment("comment");
  song_.set_composer("composer");
  song_.set_disc(789);
  song_.set_genre("genre");
  song_.set_length(987);
  song_.set_samplerate(654);
  song_.set_title("title");
  song_.set_track(321);
  song_.set_year(2010);

  format_.set_format("%album %albumartist %bitrate %bpm %comment %composer "
                     "%disc %genre %length %samplerate %title %track %year");

  ASSERT_TRUE(format_.IsValid());
  EXPECT_EQ("album albumartist 123 4.56 comment composer 789 genre 987 654 title 321 2010",
            format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, Extension) {
  song_.set_filename("/some/path/filename.mp3");

  format_.set_format("%extension");
  ASSERT_TRUE(format_.IsValid());
  EXPECT_EQ("mp3", format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, ArtistInitial) {
  song_.set_artist("bob");

  format_.set_format("%artistinitial");
  ASSERT_TRUE(format_.IsValid());
  EXPECT_EQ("B", format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, AlbumArtistInitial) {
  song_.set_albumartist("bob");

  format_.set_format("%artistinitial");
  ASSERT_TRUE(format_.IsValid());
  EXPECT_EQ("B", format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, InvalidTag) {
  format_.set_format("%invalid");
  EXPECT_FALSE(format_.IsValid());
}

TEST_F(OrganiseFormatTest, Blocks) {
  format_.set_format("Before{Inside%year}After");
  ASSERT_TRUE(format_.IsValid());

  song_.set_year(-1);
  EXPECT_EQ("BeforeAfter", format_.GetFilenameForSong(song_));

  song_.set_year(0);
  EXPECT_EQ("BeforeAfter", format_.GetFilenameForSong(song_));

  song_.set_year(123);
  EXPECT_EQ("BeforeInside123After", format_.GetFilenameForSong(song_));
}
