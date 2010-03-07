#include "gtest/gtest.h"
#include "test_utils.h"
#include "mock_taglib.h"

#include "m3uparser.h"

#include <QBuffer>
#include <QTemporaryFile>

class M3UParserTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    testing::DefaultValue<TagLib::String>::Set("foobarbaz");
  }

  M3UParserTest()
      : parser_(NULL) {
  }

  M3UParser parser_;
  MockFileRefFactory taglib_;
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
  QTemporaryFile temp;
  temp.open();
  taglib_.ExpectCall(temp.fileName(), "foo", "bar", "baz");
  Song song(&taglib_);
  QString line(temp.fileName());
  ASSERT_TRUE(parser_.ParseTrackLocation(line, &song));
  EXPECT_EQ(temp.fileName(), song.filename());
  EXPECT_EQ("foo", song.title());
  EXPECT_EQ("bar", song.artist());
  EXPECT_EQ("baz", song.album());
}

TEST_F(M3UParserTest, ParsesTrackLocationRelative) {
  QTemporaryFile temp;
  temp.open();
  QFileInfo info(temp);
  taglib_.ExpectCall(temp.fileName(), "foo", "bar", "baz");
  M3UParser parser(NULL, info.dir());
  QString line(info.fileName());
  Song song(&taglib_);
  ASSERT_TRUE(parser.ParseTrackLocation(line, &song));
  EXPECT_EQ(temp.fileName(), song.filename());
  EXPECT_EQ("foo", song.title());
}

TEST_F(M3UParserTest, ParsesTrackLocationHttp) {
  QString line("http://example.com/foo/bar.mp3");
  Song song;
  ASSERT_TRUE(parser_.ParseTrackLocation(line, &song));
  EXPECT_EQ("http://example.com/foo/bar.mp3", song.filename());
}

TEST_F(M3UParserTest, ParsesSongsFromDevice) {
  QByteArray data = "#EXTM3U\n"
                    "#EXTINF:123,Some Artist - Some Title\n"
                    "http://foo.com/bar/somefile.mp3\n";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  M3UParser parser(&buffer);
  const QList<Song>& songs = parser.Parse();
  ASSERT_EQ(1, songs.size());
  Song s = songs[0];
  EXPECT_EQ("Some Artist", s.artist());
  EXPECT_EQ("Some Title", s.title());
  EXPECT_EQ(123, s.length());
  EXPECT_PRED_FORMAT2(::testing::IsSubstring,
      "http://foo.com/bar/somefile.mp3", s.filename().toStdString());
}

TEST_F(M3UParserTest, ParsesNonExtendedM3U) {
  QByteArray data = "http://foo.com/bar/somefile.mp3\n"
                    "http://baz.com/thing.mp3\n";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  M3UParser parser(&buffer, QDir("somedir"));
  const QList<Song>& songs = parser.Parse();
  ASSERT_EQ(2, songs.size());
  EXPECT_PRED_FORMAT2(::testing::IsSubstring,
      "http://foo.com/bar/somefile.mp3", songs[0].filename().toStdString());
  EXPECT_PRED_FORMAT2(::testing::IsSubstring,
      "http://baz.com/thing.mp3", songs[1].filename().toStdString());
  EXPECT_EQ(-1, songs[0].length());
  EXPECT_EQ(-1, songs[1].length());
  EXPECT_TRUE(songs[0].artist().isEmpty());
}
