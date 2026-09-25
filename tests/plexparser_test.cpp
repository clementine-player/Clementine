/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "internet/plex/plexparser.h"

#include <QJsonDocument>
#include <QUrlQuery>

#include "core/timeconstants.h"
#include "gtest/gtest.h"
#include "internet/plex/plexauthenticator.h"
#include "internet/plex/plexnotificationlistener.h"
#include "test_utils.h"

namespace {

const QUrl kBaseUrl("https://10-0-0-5.abc123.plex.direct:32400");

QJsonDocument Json(const char* text) {
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(QByteArray(text), &error);
  EXPECT_EQ(QJsonParseError::NoError, error.error) << text;
  return doc;
}

const char* kResources = R"([
  {"name": "Living room", "provides": "server", "clientIdentifier": "abc123",
   "accessToken": "server-token",
   "connections": [
     {"uri": "https://1-2-3-4.abc123.plex.direct:32400", "local": false, "relay": false},
     {"uri": "https://relay.plex.direct:8443", "local": false, "relay": true},
     {"uri": "https://10-0-0-5.abc123.plex.direct:32400", "local": true, "relay": false}
   ]},
  {"name": "Phone", "provides": "client,player", "clientIdentifier": "phone"},
  {"name": "Both", "provides": "client,server", "clientIdentifier": "both",
   "connections": []}
])";

const char* kSections = R"({"MediaContainer": {"size": 3, "Directory": [
  {"key": "1", "type": "movie", "title": "Films", "contentChangedAt": 5},
  {"key": "3", "type": "artist", "title": "Music", "contentChangedAt": 1234,
   "updatedAt": 99},
  {"key": "7", "type": "artist", "title": "Old music", "updatedAt": 10,
   "scannedAt": 20}
]}})";

const char* kTracks = R"({"MediaContainer": {"size": 3, "totalSize": 1200,
  "offset": 0, "Metadata": [
  {"ratingKey": "101", "type": "track", "title": "Song One",
   "grandparentTitle": "The Band", "parentTitle": "First Album",
   "index": 1, "parentIndex": 2, "parentYear": 1999, "year": 2005,
   "duration": 215000, "addedAt": 1700000000, "updatedAt": 1700000500,
   "viewCount": 3, "userRating": 8.0, "parentThumb": "/library/metadata/100/thumb/1",
   "Media": [{"bitrate": 1011, "container": "flac", "Part": [
     {"key": "/library/parts/555/1700000000/file.flac",
      "file": "/music/The Band/First Album/01 Song One.flac", "size": 27000000}]}]},
  {"ratingKey": 102, "type": "track", "title": "Süß 日本",
   "grandparentTitle": "Various Artists", "originalTitle": "Guest Singer",
   "parentTitle": "Compilation", "index": 4, "year": 2010, "duration": 60000,
   "Media": [{"Part": [{"key": "/library/parts/556/1/file.mp3"}]}]},
  {"ratingKey": "103", "type": "track", "title": "No media"}
]}})";

Song MakeSong(int id, const QString& rating_key, const QString& title) {
  Song song;
  song.set_id(id);
  song.set_title(title);
  song.set_url(PlexParser::MakeSongUrl("abc123", rating_key,
                                       "/library/parts/1/2/file.flac", "3"));
  return song;
}

}  // namespace

TEST(PlexParserTest, ParsesServersOnly) {
  QList<PlexServer> servers = PlexParser::ParseResources(Json(kResources));
  ASSERT_EQ(2, servers.size());

  EXPECT_EQ("Living room", servers[0].name);
  EXPECT_EQ("abc123", servers[0].machine_id);
  EXPECT_EQ("server-token", servers[0].access_token);
  ASSERT_EQ(3, servers[0].connections.size());
  EXPECT_TRUE(servers[0].connections[1].relay);
  EXPECT_TRUE(servers[0].connections[2].local);

  EXPECT_EQ("both", servers[1].machine_id);
  EXPECT_TRUE(servers[1].connections.isEmpty());
}

TEST(PlexParserTest, OrdersLocalThenRemoteThenRelay) {
  QList<PlexConnection> ordered = PlexParser::OrderConnections(
      PlexParser::ParseResources(Json(kResources))[0].connections);
  ASSERT_EQ(3, ordered.size());
  EXPECT_EQ(QUrl("https://10-0-0-5.abc123.plex.direct:32400"), ordered[0].uri);
  EXPECT_EQ(QUrl("https://1-2-3-4.abc123.plex.direct:32400"), ordered[1].uri);
  EXPECT_EQ(QUrl("https://relay.plex.direct:8443"), ordered[2].uri);
}

