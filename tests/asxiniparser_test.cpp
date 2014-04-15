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

#include "playlistparsers/asxiniparser.h"
#include "playlistparsers/playlistparser.h"

#include <QBuffer>
#include <QUrl>

class AsxIniParserTest : public ::testing::Test {
protected:
  AsxIniParserTest() : parser_(nullptr) {}

  AsxIniParser parser_;
};

TEST_F(AsxIniParserTest, ParsesBasicTrackList) {
  QFile file(":/testdata/test.asxini");
  file.open(QIODevice::ReadOnly);

  SongList songs = parser_.Load(&file, "", QDir());
  ASSERT_EQ(2, songs.length());
  EXPECT_EQ(QUrl("http://195.245.168.21/antena3?MSWMExt=.asf"), songs[0].url());
  EXPECT_EQ(QUrl("http://195.245.168.21:80/antena3?MSWMExt=.asf"), songs[1].url());
  EXPECT_TRUE(songs[0].is_valid());
  EXPECT_TRUE(songs[1].is_valid());
}

TEST_F(AsxIniParserTest, Magic) {
  QFile file(":/testdata/test.asxini");
  file.open(QIODevice::ReadOnly);

  EXPECT_TRUE(parser_.TryMagic(file.read(PlaylistParser::kMagicSize)));
}

TEST_F(AsxIniParserTest, WritesBasicTrackList) {
  QByteArray data;
  QBuffer buffer(&data);
  buffer.open(QIODevice::WriteOnly);

  Song song;
  song.set_url(QUrl("http://www.example.com/foo.mp3"));

  SongList songs;
  songs << song;

  parser_.Save(songs, &buffer);
  EXPECT_EQ("[Reference]\nRef1=http://www.example.com/foo.mp3\n", QString(data));
}
