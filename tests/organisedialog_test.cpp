/* This file is part of Clementine.
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#include "core/song.h"
#include "core/organiseformat.h"
#include "ui/organisedialog.h"


TEST(OrganiseDialogTest, ComputeNewSongsFilenamesTest) {
  // Create some songs, with multiple similar songs
  SongList songs;
  {
    Song song;
    song.set_title("Test1");
    song.set_album("Album");
    songs << song;
  }
  // Empty song
  {
    Song song;
    song.set_basefilename("filename.mp3");
    songs << song;
  }
  // Without extension
  for (int i = 0; i < 2; i++) {
    Song song;
    song.set_title("Test2");
    song.set_url(QUrl("file://test" + QString::number(i)));
    songs << song;
  }

  // With file extension
  for (int i = 0; i < 3; i++) {
    Song song;
    song.set_artist("Foo");
    song.set_title("Bar");
    song.set_url(QUrl("file://foobar" + QString::number(i) + ".mp3"));
    songs << song;
  }

  // Generate new filenames
  OrganiseFormat format;
  format.set_format(OrganiseDialog::kDefaultFormat);
  Organise::NewSongInfoList new_songs_info = OrganiseDialog::ComputeNewSongsFilenames(songs, format);

  EXPECT_EQ("/Album/Test1.", new_songs_info[0].new_filename_);
  EXPECT_EQ("//filename.mp3", new_songs_info[1].new_filename_);
  EXPECT_EQ("//Test2.", new_songs_info[2].new_filename_);
  EXPECT_EQ("//Test2(2).", new_songs_info[3].new_filename_);
  EXPECT_EQ("Foo//Bar.mp3", new_songs_info[4].new_filename_);
  EXPECT_EQ("Foo//Bar(2).mp3", new_songs_info[5].new_filename_);
  EXPECT_EQ("Foo//Bar(3).mp3", new_songs_info[6].new_filename_);
}

