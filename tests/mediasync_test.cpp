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

#include "networkremote/sync/mediasync.h"

#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>
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
#include "networkremote/streaming/mediahttpserver.h"
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
std::ostream& operator<<(std::ostream& stream, const QByteArray& bytes) {
  return stream << bytes.toStdString();
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
    song.set_filetype(Song::Type_Flac);
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

class MediaSyncTest : public SyncResolverTest {
 protected:
  void SetUp() override {
    SyncResolverTest::SetUp();
    MediaSync::Hooks hooks;
    hooks.playlists = playlists_;
    hooks.downloads_allowed = [this]() { return downloads_allowed_; };
    sync_.reset(new MediaSync(library_.get(), hooks, items_));
    sent_.reset(
        new QSignalSpy(sync_.get(), SIGNAL(SendToClient(int, QByteArray))));
  }

  cpb::remote::ResponseSyncManifest Manifest(const SyncSourceId& source,
                                             int client = 1,
                                             const QStringList& formats = {
                                                 "audio/flac"}) {
    cpb::remote::Message msg;
    msg.set_type(cpb::remote::REQUEST_SYNC_MANIFEST);
    cpb::remote::RequestSyncManifest* req = msg.mutable_request_sync_manifest();
    req->set_source_id(source.ToString().toStdString());
    for (const QString& format : formats) {
      req->add_supported_formats()->set_mime_type(format.toStdString());
    }
    req->set_limit_items(limit_items_);
    req->set_limit_bytes(limit_bytes_);
    return Send(msg, client);
  }

  cpb::remote::ResponseSyncManifest Send(const cpb::remote::Message& msg,
                                         int client = 1) {
    const std::string data = msg.SerializeAsString();
    sync_->HandleMessage(client, peer_, QByteArray(data.data(), data.size()));
    cpb::remote::Message reply;
    EXPECT_FALSE(sent_->isEmpty());
    if (sent_->isEmpty()) return reply.response_sync_manifest();
    EXPECT_EQ(client, sent_->last()[0].toInt());
    const QByteArray out = sent_->last()[1].toByteArray();
    reply.ParseFromArray(out.constData(), out.size());
    EXPECT_EQ(cpb::remote::SYNC_MANIFEST, reply.type());
    return reply.response_sync_manifest();
  }

  // Finds the item a manifest's URL stands for.
  bool Fetch(const std::string& url, StreamItem* item, QHostAddress* peer) {
    const QStringList parts = QString::fromStdString(url).split('/');
    if (parts.size() != 4 || parts[1] != "d") return false;
    return items_->Find(parts[2].toLatin1(), parts[3].toInt(), item, peer);
  }

  std::shared_ptr<StreamItemTable> items_ = std::make_shared<StreamItemTable>();
  std::unique_ptr<MediaSync> sync_;
  std::unique_ptr<QSignalSpy> sent_;
  QString peer_ = "10.0.0.2";
  bool downloads_allowed_ = true;
  int limit_items_ = 0;
  qint64 limit_bytes_ = 0;
};

TEST_F(MediaSyncTest, ListsAnAlbumsSongsToDownload) {
  const SyncSourceId source = SyncSourceId::ForAlbum("Pixies", "Doolittle");
  const cpb::remote::ResponseSyncManifest manifest = Manifest(source);
  EXPECT_EQ(source.ToString().toStdString(), manifest.source_id());
  EXPECT_EQ(cpb::remote::SYNC_MANIFEST_STATE_READY, manifest.state());
  EXPECT_EQ(0, manifest.skipped());
  ASSERT_EQ(3, manifest.items_size());

  const cpb::remote::SyncItem& debaser = manifest.items(0);
  const Song song = Find("Debaser");
  EXPECT_EQ(SyncSong::ItemKey(song).toStdString(), debaser.item_key());
  EXPECT_EQ(SyncSong::ItemVersion(song).toStdString(), debaser.version());
  EXPECT_EQ("Debaser", debaser.song().title());
  EXPECT_EQ(cpb::remote::DOWNLOADABILITY_DOWNLOADABLE,
            debaser.song().downloadable());
  EXPECT_EQ("audio/flac", debaser.mime_type());
  EXPECT_EQ(1000, debaser.size_bytes());
  EXPECT_EQ(cpb::remote::SYNC_DELIVERY_ORIGINAL, debaser.delivery());

  StreamItem item;
  QHostAddress peer;
  ASSERT_TRUE(Fetch(debaser.url(), &item, &peer));
  EXPECT_EQ(StreamPlan::Direct, item.plan.mode);
  EXPECT_EQ(QUrl::fromLocalFile("/music/Debaser.flac"), item.req.MediaUrl());
  EXPECT_EQ(QHostAddress("10.0.0.2"), peer);
}

TEST_F(MediaSyncTest, SkipsWhatTheRemoteCantPlay) {
  // Converting comes later: for now, only files the remote plays as they are.
  const cpb::remote::ResponseSyncManifest manifest = Manifest(
      SyncSourceId::ForAlbum("Pixies", "Doolittle"), 1, {"audio/mpeg"});
  EXPECT_EQ(cpb::remote::SYNC_MANIFEST_STATE_READY, manifest.state());
  EXPECT_EQ(0, manifest.items_size());
  EXPECT_EQ(3, manifest.skipped());
}

TEST_F(MediaSyncTest, SkipsWhatCantBeDownloaded) {
  Song stream;
  stream.Init("Groove Salad", "SomaFM", "", -1);
  stream.set_url(QUrl("http://ice.somafm.com/groovesalad"));
  stream.set_filetype(Song::Type_Stream);
  playlist_ << Find("Tame") << stream;

  const cpb::remote::ResponseSyncManifest manifest =
      Manifest(SyncSourceId::ForPlaylist(7));
  ASSERT_EQ(1, manifest.items_size());
  EXPECT_EQ("Tame", manifest.items(0).song().title());
  EXPECT_EQ(1, manifest.skipped());
}

TEST_F(MediaSyncTest, StopsAtTheLimits) {
  limit_items_ = 2;
  EXPECT_EQ(2, Manifest(SyncSourceId::ForArtist("Pixies")).items_size());

  limit_items_ = 0;
  limit_bytes_ = 2500;
  EXPECT_EQ(2, Manifest(SyncSourceId::ForArtist("Pixies")).items_size());
}

TEST_F(MediaSyncTest, ASongInTwoManifestsHasOneUrl) {
  const std::string album =
      Manifest(SyncSourceId::ForAlbum("Pixies", "Doolittle")).items(0).url();
  const std::string song =
      Manifest(SyncSourceId::ForSong(Find("Debaser").id())).items(0).url();
  EXPECT_EQ(album, song);

  // And the first manifest's songs can still be fetched.
  StreamItem item;
  QHostAddress peer;
  EXPECT_TRUE(Fetch(
      Manifest(SyncSourceId::ForAlbum("Pixies", "Doolittle")).items(2).url(),
      &item, &peer));
}

TEST_F(MediaSyncTest, EachRemoteHasItsOwnUrls) {
  const SyncSourceId source = SyncSourceId::ForSong(Find("Debaser").id());
  const std::string first = Manifest(source, 1).items(0).url();
  const std::string second = Manifest(source, 2).items(0).url();
  EXPECT_NE(first, second);

  // Until it goes.
  sync_->ClientDisconnected(1);
  StreamItem item;
  QHostAddress peer;
  EXPECT_FALSE(Fetch(first, &item, &peer));
  EXPECT_TRUE(Fetch(second, &item, &peer));
}

TEST_F(MediaSyncTest, SaysWhenASourceIsGone) {
  const cpb::remote::ResponseSyncManifest manifest =
      Manifest(SyncSourceId::ForAlbum("Pixies", "Trompe le Monde"));
  EXPECT_EQ(cpb::remote::SYNC_MANIFEST_STATE_GONE, manifest.state());
  EXPECT_EQ(0, manifest.items_size());
}

TEST_F(MediaSyncTest, KeepsRulesItCantSyncYet) {
  const cpb::remote::ResponseSyncManifest manifest =
      Manifest(SyncSourceId::ForSmartPlaylist("library", "0b6e"));
  EXPECT_EQ(cpb::remote::SYNC_MANIFEST_STATE_NEEDS_SETUP, manifest.state());
  EXPECT_FALSE(manifest.message().empty());
}

TEST_F(MediaSyncTest, NeedsDownloadsAllowed) {
  downloads_allowed_ = false;
  const cpb::remote::ResponseSyncManifest manifest =
      Manifest(SyncSourceId::ForAlbum("Pixies", "Doolittle"));
  EXPECT_EQ(cpb::remote::SYNC_MANIFEST_STATE_NEEDS_SETUP, manifest.state());
  EXPECT_FALSE(manifest.message().empty());
  EXPECT_EQ(0, manifest.items_size());
}

// A remote downloading a song it was given in a manifest, over HTTP.
class SyncDownloadTest : public MediaSyncTest {
 protected:
  void SetUp() override {
    MediaSyncTest::SetUp();
    peer_ = "127.0.0.1";

    ASSERT_TRUE(dir_.isValid());
    const QString path = dir_.filePath("Debaser.flac");
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(kContents);
    file.close();

    Song song = Find("Debaser");
    song.set_url(QUrl::fromLocalFile(path));
    song.set_filesize(kContents.size());
    library_->AddOrUpdateSongs(SongList() << song);

    http_.SetSyncItems(sync_->items());
    ASSERT_TRUE(server_.listen(QHostAddress::LocalHost));
    QObject::connect(&server_, &QTcpServer::newConnection, [this]() {
      while (QTcpSocket* socket = server_.nextPendingConnection()) {
        http_.HandleConnection(socket);
      }
    });

    item_ = Manifest(SyncSourceId::ForSong(song.id())).items(0);
  }

  // The whole response: head and body.
  QByteArray Get(const std::string& path, const QByteArray& headers = "") {
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, server_.serverPort());
    EXPECT_TRUE(socket.waitForConnected(2000));
    socket.write("GET " + QByteArray::fromStdString(path) +
                 " HTTP/1.1\r\nHost: localhost\r\n" + headers + "\r\n");
    QByteArray response;
    EXPECT_TRUE(QTest::qWaitFor(
        [&]() {
          response += socket.readAll();
          return socket.state() == QAbstractSocket::UnconnectedState;
        },
        5000));
    return response + socket.readAll();
  }

  static QByteArray Body(const QByteArray& response) {
    return response.mid(response.indexOf("\r\n\r\n") + 4);
  }

  QByteArray ETag() const {
    return '"' +
           QByteArray::fromStdString(item_.item_key() + ':' + item_.version()) +
           '"';
  }

  static const QByteArray kContents;
  QTemporaryDir dir_;
  MediaHttpServer http_{std::make_shared<StreamItemTable>()};
  QTcpServer server_;
  cpb::remote::SyncItem item_;
};

