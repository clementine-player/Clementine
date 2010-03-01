#include "gtest/gtest.h"
#include "test_utils.h"

#include "m3uparser.h"

class M3UParserTest : public ::testing::Test {
 protected:
  M3UParserTest()
      : parser_(NULL) {
  }

  M3UParser parser_;
};

TEST_F(M3UParserTest, ParsesMetadata) {
  QString line("#EXTINF:123,Foo artist - Foo track");
  M3UParser::Metadata metadata;
  ASSERT_TRUE(parser_.ParseMetadata(line, &metadata));
  EXPECT_EQ("Foo artist", metadata.artist.toStdString());
  EXPECT_EQ("Foo track", metadata.title.toStdString());
  EXPECT_EQ(123, metadata.length);
}

TEST_F(M3UParserTest, ParsesTrackLocation) {
  QString line("/foo/bar.mp3");
  QUrl url;
  ASSERT_TRUE(parser_.ParseTrackLocation(line, &url));
  EXPECT_EQ(QUrl("file:///foo/bar.mp3"), url);
}
