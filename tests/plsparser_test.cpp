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

#include "playlistparsers/plsparser.h"

#include <QBuffer>
#include <QFile>
#include <QTemporaryFile>
#include <QtDebug>

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

class PLSParserTest : public ::testing::Test {
protected:
  shared_ptr<QFile> Open(const QString& filename) {
    shared_ptr<QFile> ret(new QFile(":/testdata/" + filename));
    if (!ret->open(QIODevice::ReadOnly))
      ret.reset();
    return ret;
  }

  PLSParser parser_;
};

TEST_F(PLSParserTest, ParseOneTrack) {
  shared_ptr<QFile> file(Open("pls_one.pls"));

  SongList songs = parser_.Load(file.get(), QDir("/relative/to/"));
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ("/relative/to/filename with spaces.mp3", songs[0].filename());
  EXPECT_EQ("Title", songs[0].title());
  EXPECT_EQ(123, songs[0].length());
}

TEST_F(PLSParserTest, ParseSomaFM) {
  shared_ptr<QFile> file(Open("pls_somafm.pls"));

  SongList songs = parser_.Load(file.get());
  ASSERT_EQ(4, songs.length());
  EXPECT_EQ("http://streamer-dtc-aa05.somafm.com:80/stream/1018", songs[0].filename());
  EXPECT_EQ("http://streamer-mtc-aa03.somafm.com:80/stream/1018", songs[1].filename());
  EXPECT_EQ("http://streamer-ntc-aa04.somafm.com:80/stream/1018", songs[2].filename());
  EXPECT_EQ("http://ice.somafm.com/groovesalad", songs[3].filename());
  EXPECT_EQ("SomaFM: Groove Salad (#1 128k mp3): A nicely chilled plate of ambient beats and grooves.", songs[0].title());
  EXPECT_EQ("SomaFM: Groove Salad (#2 128k mp3): A nicely chilled plate of ambient beats and grooves.", songs[1].title());
  EXPECT_EQ("SomaFM: Groove Salad (#3 128k mp3): A nicely chilled plate of ambient beats and grooves.", songs[2].title());
  EXPECT_EQ(-1, songs[0].length());
  EXPECT_EQ(Song::Type_Stream, songs[0].filetype());
}

TEST_F(PLSParserTest, ParseSomaFM2) {
  shared_ptr<QFile> file(Open("secretagent.pls"));

  SongList songs = parser_.Load(file.get());
  ASSERT_EQ(4, songs.length());
  EXPECT_EQ("http://streamer-ntc-aa03.somafm.com:80/stream/1021", songs[0].filename());
  EXPECT_EQ("http://streamer-mtc-aa04.somafm.com:80/stream/1021", songs[1].filename());
  EXPECT_EQ("http://streamer-dtc-aa05.somafm.com:80/stream/1021", songs[2].filename());
  EXPECT_EQ("http://ice.somafm.com/secretagent", songs[3].filename());
  EXPECT_EQ("SomaFM: Secret Agent (#1 128k mp3): The soundtrack for your stylish, mysterious, dangerous life. For Spies and PIs too!", songs[0].title());
  EXPECT_EQ("SomaFM: Secret Agent (#2 128k mp3): The soundtrack for your stylish, mysterious, dangerous life. For Spies and PIs too!", songs[1].title());
  EXPECT_EQ("SomaFM: Secret Agent (#3 128k mp3): The soundtrack for your stylish, mysterious, dangerous life. For Spies and PIs too!", songs[2].title());
  EXPECT_EQ(-1, songs[0].length());
  EXPECT_EQ(Song::Type_Stream, songs[0].filetype());
}

TEST_F(PLSParserTest, SaveAndLoad) {
  Song one;
  one.set_filename("http://www.example.com/foo.mp3");
  one.set_title("Foo, with, some, commas");

  Song two;
  two.set_filename("relative/bar.mp3");
  two.set_title("Bar");
  two.set_length(123);

  SongList songs;
  songs << one << two;

  QTemporaryFile temp;
  temp.open();
  parser_.Save(songs, &temp);

  temp.seek(0);
  songs = parser_.Load(&temp, QDir("/meep"));

  ASSERT_EQ(2, songs.count());
  EXPECT_EQ(one.filename(), songs[0].filename());
  EXPECT_EQ("/meep/relative/bar.mp3", songs[1].filename());
  EXPECT_EQ(one.title(), songs[0].title());
  EXPECT_EQ(two.title(), songs[1].title());
  EXPECT_EQ(one.length(), songs[0].length());
  EXPECT_EQ(two.length(), songs[1].length());
}