const QByteArray SyncDownloadTest::kContents = "fLaC and then the song";

TEST_F(SyncDownloadTest, DownloadsTheOriginalFile) {
  EXPECT_EQ(kContents.size(), item_.size_bytes());
  const QByteArray response = Get(item_.url());
  EXPECT_TRUE(response.startsWith("HTTP/1.1 200 OK\r\n")) << response.data();
  EXPECT_TRUE(response.contains("Content-Type: audio/flac\r\n"));
  EXPECT_TRUE(response.contains("ETag: " + ETag() + "\r\n")) << response.data();
  EXPECT_EQ(kContents, Body(response));
}

TEST_F(SyncDownloadTest, ResumesFromTheSameFile) {
  const QByteArray resumed =
      Get(item_.url(), "Range: bytes=5-\r\nIf-Range: " + ETag() + "\r\n");
  EXPECT_TRUE(resumed.startsWith("HTTP/1.1 206")) << resumed.data();
  EXPECT_EQ(kContents.mid(5), Body(resumed));

  // A file that's changed since starts over.
  const QByteArray changed =
      Get(item_.url(), "Range: bytes=5-\r\nIf-Range: \"library/1:0-0\"\r\n");
  EXPECT_TRUE(changed.startsWith("HTTP/1.1 200 OK")) << changed.data();
  EXPECT_EQ(kContents, Body(changed));
}

TEST_F(SyncDownloadTest, OnlyWhatWasOfferedCanBeDownloaded) {
  const QStringList parts = QString::fromStdString(item_.url()).split('/');
  const std::string token = parts[2].toStdString();
  const std::string id = parts[3].toStdString();
  // Another item, another token, or as a renderer's URL.
  EXPECT_TRUE(Get("/d/" + token + "/99").startsWith("HTTP/1.1 404"));
  EXPECT_TRUE(Get("/d/0123456789abcdef/" + id).startsWith("HTTP/1.1 404"));
  EXPECT_TRUE(Get("/s/" + token + "/" + id).startsWith("HTTP/1.1 404"));

  // Nor once the remote has gone.
  sync_->ClientDisconnected(1);
  EXPECT_TRUE(Get(item_.url()).startsWith("HTTP/1.1 404"));
}

TEST_F(SyncDownloadTest, OnlyTheRemoteItWasGivenToCanDownload) {
  peer_ = "10.0.0.9";
  const cpb::remote::SyncItem elsewhere =
      Manifest(SyncSourceId::ForSong(Find("Debaser").id()), 2).items(0);
  EXPECT_TRUE(Get(elsewhere.url()).startsWith("HTTP/1.1 403"));
}

}  // namespace
