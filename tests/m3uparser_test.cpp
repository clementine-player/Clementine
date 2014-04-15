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
#include "playlistparsers/m3uparser.h"

#include <QBuffer>
#include <QTemporaryFile>

using ::testing::HasSubstr;

class M3UParserTest : public ::testing::Test {
 protected:
  M3UParserTest()
      : parser_(nullptr) {
  }

  M3UParser parser_;
};

TEST_F(M3UParserTest, ParsesMetadata) {
  QString line("#EXTINF:123,Foo artist - Foo track");
  M3UParser::Metadata metadata;
  ASSERT_TRUE(parser_.ParseMetadata(line, &metadata));
  EXPECT_EQ("Foo artist", metadata.artist.toStdString());
  EXPECT_EQ("Foo track", metadata.title.toStdString());
  EXPECT_EQ(123 * kNsecPerSec, metadata.length);
}

TEST_F(M3UParserTest, ParsesTrackLocation) {
  QTemporaryFile temp;
  temp.open();
  taglib_.ExpectCall(temp.fileName(), "foo", "bar", "baz");
  Song song(&taglib_);
  QString line(temp.fileName());
  parser_.LoadSong(line, 0, QDir(), &song);
  ASSERT_EQ(QUrl::fromLocalFile(temp.fileName()), song.url());

  song.InitFromFile(song.url().toLocalFile(), -1);

  EXPECT_EQ("foo", song.title());
  EXPECT_EQ("bar", song.artist());
  EXPECT_EQ("baz", song.album());
}

TEST_F(M3UParserTest, ParsesTrackLocationRelative) {
  QTemporaryFile temp;
  temp.open();
  QFileInfo info(temp);
  taglib_.ExpectCall(temp.fileName(), "foo", "bar", "baz");
  M3UParser parser(nullptr);
  QString line(info.fileName());
  Song song(&taglib_);
  parser.LoadSong(line, 0, info.dir(), &song);
  ASSERT_EQ(QUrl::fromLocalFile(temp.fileName()), song.url());

  song.InitFromFile(song.url().toLocalFile(), -1);

  EXPECT_EQ("foo", song.title());
}

TEST_F(M3UParserTest, ParsesTrackLocationHttp) {
  QString line("http://example.com/foo/bar.mp3");
  Song song;
  parser_.LoadSong(line, 0, QDir(), &song);
  EXPECT_EQ(QUrl("http://example.com/foo/bar.mp3"), song.url());
}

TEST_F(M3UParserTest, ParsesSongsFromDevice) {
  QByteArray data = "#EXTM3U\n"
                    "#EXTINF:123,Some Artist - Some Title\n"
                    "http://foo.com/bar/somefile.mp3\n";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  M3UParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.size());
  Song s = songs[0];
  EXPECT_EQ("Some Artist", s.artist());
  EXPECT_EQ("Some Title", s.title());
  EXPECT_EQ(123 * kNsecPerSec, s.length_nanosec());
  EXPECT_EQ(QUrl("http://foo.com/bar/somefile.mp3"), s.url());
}

TEST_F(M3UParserTest, ParsesNonExtendedM3U) {
  QByteArray data = "http://foo.com/bar/somefile.mp3\n"
                    "http://baz.com/thing.mp3\n";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  M3UParser parser(nullptr);
  SongList songs = parser.Load(&buffer, "", QDir("somedir"));
  ASSERT_EQ(2, songs.size());
  EXPECT_EQ(QUrl("http://foo.com/bar/somefile.mp3"), songs[0].url());
  EXPECT_EQ(QUrl("http://baz.com/thing.mp3"), songs[1].url());
  EXPECT_EQ(-1, songs[0].length_nanosec());
  EXPECT_EQ(-1, songs[1].length_nanosec());
  EXPECT_TRUE(songs[0].artist().isEmpty());
}

TEST_F(M3UParserTest, ParsesActualM3U) {
  QFile file(":testdata/test.m3u");
  file.open(QIODevice::ReadOnly);
  M3UParser parser(nullptr);
  SongList songs = parser.Load(&file, "", QDir("somedir"));
  ASSERT_EQ(239, songs.size());
  EXPECT_EQ("gravity", songs[0].title());
  EXPECT_EQ(203 * kNsecPerSec, songs[0].length_nanosec());
  EXPECT_EQ(QString::fromUtf8("ほっぴンちょっぴン"), songs.back().title());
  EXPECT_EQ(85 * kNsecPerSec, songs.back().length_nanosec());
}

TEST_F(M3UParserTest, SavesSong) {
  QByteArray data;
  QBuffer buffer(&data);
  buffer.open(QIODevice::WriteOnly);
  Song one;
  one.set_filetype(Song::Type_Stream);
  one.set_title("foo");
  one.set_artist("bar");
  one.set_length_nanosec(123 * kNsecPerSec);
  one.set_url(QUrl("http://www.example.com/foo.mp3"));
  SongList songs;
  songs << one;
  M3UParser parser(nullptr);
  parser.Save(songs, &buffer);
  EXPECT_THAT(data.constData(), HasSubstr("#EXTM3U"));
  EXPECT_THAT(data.constData(), HasSubstr("#EXTINF:123,bar - foo"));
  EXPECT_THAT(data.constData(), HasSubstr("http://www.example.com/foo.mp3"));
}

TEST_F(M3UParserTest, ParsesUTF8) {
  QByteArray data = "#EXTM3U\n"
                    "#EXTINF:123,Разные - исполнители\n"
                    "/foo/Разные/исполнители.mp3\n";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  M3UParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ(6, songs[0].artist().length());
  EXPECT_EQ(11, songs[0].title().length());
  EXPECT_EQ(QString::fromUtf8("Разные"), songs[0].artist());
  EXPECT_EQ(QString::fromUtf8("исполнители"), songs[0].title());
  EXPECT_EQ(QUrl::fromLocalFile(QString::fromUtf8("/foo/Разные/исполнители.mp3")),
            songs[0].url());
}
