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

#include "core/database.h"
#include "core/song.h"
#include "core/timeconstants.h"
#include "core/urlhandler.h"
#include "gtest/gtest.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "networkremote/networkremote.h"
#include "networkremote/outgoingdatacreator.h"
#include "networkremote/sync/serverid.h"
#include "networkremote/sync/syncresolver.h"
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
std::ostream& operator<<(std::ostream& stream, SyncResolver::State state) {
  return stream << static_cast<int>(state);
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

constexpr qint64 kThreeMinutesNanosec = 3 * 60 * kNsecPerSec;

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
  Song song = MakeSong(QUrl("file:///music/a.mp3"), kThreeMinutesNanosec);
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, nullptr));

  // A local file whose length isn't known yet is still a file.
  song.set_length_nanosec(-1);
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, nullptr));
}

TEST(SyncSongTest, ServiceSongsWithALengthAreDownloadable) {
  const Song song =
      MakeSong(QUrl("plex://server/library/1"), kThreeMinutesNanosec);
  const FakeUrlHandler handler(Downloadability::Downloadable);
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, &handler));
  EXPECT_EQ(Downloadability::Downloadable,
            SyncSong::Downloadability(song, nullptr));
}

TEST(SyncSongTest, StreamsAreNotDownloadable) {
  Song stream =
      MakeSong(QUrl("http://radio.example/stream"), kThreeMinutesNanosec);
  stream.set_filetype(Song::Type_Stream);
  EXPECT_EQ(Downloadability::NotDownloadable,
            SyncSong::Downloadability(stream, nullptr));

  // No length, and not a file: something that plays forever.
  const Song endless = MakeSong(QUrl("http://radio.example/stream"), -1);
  EXPECT_EQ(Downloadability::NotDownloadable,
            SyncSong::Downloadability(endless, nullptr));
}

TEST(SyncSongTest, HandlersCanRefuse) {
  const Song song = MakeSong(QUrl("fake://station/1"), kThreeMinutesNanosec);
  const FakeUrlHandler handler(Downloadability::NotDownloadable);
  EXPECT_EQ(Downloadability::NotDownloadable,
            SyncSong::Downloadability(song, &handler));
}

TEST(SyncSongTest, RemotesAreToldOnTheWire) {
  const Song song = MakeSong(QUrl("fake://song"), kThreeMinutesNanosec);
  const FakeUrlHandler refuses(Downloadability::NotDownloadable);
  EXPECT_EQ(cpb::remote::DOWNLOADABILITY_DOWNLOADABLE,
            OutgoingDataCreator::DownloadabilityOf(song, nullptr));
  EXPECT_EQ(cpb::remote::DOWNLOADABILITY_NOT_DOWNLOADABLE,
            OutgoingDataCreator::DownloadabilityOf(song, &refuses));
}

TEST(SyncSongTest, ItemKeys) {
  Song library = MakeSong(QUrl("file:///music/a.mp3"), kThreeMinutesNanosec);
  library.set_id(17);
  EXPECT_EQ("library/17", SyncSong::ItemKey(library));

  const Song plex =
      MakeSong(QUrl("plex://server/library/1"), kThreeMinutesNanosec);
  EXPECT_EQ("url/plex%3A%2F%2Fserver%2Flibrary%2F1", SyncSong::ItemKey(plex));

  EXPECT_TRUE(SyncSong::ItemKey(Song()).isEmpty());
}

