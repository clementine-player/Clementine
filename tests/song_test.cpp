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

#include "config.h"
#include "core/song.h"
#ifdef HAVE_LIBLASTFM
  #include "internet/lastfmcompat.h"
#endif

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils.h"

#include <QTemporaryFile>
#include <QTextCodec>

#include <id3v2tag.h>

namespace {

class SongTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    // Return something from uninteresting mock functions.
    testing::DefaultValue<TagLib::String>::Set("foobarbaz");
  }
};


#ifdef HAVE_LIBLASTFM
TEST_F(SongTest, InitsFromLastFM) {
  Song song;
  lastfm::MutableTrack track;
  track.setTitle("Foo");
  lastfm::Artist artist("Bar");
  track.setArtist(artist);
  lastfm::Album album(artist, "Baz");
  track.setAlbum(album);

  song.InitFromLastFM(track);
  EXPECT_EQ("Foo", song.title());
  EXPECT_EQ("Baz", song.album());
  EXPECT_EQ("Bar", song.artist());
}
#endif // HAVE_LIBLASTFM

/*TEST_F(SongTest, InitsFromFile) {
  QTemporaryFile temp;
  temp.open();
  mock_factory_.ExpectCall(temp.fileName(), "Foo", "Bar", "Baz");
  Song song(&mock_factory_);
  song.InitFromFile(temp.fileName(), 42);
  EXPECT_EQ("Foo", song.title());
  EXPECT_EQ("Bar", song.artist());
  EXPECT_EQ("Baz", song.album());
}*/

/*TEST_F(SongTest, FMPSRating) {
  TemporaryResource r(":/testdata/fmpsrating.mp3");
  Song song;
  song.InitFromFile(r.fileName(), -1);
  EXPECT_FLOAT_EQ(0.42, song.rating());
}

TEST_F(SongTest, FMPSRatingUser) {
  TemporaryResource r(":/testdata/fmpsratinguser.mp3");
  Song song;
  song.InitFromFile(r.fileName(), -1);
  EXPECT_FLOAT_EQ(0.10, song.rating());
}

TEST_F(SongTest, FMPSRatingBoth) {
  TemporaryResource r(":/testdata/fmpsratingboth.mp3");
  Song song;
  song.InitFromFile(r.fileName(), -1);
  EXPECT_FLOAT_EQ(0.42, song.rating());
}

TEST_F(SongTest, FMPSPlayCount) {
  TemporaryResource r(":/testdata/fmpsplaycount.mp3");
  Song song;
  song.InitFromFile(r.fileName(), -1);
  EXPECT_EQ(123, song.playcount());
}

TEST_F(SongTest, FMPSPlayCountUser) {
  TemporaryResource r(":/testdata/fmpsplaycountuser.mp3");
  Song song;
  song.InitFromFile(r.fileName(), -1);
  EXPECT_EQ(42, song.playcount());
}

TEST_F(SongTest, FMPSPlayCountBoth) {
  TemporaryResource r(":/testdata/fmpsplaycountboth.mp3");
  Song song;
  song.InitFromFile(r.fileName(), -1);
  EXPECT_EQ(123, song.playcount());
}*/

}  // namespace
