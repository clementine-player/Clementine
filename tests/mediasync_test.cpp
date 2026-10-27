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

#include <QCoreApplication>
#include <QSettings>
#include <QUuid>

#include "core/song.h"
#include "core/urlhandler.h"
#include "gtest/gtest.h"
#include "networkremote/networkremote.h"
#include "networkremote/sync/serverid.h"
#include "networkremote/sync/syncsong.h"
#include "networkremote/sync/syncsourceid.h"
#include "test_utils.h"

// For gtest's messages.
std::ostream& operator<<(std::ostream& stream, SyncSourceId::Kind kind) {
  return stream << static_cast<int>(kind);
}
std::ostream& operator<<(std::ostream& stream, const SyncSourceId& id) {
  return stream << id.ToString().toStdString();
}
std::ostream& operator<<(std::ostream& stream,
                         UrlHandler::Downloadability downloadability) {
  return stream << static_cast<int>(downloadability);
}

namespace {

using Downloadability = UrlHandler::Downloadability;
using Kind = SyncSourceId::Kind;

class FakeUrlHandler : public UrlHandler {
 public:
  explicit FakeUrlHandler(Downloadability downloadability)
      : downloadability_(downloadability) {}

  QString scheme() const { return "fake"; }
  LoadResult StartLoading(const QUrl& url) {
    return LoadResult(url, LoadResult::NoMoreTracks);
  }
  Downloadability downloadability(const Song&) const {
    return downloadability_;
  }

 private:
  Downloadability downloadability_;
};

Song MakeSong(const QUrl& url, qint64 length_nanosec) {
  Song song;
  song.Init("Title", "Artist", "Album", length_nanosec);
  song.set_url(url);
  song.set_filetype(Song::Type_Mpeg);
  return song;
}

constexpr qint64 kThreeMinutes = 180ll * 1000 * 1000 * 1000;

TEST(SyncSourceIdTest, WritesEachKind) {
  EXPECT_EQ("library/song/42", SyncSourceId::ForSong(42).ToString());
  EXPECT_EQ("library/album/Pixies/Doolittle",
            SyncSourceId::ForAlbum("Pixies", "Doolittle").ToString());
  EXPECT_EQ("library/artist/Pixies",
            SyncSourceId::ForArtist("Pixies").ToString());
  EXPECT_EQ("playlist/7", SyncSourceId::ForPlaylist(7).ToString());
  EXPECT_EQ("smart/songs/0b6e",
            SyncSourceId::ForSmartPlaylist("songs", "0b6e").ToString());
  EXPECT_EQ("browse/Plex/1234",
            SyncSourceId::ForInternetNode("Plex", "1234").ToString());
}

TEST(SyncSourceIdTest, EncodesSlashesAndSpaces) {
  const SyncSourceId id = SyncSourceId::ForAlbum("AC/DC", "Back in Black");
  EXPECT_EQ("library/album/AC%2FDC/Back%20in%20Black", id.ToString());

  const SyncSourceId parsed = SyncSourceId::Parse(id.ToString());
  EXPECT_EQ(Kind::Album, parsed.kind());
  EXPECT_EQ(QStringList({"AC/DC", "Back in Black"}), parsed.parts());
}

TEST(SyncSourceIdTest, RoundTrips) {
  const QList<SyncSourceId> ids = {
      SyncSourceId::ForSong(0),
      SyncSourceId::ForAlbum("Sigur Rós", "( )"),
      SyncSourceId::ForArtist("Björk"),
      SyncSourceId::ForPlaylist(12),
      SyncSourceId::ForSmartPlaylist("subsonic_songs",
                                     "4f1c2d6e-8a0b-4c8e-9b7a-2e3f5d6c7b8a"),
      SyncSourceId::ForInternetNode("Google Drive", "1a2B_3c-4D"),
  };
  for (const SyncSourceId& id : ids) {
    ASSERT_TRUE(id.is_valid()) << id.ToString().toStdString();
    EXPECT_EQ(id, SyncSourceId::Parse(id.ToString()))
        << id.ToString().toStdString();
  }
}

TEST(SyncSourceIdTest, ParsesNumericIds) {
  const SyncSourceId song = SyncSourceId::Parse("library/song/42");
  EXPECT_EQ(Kind::Song, song.kind());
  EXPECT_EQ(42, song.id());

  const SyncSourceId playlist = SyncSourceId::Parse("playlist/3");
  EXPECT_EQ(Kind::Playlist, playlist.kind());
  EXPECT_EQ(3, playlist.id());
}

TEST(SyncSourceIdTest, RejectsMalformedIds) {
  for (const char* source_id : {
           "",
           "library",
           "library/song",
           "library/song/",
           "library/song/abc",
           "library/song/-1",
           "library/song/042",
           "library/song/4/2",
           "library/album/Pixies",
           "library/album/Pixies/",
           "library/album//Doolittle",
           "library/album/Pixies/Doolittle/extra",
           "library/genre/Rock",
           "playlist/one",
           "smart/songs",
           "browse/Plex",
           "file:///music/song.mp3",
       }) {
    EXPECT_FALSE(SyncSourceId::Parse(source_id).is_valid()) << source_id;
  }
}

TEST(SyncSourceIdTest, RejectsEmptyParts) {
  EXPECT_FALSE(SyncSourceId::ForSong(-1).is_valid());
  EXPECT_FALSE(SyncSourceId::ForAlbum("", "Doolittle").is_valid());
  EXPECT_FALSE(SyncSourceId::ForAlbum("Pixies", "").is_valid());
  EXPECT_FALSE(SyncSourceId::ForArtist("").is_valid());
  EXPECT_FALSE(SyncSourceId::ForPlaylist(-1).is_valid());
  EXPECT_FALSE(SyncSourceId::ForSmartPlaylist("songs", "").is_valid());
  EXPECT_FALSE(SyncSourceId::ForInternetNode("", "1234").is_valid());
  EXPECT_TRUE(SyncSourceId().ToString().isEmpty());
}

TEST(SyncSongTest, LocalFilesAreDownloadable) {
  Song song = MakeSong(QUrl("file:///music/a.mp3"), kThreeMinutes);
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, nullptr));

  // A local file whose length isn't known yet is still a file.
  song.set_length_nanosec(-1);
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, nullptr));
}

