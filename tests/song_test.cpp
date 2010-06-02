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

#include "core/song.h"
#include "radio/fixlastfm.h"
#include <lastfm/Track>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils.h"
#include "mock_taglib.h"

#include <QTemporaryFile>
#include <QTextCodec>

namespace {

class SongTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    // Return something from uninteresting mock functions.
    testing::DefaultValue<TagLib::String>::Set("foobarbaz");
  }

  MockFileRefFactory mock_factory_;
};


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

TEST_F(SongTest, InitsFromFile) {
  QTemporaryFile temp;
  temp.open();
  mock_factory_.ExpectCall(temp.fileName(), "Foo", "Bar", "Baz");
  Song song(&mock_factory_);
  song.InitFromFile(temp.fileName(), 42);
  EXPECT_EQ("Foo", song.title());
  EXPECT_EQ("Bar", song.artist());
  EXPECT_EQ("Baz", song.album());
}

TEST_F(SongTest, DetectsWindows1251) {
  char cp1251[] = { 0xc2, 0xfb, 0xe4, 0xfb, 0xf5, 0xe0, 0xe9, 0x00 };  // Выдыхай
  UniversalEncodingHandler handler;
  TagLib::ByteVector bytes(cp1251);
  TagLib::String str = handler.parse(bytes);
  EXPECT_FALSE(str.isAscii());
  EXPECT_FALSE(str.isLatin1());
  EXPECT_STREQ("Выдыхай", str.to8Bit(true).c_str());
}

TEST_F(SongTest, LeavesASCIIAlone) {
  const char* ascii = "foobar";
  UniversalEncodingHandler handler;
  TagLib::ByteVector bytes(ascii);
  TagLib::String str = handler.parse(bytes);
  EXPECT_TRUE(str.isAscii());
  EXPECT_TRUE(str.isLatin1());
  EXPECT_STREQ("foobar", str.to8Bit(false).c_str());
}

TEST_F(SongTest, FixesCP866) {
  const char cp866[] = { 0x8a, 0xa8, 0xad, 0xae, '\0' };  // Кино
  TagLib::ByteVector bytes(cp866);
  TagLib::String str(bytes);
  QString fixed = UniversalEncodingHandler::FixEncoding(str);
  EXPECT_EQ(4, fixed.length());
  EXPECT_STREQ("Кино", fixed.toUtf8().constData());
}

TEST_F(SongTest, FixesWindows1251) {
  const char w1251[] = { 0xca, 0xe8, 0xed, 0xee, '\0' };  // Кино
  TagLib::ByteVector bytes(w1251);
  TagLib::String str(bytes);
  QString fixed = UniversalEncodingHandler::FixEncoding(str);
  EXPECT_EQ(4, fixed.length());
  EXPECT_STREQ("Кино", fixed.toUtf8().constData());
}

TEST_F(SongTest, DoesNotFixAscii) {
  TagLib::ByteVector bytes("foobar");
  TagLib::String str(bytes);
  QString fixed = UniversalEncodingHandler::FixEncoding(str);
  EXPECT_EQ(6, fixed.length());
  EXPECT_STREQ("foobar", fixed.toUtf8().constData());
}

TEST_F(SongTest, DoesNotFixUtf8) {
  TagLib::ByteVector bytes("Кино");
  TagLib::String str(bytes, TagLib::String::UTF8);
  QString fixed = UniversalEncodingHandler::FixEncoding(str);
  EXPECT_EQ(4, fixed.length());
  EXPECT_STREQ("Кино", fixed.toUtf8().constData());
}

TEST_F(SongTest, DoesNotFixExtendedAscii) {
  char latin1[] = { 'R', 0xf6, 'y', 'k', 's', 'o', 'p', 'p', 0x00 };
  QTextCodec* codec = QTextCodec::codecForName("latin1");
  QString unicode = codec->toUnicode(latin1);
  TagLib::String str(latin1);
  QString fixed = UniversalEncodingHandler::FixEncoding(str);
  EXPECT_EQ(fixed, unicode);
}

TEST_F(SongTest, FixesUtf8MungedIntoLatin1) {
  char latin1[] = { 'E', 's', 't', 'h', 'e', 'r', 0xe2, 0x80, 0x99, 's', 0x00 };
  TagLib::String str(latin1, TagLib::String::Latin1);
  QString fixed = UniversalEncodingHandler::FixEncoding(str);
  EXPECT_EQ(8, fixed.length());
  EXPECT_EQ(QString::fromUtf8("Esther’s"), fixed);
}

}  // namespace
