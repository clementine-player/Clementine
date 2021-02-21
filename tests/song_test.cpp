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
#include "tagreader.h"
#include "core/song.h"
#ifdef HAVE_LIBLASTFM
#include "internet/lastfm/lastfmcompat.h"
#endif

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils.h"

#include <QStringList>
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

  static Song ReadSongFromFile(const QString& filename) {
    TagReader tag_reader;
    Song song;
    ::cpb::tagreader::SongMetadata pb_song;

    // We need to init protobuf object from a Song object, to have default
    // values initialized correctly. For example, Song's rating is -1 by
    // default: using protobuf directly would lead to 0 by default, which is not
    // what we want.
    song.ToProtobuf(&pb_song);
    tag_reader.ReadFile(filename, &pb_song);
    song.InitFromProtobuf(pb_song);
    return song;
  }

  static void WriteSongToFile(const Song& song, const QString& filename) {
    TagReader tag_reader;
    ::cpb::tagreader::SongMetadata pb_song;
    song.ToProtobuf(&pb_song);
    tag_reader.SaveFile(filename, pb_song);
  }

  static void WriteSongStatisticsToFile(const Song& song,
                                        const QString& filename) {
    TagReader tag_reader;
    ::cpb::tagreader::SongMetadata pb_song;
    song.ToProtobuf(&pb_song);
    tag_reader.SaveSongStatisticsToFile(filename, pb_song);
  }

  static void WriteSongRatingToFile(const Song& song, const QString& filename) {
    TagReader tag_reader;
    ::cpb::tagreader::SongMetadata pb_song;
    song.ToProtobuf(&pb_song);
    tag_reader.SaveSongRatingToFile(filename, pb_song);
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
#endif  // HAVE_LIBLASTFM

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

TEST_F(SongTest, FMPSRating) {
  TemporaryResource r(":/testdata/fmpsrating.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.42, song.rating());
}

TEST_F(SongTest, FMPSRatingUser) {
  TemporaryResource r(":/testdata/fmpsratinguser.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.10, song.rating());

  song.set_rating(0.20);
  WriteSongRatingToFile(song, r.fileName());
  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.20, new_song.rating());
}

TEST_F(SongTest, FMPSRatingBoth) {
  TemporaryResource r(":/testdata/fmpsratingboth.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.42, song.rating());
}

TEST_F(SongTest, FMPSPlayCount) {
  TemporaryResource r(":/testdata/fmpsplaycount.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(123, song.playcount());

  song.set_playcount(69);
  WriteSongStatisticsToFile(song, r.fileName());
  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(69, new_song.playcount());
}

TEST_F(SongTest, FMPSPlayCountUser) {
  TemporaryResource r(":/testdata/fmpsplaycountuser.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(42, song.playcount());
}

TEST_F(SongTest, FMPSPlayCountBoth) {
  TemporaryResource r(":/testdata/fmpsplaycountboth.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(123, song.playcount());
}

TEST_F(SongTest, FMPSUnrated) {
  QStringList files_to_test;
  files_to_test << ":/testdata/beep.m4a"
                << ":/testdata/beep.mp3"
                << ":/testdata/beep.flac"
                << ":/testdata/beep.ogg"
                << ":/testdata/beep.spx"
                << ":/testdata/beep.wav"
                << ":/testdata/beep.wma";
  for (const QString& test_filename : files_to_test) {
    TemporaryResource r(test_filename);
    Song song = ReadSongFromFile(r.fileName());
    // beep files don't contain rating info, so they should be considered as
    // "unrated" i.e. rating == -1
    EXPECT_EQ(-1, song.rating());
    // Writing -1 i.e. "unrated" to a file shouldn't write anything
    WriteSongRatingToFile(song, r.fileName());

    // Compare files
    QFile orig_file(test_filename);
    orig_file.open(QIODevice::ReadOnly);
    QByteArray orig_file_data = orig_file.readAll();
    QFile temp_file(r.fileName());
    temp_file.open(QIODevice::ReadOnly);
    QByteArray temp_file_data = temp_file.readAll();
    EXPECT_TRUE(!orig_file_data.isEmpty());
    EXPECT_TRUE(!temp_file_data.isEmpty());
    EXPECT_TRUE(orig_file_data == temp_file_data);
  }
}

TEST_F(SongTest, FMPSScore) {
  TemporaryResource r(":/testdata/beep.mp3");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_score(87);

    WriteSongStatisticsToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(87, new_song.score());
}

TEST_F(SongTest, POPMRating) {
  TemporaryResource r(":/testdata/popmrating.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.60, song.rating());
}

TEST_F(SongTest, BothFMPSPOPMRating) {
  // fmpspopmrating.mp3 contains FMPS with rating 0.42 and POPM with 0x80
  // (corresponds to 0.60 rating for us): check that FMPS tag has precedence
  TemporaryResource r(":/testdata/fmpspopmrating.mp3");
  Song song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.42, song.rating());
}

TEST_F(SongTest, RatingOgg) {
  TemporaryResource r(":/testdata/beep.ogg");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_rating(0.20);
    WriteSongRatingToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.20, new_song.rating());
}

TEST_F(SongTest, StatisticsOgg) {
  TemporaryResource r(":/testdata/beep.ogg");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_playcount(1337);
    song.set_score(87);

    WriteSongStatisticsToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(1337, new_song.playcount());
  EXPECT_EQ(87, new_song.score());
}

TEST_F(SongTest, TagsOgg) {
  TemporaryResource r(":/testdata/beep.ogg");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_title("beep title");
    song.set_artist("beep artist");
    song.set_album("beep album");
    song.set_albumartist("beep album artist");
    song.set_composer("beep composer");
    song.set_performer("beep performer");
    song.set_grouping("beep grouping");
    song.set_genre("beep genre");
    song.set_comment("beep comment");
    song.set_track(12);
    song.set_disc(1234);
    song.set_year(2015);

    WriteSongToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ("beep title", new_song.title());
  EXPECT_EQ("beep artist", new_song.artist());
  EXPECT_EQ("beep album", new_song.album());
  EXPECT_EQ("beep album artist", new_song.albumartist());
  EXPECT_EQ("beep composer", new_song.composer());
  EXPECT_EQ("beep performer", new_song.performer());
  EXPECT_EQ("beep grouping", new_song.grouping());
  EXPECT_EQ("beep genre", new_song.genre());
  EXPECT_EQ("beep comment", new_song.comment());
  EXPECT_EQ(12, new_song.track());
  EXPECT_EQ(1234, new_song.disc());
  EXPECT_EQ(2015, new_song.year());
}

TEST_F(SongTest, RatingFLAC) {
  TemporaryResource r(":/testdata/beep.flac");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_rating(0.20);
    WriteSongRatingToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.20, new_song.rating());
}

TEST_F(SongTest, StatisticsFLAC) {
  TemporaryResource r(":/testdata/beep.flac");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_playcount(1337);
    song.set_score(87);

    WriteSongStatisticsToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(1337, new_song.playcount());
  EXPECT_EQ(87, new_song.score());
}

TEST_F(SongTest, TagsFLAC) {
  TemporaryResource r(":/testdata/beep.flac");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_title("beep title");
    song.set_artist("beep artist");
    song.set_album("beep album");
    song.set_albumartist("beep album artist");
    song.set_composer("beep composer");
    song.set_performer("beep performer");
    song.set_grouping("beep grouping");
    song.set_genre("beep genre");
    song.set_comment("beep comment");
    song.set_track(12);
    song.set_disc(1234);
    song.set_year(2015);

    WriteSongToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ("beep title", new_song.title());
  EXPECT_EQ("beep artist", new_song.artist());
  EXPECT_EQ("beep album", new_song.album());
  EXPECT_EQ("beep album artist", new_song.albumartist());
  EXPECT_EQ("beep composer", new_song.composer());
  EXPECT_EQ("beep performer", new_song.performer());
  EXPECT_EQ("beep grouping", new_song.grouping());
  EXPECT_EQ("beep genre", new_song.genre());
  EXPECT_EQ("beep comment", new_song.comment());
  EXPECT_EQ(12, new_song.track());
  EXPECT_EQ(1234, new_song.disc());
  EXPECT_EQ(2015, new_song.year());
}

#ifdef TAGLIB_WITH_ASF
TEST_F(SongTest, RatingASF) {
  TemporaryResource r(":/testdata/beep.wma");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_rating(0.20);

    WriteSongRatingToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.20, new_song.rating());
}

TEST_F(SongTest, StatisticsASF) {
  TemporaryResource r(":/testdata/beep.wma");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_playcount(1337);
    song.set_score(87);

    WriteSongStatisticsToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(1337, new_song.playcount());
  EXPECT_EQ(87, new_song.score());
}
#endif  // TAGLIB_WITH_ASF

TEST_F(SongTest, RatingMP4) {
  TemporaryResource r(":/testdata/beep.m4a");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_rating(0.20);

    WriteSongRatingToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_FLOAT_EQ(0.20, new_song.rating());
}

TEST_F(SongTest, StatisticsMP4) {
  TemporaryResource r(":/testdata/beep.m4a");
  {
    Song song = ReadSongFromFile(r.fileName());
    song.set_playcount(1337);
    song.set_score(87);

    WriteSongStatisticsToFile(song, r.fileName());
  }

  Song new_song = ReadSongFromFile(r.fileName());
  EXPECT_EQ(1337, new_song.playcount());
  EXPECT_EQ(87, new_song.score());
}

TEST_F(SongTest, MergeUserSetDataTest) {
  // Suppose we have songs from files and from the DB
  // Songs from files are the ones that will be imported in the DB after being merged with the
  // former DB song values
  Song song_db_with_rating;
  Song song_db_with_no_rating;
  Song song_file_with_rating;
  Song song_file_with_no_rating;

  song_db_with_rating.set_rating(0.42);
  song_file_with_rating.set_rating(0.43);

  // Merging a DB song with no rating should not update the rating that is in the file song
  float old_rating_value = song_file_with_rating.rating();
  song_file_with_rating.MergeUserSetData(song_db_with_no_rating);
  EXPECT_NE(song_db_with_no_rating.rating(), song_file_with_rating.rating());
  EXPECT_EQ(song_file_with_rating.rating(), old_rating_value);

  // Merging a DB song with rating should not update the rating that is in the file song...
  old_rating_value = song_file_with_rating.rating();
  song_file_with_rating.MergeUserSetData(song_db_with_rating);
  EXPECT_NE(song_db_with_rating.rating(), song_file_with_rating.rating());
  EXPECT_EQ(song_file_with_rating.rating(), old_rating_value);

  // ...but DB song's rating shouldn't be erased if the file song has no rating
  song_file_with_no_rating.MergeUserSetData(song_db_with_rating);
  EXPECT_EQ(song_file_with_no_rating.rating(), song_db_with_rating.rating());
}

}  // namespace
