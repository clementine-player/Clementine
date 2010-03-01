#include "song.h"
#include <lastfm/Track>

#include "gtest/gtest.h"

std::ostream& operator<<(std::ostream& stream, const QString& str) {
  stream << str.toStdString();
  return stream;
}

namespace {

class SongTest : public ::testing::Test {
  
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


}  // namespace
