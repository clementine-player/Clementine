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

#include "gtest/gtest.h"
#include "test_utils.h"

#include "core/organiseformat.h"
#include "core/timeconstants.h"

#include <QUrl>

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
  song_.set_length_nanosec(987 * kNsecPerSec);
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
  song_.set_url(QUrl("file:///some/path/filename.mp3"));

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

TEST_F(OrganiseFormatTest, ReplaceSpaces) {
  song_.set_title("The Song Title");
  format_.set_format("The Format String %title");

  EXPECT_EQ("The Format String The Song Title", format_.GetFilenameForSong(song_));
  format_.set_replace_spaces(true);
  EXPECT_EQ("The_Format_String_The_Song_Title", format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, ReplaceThe) {
  song_.set_title("The Title");
  song_.set_artist("The Artist");
  format_.set_format("%artist %title");

  EXPECT_EQ("The Artist The Title", format_.GetFilenameForSong(song_));
  format_.set_replace_the(true);
  EXPECT_EQ("Artist The Title", format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, ReplaceNonAscii) {
  song_.set_artist(QString::fromUtf8("Röyksopp"));
  format_.set_format("%artist");

  EXPECT_EQ(QString::fromUtf8("Röyksopp"), format_.GetFilenameForSong(song_));
  format_.set_replace_non_ascii(true);
  EXPECT_EQ("Royksopp", format_.GetFilenameForSong(song_));

  song_.set_artist(QString::fromUtf8("Владимир Высоцкий"));
  EXPECT_EQ("________ ________", format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, TrackNumberPadding) {
  format_.set_format("%track");

  song_.set_track(9);
  EXPECT_EQ("09", format_.GetFilenameForSong(song_));

  song_.set_track(99);
  EXPECT_EQ("99", format_.GetFilenameForSong(song_));

  song_.set_track(999);
  EXPECT_EQ("999", format_.GetFilenameForSong(song_));

  song_.set_track(0);
  EXPECT_EQ("", format_.GetFilenameForSong(song_));
}

TEST_F(OrganiseFormatTest, ReplaceSlashes) {
  format_.set_format("%title");
  song_.set_title("foo/bar\\baz");
  EXPECT_EQ("foo_bar_baz", format_.GetFilenameForSong(song_));
}
