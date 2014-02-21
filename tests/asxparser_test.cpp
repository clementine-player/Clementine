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
#include "playlistparsers/asxparser.h"

#include <QBuffer>
#include <QUrl>

using ::testing::HasSubstr;

class ASXParserTest : public ::testing::Test {

};

TEST_F(ASXParserTest, ParsesOneTrackFromXML) {
  QByteArray data =
      "<asx version=\"3.0\"><title>foobar</title><entry>"
        "<ref href=\"http://example.com/foo.mp3\"/>"
        "<title>Foo</title>"
        "<author>Bar</author>"
        "<copyright>mumble mumble</copyright>"
      "</entry></asx>";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  ASXParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.length());
  const Song& song = songs[0];
  EXPECT_EQ("Foo", song.title());
  EXPECT_EQ("Bar", song.artist());
  EXPECT_EQ(QUrl("http://example.com/foo.mp3"), song.url());
  EXPECT_TRUE(song.is_valid());
}

TEST_F(ASXParserTest, ParsesMoreThanOneTrackFromXML) {
  QByteArray data =
      "<asx><entry>"
        "<entry>"
          "<ref href=\"http://example.com/foo.mp3\"/>"
        "</entry>"
        "<entry>"
          "<ref href=\"http://example.com/bar.mp3\"/>"
        "</entry>"
      "</entry></asx>";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  ASXParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(2, songs.length());
  EXPECT_EQ(QUrl("http://example.com/foo.mp3"), songs[0].url());
  EXPECT_EQ(QUrl("http://example.com/bar.mp3"), songs[1].url());
  EXPECT_TRUE(songs[0].is_stream());
  EXPECT_TRUE(songs[1].is_stream());
}

TEST_F(ASXParserTest, ParsesBrokenXmlEntities) {
  QByteArray data =
      "<asx version = \"3.0\">"
      "<Title>DI.fm</Title>"
      "<Author>Digitally Imported Radio</Author>"
      "<Copyright>2006 Digitally Imported., Inc</Copyright>"
      "<MoreInfo href=\"http://www.di.fm/\" />"
      "  <entry>"
      "    <PARAM name=\"HTMLView\" value=\"http://www.di.fm/classictrance/pro/mini_playlist.html\"/>"
      "    <ref href = \"mms://72.26.204.105/classictrance128k?user=h&pass=xxxxxxxxxxxxxxx\"/>"
      "    <Title>Classic Trance</Title>"
      "    <Author>Digitally Imported Premium</Author>"
      "  </entry>"
      "</asx>";

  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  ASXParser parser(nullptr);
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ(QUrl("mms://72.26.204.105/classictrance128k?user=h&pass=xxxxxxxxxxxxxxx"), songs[0].url());
}

TEST_F(ASXParserTest, SavesSong) {
  QByteArray data;
  QBuffer buffer(&data);
  buffer.open(QIODevice::WriteOnly);
  ASXParser parser(nullptr);
  Song one;
  one.set_url(QUrl("http://www.example.com/foo.mp3"));
  one.set_filetype(Song::Type_Stream);
  one.set_title("foo");
  one.set_length_nanosec(123 * kNsecPerSec);
  one.set_artist("bar");
  SongList songs;
  songs << one;

  parser.Save(songs, &buffer);
  EXPECT_THAT(data.constData(), HasSubstr("<ref href=\"http://www.example.com/foo.mp3\"/>"));
  EXPECT_THAT(data.constData(), HasSubstr("<title>foo</title>"));
  EXPECT_THAT(data.constData(), HasSubstr("<author>bar</author>"));
}

TEST_F(ASXParserTest, ParsesSomaFM) {
  QFile somafm(":/testdata/secretagent.asx");
  somafm.open(QIODevice::ReadOnly);

  ASXParser parser(nullptr);
  SongList songs = parser.Load(&somafm);

  ASSERT_EQ(4, songs.count());
  EXPECT_EQ("SomaFM: Secret Agent", songs[0].title());
  EXPECT_EQ("Keep us on the air! Click Support SomaFM above!", songs[0].artist());
  EXPECT_EQ(QUrl("http://streamer-ntc-aa03.somafm.com:80/stream/1021"), songs[0].url());
}