TEST(PlexParserTest, ConnectionsRoundTrip) {
  QList<PlexConnection> connections =
      PlexParser::ParseResources(Json(kResources))[0].connections;
  QList<PlexConnection> round_tripped = PlexParser::ParseConnections(
      PlexParser::SerializeConnections(connections));
  ASSERT_EQ(connections.size(), round_tripped.size());
  for (int i = 0; i < connections.size(); ++i) {
    EXPECT_EQ(connections[i].uri, round_tripped[i].uri);
    EXPECT_EQ(connections[i].local, round_tripped[i].local);
    EXPECT_EQ(connections[i].relay, round_tripped[i].relay);
  }
}

TEST(PlexParserTest, ParsesMusicSections) {
  QList<PlexSection> sections = PlexParser::ParseMusicSections(Json(kSections));
  ASSERT_EQ(2, sections.size());
  EXPECT_EQ("3", sections[0].key);
  EXPECT_EQ("Music", sections[0].title);
  EXPECT_EQ(1234, sections[0].changed_at);
  // Without contentChangedAt, the newer of updatedAt and scannedAt.
  EXPECT_EQ("7", sections[1].key);
  EXPECT_EQ(20, sections[1].changed_at);
}

TEST(PlexParserTest, ParsesTracks) {
  PlexTracksPage page = PlexParser::ParseTracks(Json(kTracks), "abc123", "3",
                                                kBaseUrl, "server-token");
  EXPECT_EQ(3, page.size);
  EXPECT_EQ(1200, page.total_size);
  // The track without media is dropped.
  ASSERT_EQ(2, page.songs.size());

  const Song& one = page.songs[0];
  EXPECT_EQ("Song One", one.title());
  EXPECT_EQ("The Band", one.artist());
  EXPECT_EQ("The Band", one.albumartist());
  EXPECT_EQ("First Album", one.album());
  EXPECT_EQ(1, one.track());
  EXPECT_EQ(2, one.disc());
  EXPECT_EQ(1999, one.year());  // The album's year wins.
  EXPECT_EQ(215000 * kNsecPerMsec, one.length_nanosec());
  EXPECT_EQ(1011, one.bitrate());
  EXPECT_EQ(27000000, one.filesize());
  EXPECT_EQ(1700000000, one.ctime());
  EXPECT_EQ(1700000500, one.mtime());
  EXPECT_EQ(3, one.playcount());
  EXPECT_FLOAT_EQ(0.8, one.rating());
  EXPECT_EQ("01 Song One.flac", one.basefilename());
  EXPECT_EQ(Song::Type_Unknown, one.filetype());

  EXPECT_EQ("plex", one.url().scheme());
  EXPECT_EQ("abc123", PlexParser::MachineIdFromUrl(one.url()));
  EXPECT_EQ("101", PlexParser::RatingKeyFromUrl(one.url()));
  EXPECT_EQ("3", PlexParser::SectionKeyFromUrl(one.url()));
  EXPECT_EQ("/library/parts/555/1700000000/file.flac",
            PlexParser::PartKeyFromUrl(one.url()));

  QUrl art(one.art_automatic());
  EXPECT_EQ(kBaseUrl.host(), art.host());
  EXPECT_EQ("/photo/:/transcode", art.path());
  QUrlQuery art_query(art);
  EXPECT_EQ("/library/metadata/100/thumb/1",
            art_query.queryItemValue("url", QUrl::FullyDecoded));
  EXPECT_EQ("server-token", art_query.queryItemValue("X-Plex-Token"));

  const Song& two = page.songs[1];
  EXPECT_EQ("102", PlexParser::RatingKeyFromUrl(two.url()));
  EXPECT_EQ(QString::fromUtf8("Süß 日本"), two.title());
  EXPECT_EQ("Guest Singer", two.artist());
  EXPECT_EQ("Various Artists", two.albumartist());
  EXPECT_EQ(2010, two.year());
  // Missing numbers are -1, as they come back from the database.
  EXPECT_EQ(-1, two.disc());
  EXPECT_EQ(-1, two.bitrate());
  EXPECT_EQ(-1, two.rating());
  EXPECT_TRUE(two.art_automatic().isEmpty());
}

TEST(PlexParserTest, SongUrlSurvivesAStringRoundTrip) {
  // Song URLs are stored in the database as strings.
  const QUrl url = PlexParser::MakeSongUrl(
      "abc123", "101", "/library/parts/555/1/file name?.flac", "3");
  const QUrl reloaded(url.toString());
  EXPECT_EQ("/library/parts/555/1/file name?.flac",
            PlexParser::PartKeyFromUrl(reloaded));
  EXPECT_EQ("101", PlexParser::RatingKeyFromUrl(reloaded));
  EXPECT_EQ("3", PlexParser::SectionKeyFromUrl(reloaded));
}

