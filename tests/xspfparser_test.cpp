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
#include "gtest/gtest.h"

#include "core/xspfparser.h"

#include <QBuffer>

class XSPFParserTest : public ::testing::Test {

};

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
  XSPFParser parser(&buffer);
  const SongList& songs = parser.Parse();
  ASSERT_EQ(1, songs.length());
  const Song& song = songs[0];
  EXPECT_EQ("Foo", song.title());
  EXPECT_EQ("Bar", song.artist());
  EXPECT_EQ("Baz", song.album());
  EXPECT_EQ("http://example.com/foo.mp3", song.filename());
  EXPECT_EQ(60, song.length());
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
  XSPFParser parser(&buffer);
  const SongList& songs = parser.Parse();
  ASSERT_EQ(2, songs.length());
  EXPECT_EQ("http://example.com/foo.mp3", songs[0].filename());
  EXPECT_EQ("http://example.com/bar.mp3", songs[1].filename());
  EXPECT_EQ(Song::Type_Stream, songs[0].filetype());
  EXPECT_EQ(Song::Type_Stream, songs[1].filetype());
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
  XSPFParser parser(&buffer);
  const SongList& songs = parser.Parse();
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ(-1, songs[0].length());
}
