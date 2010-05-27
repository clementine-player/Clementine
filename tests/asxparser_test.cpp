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

#include "test_utils.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

#include "playlistparsers/asxparser.h"

#include <QBuffer>

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
  ASXParser parser;
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(1, songs.length());
  const Song& song = songs[0];
  EXPECT_EQ("Foo", song.title());
  EXPECT_EQ("Bar", song.artist());
  EXPECT_EQ("http://example.com/foo.mp3", song.filename());
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
  ASXParser parser;
  SongList songs = parser.Load(&buffer);
  ASSERT_EQ(2, songs.length());
  EXPECT_EQ("http://example.com/foo.mp3", songs[0].filename());
  EXPECT_EQ("http://example.com/bar.mp3", songs[1].filename());
  EXPECT_EQ(Song::Type_Stream, songs[0].filetype());
  EXPECT_EQ(Song::Type_Stream, songs[1].filetype());
}

TEST_F(ASXParserTest, SavesSong) {
  QByteArray data;
  QBuffer buffer(&data);
  buffer.open(QIODevice::WriteOnly);
  ASXParser parser;
  Song one;
  one.set_filename("http://www.example.com/foo.mp3");
  one.set_filetype(Song::Type_Stream);
  one.set_title("foo");
  one.set_length(123);
  one.set_artist("bar");
  SongList songs;
  songs << one;

  parser.Save(songs, &buffer);
  EXPECT_THAT(data.constData(), HasSubstr("<ref href=\"http://www.example.com/foo.mp3\"/>"));
  EXPECT_THAT(data.constData(), HasSubstr("<title>foo</title>"));
  EXPECT_THAT(data.constData(), HasSubstr("<author>bar</author>"));
}
