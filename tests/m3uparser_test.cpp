#include "gtest/gtest.h"
#include "test_utils.h"

#include "m3uparser.h"

#include <QBuffer>

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

TEST_F(M3UParserTest, ParsesTrackLocationRelative) {
  M3UParser parser(NULL, QDir("/tmp"));
  QString line("foo/bar.mp3");
  QUrl url;
  ASSERT_TRUE(parser.ParseTrackLocation(line, &url));
  EXPECT_EQ(QUrl("file:///tmp/foo/bar.mp3"), url);
}

TEST_F(M3UParserTest, ParsesTrackLocationHttp) {
  QString line("http://example.com/foo/bar.mp3");
  QUrl url;
  ASSERT_TRUE(parser_.ParseTrackLocation(line, &url));
  EXPECT_EQ(QUrl("http://example.com/foo/bar.mp3"), url);
}

#ifdef Q_OS_WIN32
TEST_F(M3UParserTest, ParsesTrackLocationAbsoluteWindows) {
  QString line("c:/foo/bar.mp3");
  QUrl url;
  ASSERT_TRUE(parser_.ParseTrackLocation(line, &url));
  EXPECT_EQ(QUrl("file:///c:/foo/bar.mp3"), url);
}
#endif  // Q_OS_WIN32

TEST_F(M3UParserTest, ParsesSongsFromDevice) {
  QByteArray data = "#EXTM3U\n"
                    "#EXTINF:123,Some Artist - Some Title\n"
                    "foo/bar/somefile.mp3\n";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  M3UParser parser(&buffer, QDir("somedir"));
  const QList<Song>& songs = parser.Parse();
  ASSERT_EQ(1, songs.size());
  Song s = songs[0];
  EXPECT_EQ("Some Artist", s.artist());
  EXPECT_EQ("Some Title", s.title());
  EXPECT_EQ(123, s.length());
  EXPECT_PRED_FORMAT2(::testing::IsSubstring,
      "somedir/foo/bar/somefile.mp3", s.filename().toStdString());
}

TEST_F(M3UParserTest, ParsesNonExtendedM3U) {
  QByteArray data = "foo/bar/somefile.mp3\n"
                    "baz/thing.mp3\n";
  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  M3UParser parser(&buffer, QDir("somedir"));
  const QList<Song>& songs = parser.Parse();
  ASSERT_EQ(2, songs.size());
  EXPECT_PRED_FORMAT2(::testing::IsSubstring,
      "somedir/foo/bar/somefile.mp3", songs[0].filename().toStdString());
  EXPECT_PRED_FORMAT2(::testing::IsSubstring,
      "somedir/baz/thing.mp3", songs[1].filename().toStdString());
  EXPECT_EQ(-1, songs[0].length());
  EXPECT_EQ(-1, songs[1].length());
  EXPECT_TRUE(songs[0].artist().isEmpty());
}