TEST(PlexParserTest, DiffAddsUpdatesAndRemoves) {
  const SongList existing = SongList() << MakeSong(1, "10", "Unchanged")
                                       << MakeSong(2, "11", "Old title")
                                       << MakeSong(3, "12", "Deleted");
  const SongList fetched = SongList() << MakeSong(-1, "10", "Unchanged")
                                      << MakeSong(-1, "11", "New title")
                                      << MakeSong(-1, "13", "Added");

  PlexDiff diff = PlexParser::ComputeDiff(existing, fetched);

  ASSERT_EQ(1, diff.add.size());
  EXPECT_EQ("Added", diff.add[0].title());
  EXPECT_EQ(-1, diff.add[0].id());

  ASSERT_EQ(1, diff.update.size());
  EXPECT_EQ("New title", diff.update[0].title());
  EXPECT_EQ(2, diff.update[0].id());  // Keeps the existing row.

  ASSERT_EQ(1, diff.remove.size());
  EXPECT_EQ(3, diff.remove[0].id());
}

TEST(PlexParserTest, DiffOfIdenticalListsIsEmpty) {
  const SongList songs = SongList()
                         << MakeSong(1, "10", "A") << MakeSong(2, "11", "B");
  SongList fetched;
  for (Song song : songs) {
    song.set_id(-1);
    fetched << song;
  }

  PlexDiff diff = PlexParser::ComputeDiff(songs, fetched);
  EXPECT_TRUE(diff.add.isEmpty());
  EXPECT_TRUE(diff.update.isEmpty());
  EXPECT_TRUE(diff.remove.isEmpty());
}

TEST(PlexEventStreamParserTest, HandlesChunkedEvents) {
  PlexEventStreamParser parser;

  EXPECT_TRUE(parser.Feed("event: ping\r\ndata: {}\r\n").isEmpty());
  QList<PlexServerEvent> events = parser.Feed("\r\nevent: timel");
  ASSERT_EQ(1, events.size());
  EXPECT_EQ("ping", events[0].event);
  EXPECT_EQ("{}", QString::fromUtf8(events[0].data));

  events = parser.Feed("ine\ndata: {\"a\":\ndata: 1}\n\n: comment\n\n");
  ASSERT_EQ(1, events.size());
  EXPECT_EQ("timeline", events[0].event);
  EXPECT_EQ("{\"a\":\n1}", QString::fromUtf8(events[0].data));
}

TEST(PlexNotificationListenerTest, RecognisesLibraryChanges) {
  QStringList ids;

  // A music track finished processing.
  EXPECT_TRUE(PlexNotificationListener::IsLibraryChange(
      {"timeline",
       R"({"TimelineEntry": [{"itemID": 101, "type": 10, "state": 5},
                             {"itemID": "102", "type": 10, "state": 9}]})"},
      &ids));
  EXPECT_EQ(QStringList() << "101" << "102", ids);

  // Video items and in-progress states are ignored.
  ids.clear();
  EXPECT_FALSE(PlexNotificationListener::IsLibraryChange(
      {"timeline",
       R"({"TimelineEntry": [{"itemID": 5, "type": 1, "state": 5},
                             {"itemID": 6, "type": 10, "state": 1}]})"},
      &ids));
  EXPECT_TRUE(ids.isEmpty());

  // A library scan ending, in the websocket's container form.
  EXPECT_TRUE(PlexNotificationListener::IsLibraryChange(
      {"activity",
       R"({"NotificationContainer": {"ActivityNotification": {
            "event": "ended",
            "Activity": {"type": "library.update.section"}}}})"},
      &ids));
  EXPECT_TRUE(ids.isEmpty());

  EXPECT_FALSE(PlexNotificationListener::IsLibraryChange(
      {"activity",
       R"({"ActivityNotification": {"event": "started",
            "Activity": {"type": "library.update.section"}}})"},
      &ids));
  EXPECT_FALSE(PlexNotificationListener::IsLibraryChange({"ping", ""}, &ids));
}

TEST(PlexAuthenticatorTest, BuildsAuthUrl) {
  const QUrl url = PlexAuthenticator::AuthUrl("client-1", "CODE",
                                              QUrl("http://localhost:4567/"));
  EXPECT_EQ("app.plex.tv", url.host());
  EXPECT_EQ("/auth", url.path());

  // app.plex.tv reads a query string from the fragment.
  QUrlQuery params(url.fragment(QUrl::FullyEncoded).mid(1));
  EXPECT_EQ("client-1", params.queryItemValue("clientID"));
  EXPECT_EQ("CODE", params.queryItemValue("code"));
  EXPECT_EQ("http://localhost:4567/",
            params.queryItemValue("forwardUrl", QUrl::FullyDecoded));
  // QUrlQuery matches keys in their encoded form.
  EXPECT_EQ("Clementine",
            params.queryItemValue("context%5Bdevice%5D%5Bproduct%5D"));
}
