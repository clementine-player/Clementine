#include "test_utils.h"
#include "gtest/gtest.h"

#include "xspfparser.h"

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
  EXPECT_TRUE(song.is_valid());
}