TEST(SyncSongTest, ItemVersionChangesWithTheFileAndPreset) {
  Song song = MakeSong(QUrl("file:///music/a.mp3"), kThreeMinutesNanosec);
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

class SyncResolverTest : public ::testing::Test {
 protected:
  void SetUp() override {
    database_.reset(new MemoryDatabase(nullptr));
    library_.reset(new LibraryBackend);
    library_->Init(database_.get(), Library::kSongsTable, Library::kDirsTable,
                   Library::kSubdirsTable, Library::kFtsTable);
    library_->AddDirectory("/music");

    // Two Pixies albums, given out of order, and a song by someone else.
    Add("Tame", "Pixies", "Doolittle", 1989, 1, 3);
    Add("Debaser", "Pixies", "Doolittle", 1989, 1, 1);
    Add("Bone Machine", "Pixies", "Surfer Rosa", 1988, 1, 1);
    Add("Wave of Mutilation", "Pixies", "Doolittle", 1989, 1, 7);
    Add("Clair de lune", "Claude Debussy", "Suite bergamasque", 1905, 1, 3);

    playlists_.songs = [this](int id, SongList* songs) {
      if (id != 7) return false;
      *songs = playlist_;
      return true;
    };
  }

  void Add(const QString& title, const QString& artist, const QString& album,
           int year, int disc, int track) {
    Song song;
    song.Init(title, artist, album, kThreeMinutesNanosec);
    song.set_year(year);
    song.set_disc(disc);
    song.set_track(track);
    song.set_directory_id(1);
    song.set_url(QUrl::fromLocalFile("/music/" + title + ".flac"));
    song.set_mtime(1);
    song.set_ctime(1);
    song.set_filesize(1000);
    library_->AddOrUpdateSongs(SongList() << song);
  }

  SyncResolver::Result Resolve(const SyncSourceId& source) {
    return SyncResolver(library_.get(), playlists_).Resolve(source);
  }

  static QStringList Titles(const SyncResolver::Result& result) {
    QStringList titles;
    for (const Song& song : result.songs) titles << song.title();
    return titles;
  }

  Song Find(const QString& title) {
    for (const Song& song : library_->GetAllSongs()) {
      if (song.title() == title) return song;
    }
    ADD_FAILURE() << "no song called " << title.toStdString();
    return Song();
  }

  std::unique_ptr<Database> database_;
  std::unique_ptr<LibraryBackend> library_;
  SyncResolver::Playlists playlists_;
  SongList playlist_;
};

TEST_F(SyncResolverTest, FindsASong) {
  const Song debaser = Find("Debaser");
  const SyncResolver::Result result =
      Resolve(SyncSourceId::ForSong(debaser.id()));
  EXPECT_EQ(SyncResolver::State::Ready, result.state);
  EXPECT_EQ(QStringList{"Debaser"}, Titles(result));
  EXPECT_TRUE(result.songs[0].is_library_song());
}

TEST_F(SyncResolverTest, FindsAnAlbumInOrder) {
  const SyncResolver::Result result =
      Resolve(SyncSourceId::ForAlbum("Pixies", "Doolittle"));
  EXPECT_EQ(SyncResolver::State::Ready, result.state);
  EXPECT_EQ((QStringList{"Debaser", "Tame", "Wave of Mutilation"}),
            Titles(result));
}

TEST_F(SyncResolverTest, FindsAnArtistsAlbumsInOrder) {
  const SyncResolver::Result result =
      Resolve(SyncSourceId::ForArtist("Pixies"));
  EXPECT_EQ(SyncResolver::State::Ready, result.state);
  EXPECT_EQ(
      (QStringList{"Bone Machine", "Debaser", "Tame", "Wave of Mutilation"}),
      Titles(result));
}

TEST_F(SyncResolverTest, GroupsByAlbumArtist) {
  Song cover = Find("Clair de lune");
  cover.set_artist("Isao Tomita");
  cover.set_albumartist("Claude Debussy");
  cover.set_album("Snowflakes Are Dancing");
  library_->AddOrUpdateSongs(SongList() << cover);

  EXPECT_EQ(QStringList{"Clair de lune"},
            Titles(Resolve(SyncSourceId::ForAlbum("Claude Debussy",
                                                  "Snowflakes Are Dancing"))));
  // But an artist is the songs' own.
  EXPECT_EQ(QStringList{"Clair de lune"},
            Titles(Resolve(SyncSourceId::ForArtist("Isao Tomita"))));
  EXPECT_EQ(SyncResolver::State::Gone,
            Resolve(SyncSourceId::ForArtist("Claude Debussy")).state);
}

TEST_F(SyncResolverTest, GoneSourcesAreGone) {
  EXPECT_EQ(SyncResolver::State::Gone,
            Resolve(SyncSourceId::ForSong(999)).state);
  EXPECT_EQ(SyncResolver::State::Gone,
            Resolve(SyncSourceId::ForAlbum("Pixies", "Trompe le Monde")).state);
  EXPECT_EQ(SyncResolver::State::Gone,
            Resolve(SyncSourceId::ForArtist("Breeders")).state);
  EXPECT_EQ(SyncResolver::State::Gone,
            Resolve(SyncSourceId::ForPlaylist(8)).state);
  EXPECT_EQ(SyncResolver::State::Gone, Resolve(SyncSourceId()).state);
}

TEST_F(SyncResolverTest, UnavailableSongsAreLeftOut) {
  const Song tame = Find("Tame");
  library_->MarkSongsUnavailable(SongList() << tame);

  EXPECT_EQ(SyncResolver::State::Gone,
            Resolve(SyncSourceId::ForSong(tame.id())).state);
  EXPECT_EQ((QStringList{"Debaser", "Wave of Mutilation"}),
            Titles(Resolve(SyncSourceId::ForAlbum("Pixies", "Doolittle"))));
}

TEST_F(SyncResolverTest, FindsAPlaylistInItsOrder) {
  playlist_ << Find("Tame") << Find("Clair de lune") << Find("Debaser");
  const SyncResolver::Result result = Resolve(SyncSourceId::ForPlaylist(7));
  EXPECT_EQ(SyncResolver::State::Ready, result.state);
  EXPECT_EQ((QStringList{"Tame", "Clair de lune", "Debaser"}), Titles(result));

  // An empty playlist is still there.
  playlist_.clear();
  EXPECT_EQ(SyncResolver::State::Ready,
            Resolve(SyncSourceId::ForPlaylist(7)).state);
}

TEST_F(SyncResolverTest, OtherSourcesAreNotSupportedYet) {
  EXPECT_EQ(SyncResolver::State::Unsupported,
            Resolve(SyncSourceId::ForSmartPlaylist("library", "0b6e")).state);
  EXPECT_EQ(SyncResolver::State::Unsupported,
            Resolve(SyncSourceId::ForInternetNode("plex", "42")).state);
}

}  // namespace