TEST(SyncSongTest, ServiceSongsWithALengthAreDownloadable) {
  const Song song = MakeSong(QUrl("plex://server/library/1"), kThreeMinutes);
  const FakeUrlHandler handler(Downloadability::Downloadable);
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, &handler));
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, nullptr));
}

TEST(SyncSongTest, StreamsAreNotDownloadable) {
  Song stream = MakeSong(QUrl("http://radio.example/stream"), kThreeMinutes);
  stream.set_filetype(Song::Type_Stream);
  EXPECT_EQ(Downloadability::NotDownloadable,
            SyncSong::Downloadability(stream, nullptr));

  // No length, and not a file: something that plays forever.
  const Song endless = MakeSong(QUrl("http://radio.example/stream"), -1);
  EXPECT_EQ(Downloadability::NotDownloadable,
            SyncSong::Downloadability(endless, nullptr));
}

TEST(SyncSongTest, HandlersCanRefuse) {
  const Song song = MakeSong(QUrl("fake://station/1"), kThreeMinutes);
  const FakeUrlHandler handler(Downloadability::NotDownloadable);
  EXPECT_EQ(Downloadability::NotDownloadable,
            SyncSong::Downloadability(song, &handler));
}

TEST(SyncSongTest, ItemKeys) {
  Song library = MakeSong(QUrl("file:///music/a.mp3"), kThreeMinutes);
  library.set_id(17);
  EXPECT_EQ("library/17", SyncSong::ItemKey(library));

  const Song plex = MakeSong(QUrl("plex://server/library/1"), kThreeMinutes);
  EXPECT_EQ("url/plex%3A%2F%2Fserver%2Flibrary%2F1", SyncSong::ItemKey(plex));

  EXPECT_TRUE(SyncSong::ItemKey(Song()).isEmpty());
}

TEST(SyncSongTest, ItemVersionChangesWithTheFileAndPreset) {
  Song song = MakeSong(QUrl("file:///music/a.mp3"), kThreeMinutes);
  song.set_mtime(1700000000);
  song.set_filesize(4000000);
  const QString original = SyncSong::ItemVersion(song);
  EXPECT_EQ("1700000000-4000000", original);

  const QString converted = SyncSong::ItemVersion(song, "opus-160");
  EXPECT_NE(original, converted);
  EXPECT_EQ(converted, SyncSong::ItemVersion(song, "opus-160"));

  song.set_mtime(1700000001);
  EXPECT_NE(original, SyncSong::ItemVersion(song));
}

class ServerIdTest : public ::testing::Test {
 protected:
  void SetUp() override {
    QCoreApplication::setOrganizationName("ClementineTests");
    QCoreApplication::setApplicationName("mediasync_test");
    Clear();
  }
  void TearDown() override { Clear(); }

  void Clear() {
    QSettings s;
    s.beginGroup(NetworkRemote::kSettingsGroup);
    s.remove("server_id");
  }
};

TEST_F(ServerIdTest, IsAStableUuid) {
  const QString id = NetworkRemoteServerId();
  EXPECT_FALSE(QUuid::fromString(id).isNull()) << id.toStdString();
  EXPECT_EQ(id, NetworkRemoteServerId());
}

TEST_F(ServerIdTest, IsNewOnceCleared) {
  const QString first = NetworkRemoteServerId();
  Clear();
  EXPECT_NE(first, NetworkRemoteServerId());
}

}  // namespace
