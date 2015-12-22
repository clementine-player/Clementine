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

#include "test_utils.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

#include "core/timeconstants.h"
#include "playlistparsers/xspfparser.h"

#include <QBuffer>
#include <QUrl>

using ::testing::HasSubstr;

class XSPFParserTest : public ::testing::Test {};

TEST_F(XSPFParserTest, ParsesOneTrackFromXML) {
  QByteArray data =
      "<playlist><trackList><track>"
      "<location>http://example.com/foo.mp3</location>"
      "<title>Foo</title>"
      "<creator>Bar</creator>"
      "<album>Baz</album>"
      "<duration>60000</duration>"
      "<image>http://example.com/albumcover.jpg</image>"
      "<info>http://example.com</info>"
      "</track></trackList></playlist>";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  XSPFParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.length());
  const Song& song = songs[0];
  EXPECT_EQ("Foo", song.title());
  EXPECT_EQ("Bar", song.artist());
  EXPECT_EQ("Baz", song.album());
  EXPECT_EQ(QUrl("http://example.com/foo.mp3"), song.url());
  EXPECT_EQ(60 * kNsecPerSec, song.length_nanosec());
  EXPECT_TRUE(song.is_valid());
}

TEST_F(XSPFParserTest, ParsesMoreThanOneTrackFromXML) {
  QByteArray data =
      "<playlist><trackList>"
      "<track>"
      "<location>http://example.com/foo.mp3</location>"
      "</track>"
      "<track>"
      "<location>http://example.com/bar.mp3</location>"
      "</track>"
      "</trackList></playlist>";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  XSPFParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(2, songs.length());
  EXPECT_EQ(QUrl("http://example.com/foo.mp3"), songs[0].url());
  EXPECT_EQ(QUrl("http://example.com/bar.mp3"), songs[1].url());
  EXPECT_TRUE(songs[0].is_stream());
  EXPECT_TRUE(songs[1].is_stream());
}

TEST_F(XSPFParserTest, IgnoresInvalidLength) {
  QByteArray data =
      "<playlist><trackList><track>"
      "<location>http://example.com/foo.mp3</location>"
      "<title>Foo</title>"
      "<creator>Bar</creator>"
      "<album>Baz</album>"
      "<duration>60000qwerty</duration>"
      "<image>http://example.com/albumcover.jpg</image>"
      "<info>http://example.com</info>"
      "</track></trackList></playlist>";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  XSPFParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ(-1, songs[0].length_nanosec());
}

TEST_F(XSPFParserTest, ParsesTrackNum) {
  QByteArray data =
      "<playlist><trackList><track>"
      "<location>http://example.com/foo.mp3</location>"
      "<title>Foo</title>"
      "<creator>Bar</creator>"
      "<album>Baz</album>"
      "<image>http://example.com/albumcover.jpg</image>"
      "<info>http://example.com</info>"
      "<trackNum>42</trackNum>"
      "</track></trackList></playlist>";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  XSPFParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ(42, songs[0].track());
}

TEST_F(XSPFParserTest, SavesSong) {
  QByteArray data;
  QBuffer buffer(&data);
  buffer.open(QIODevice::WriteOnly);
  XSPFParser parser(nullptr);
  Song one;
  one.set_url(QUrl("http://www.example.com/foo.mp3"));
  one.set_filetype(Song::Type_Stream);
  one.set_title("foo");
  one.set_length_nanosec(123 * kNsecPerSec);
  one.set_artist("bar");
  SongList songs;
  songs << one;

  parser.Save(songs, &buffer);
  EXPECT_THAT(data.constData(),
              HasSubstr("<location>http://www.example.com/foo.mp3</location>"));
  EXPECT_THAT(data.constData(), HasSubstr("<duration>123000</duration>"));
  EXPECT_THAT(data.constData(), HasSubstr("<title>foo</title>"));
  EXPECT_THAT(data.constData(), HasSubstr("<creator>bar</creator>"));
}

TEST_F(XSPFParserTest, SavesLocalFile) {
  QByteArray data;
  QBuffer buffer(&data);
  buffer.open(QIODevice::WriteOnly);
  XSPFParser parser(nullptr);
  Song one;
  one.set_url(QUrl("file:///bar/foo.mp3"));
  one.set_filetype(Song::Type_Mpeg);
  one.set_title("foo");
  one.set_length_nanosec(123 * kNsecPerSec);
  one.set_artist("bar");
  one.set_track(42);
  SongList songs;
  songs << one;

  parser.Save(songs, &buffer);
  EXPECT_THAT(data.constData(), HasSubstr("<location>/bar/foo.mp3</location>"));
  EXPECT_THAT(data.constData(), HasSubstr("<duration>123000</duration>"));
  EXPECT_THAT(data.constData(), HasSubstr("<title>foo</title>"));
  EXPECT_THAT(data.constData(), HasSubstr("<creator>bar</creator>"));
  EXPECT_THAT(data.constData(), HasSubstr("<trackNum>42</trackNum>"));
}
