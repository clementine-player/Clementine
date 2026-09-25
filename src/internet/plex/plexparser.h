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

#ifndef INTERNET_PLEX_PLEXPARSER_H_
#define INTERNET_PLEX_PLEXPARSER_H_

#include <QByteArray>
#include <QJsonDocument>
#include <QList>
#include <QString>
#include <QUrl>

#include "core/song.h"

// Pure functions that turn Plex JSON into Clementine types.  Nothing in here
// touches the network, so it can be unit tested with canned responses.

struct PlexConnection {
  QUrl uri;
  bool local = false;
  bool relay = false;
};

struct PlexServer {
  QString name;
  QString machine_id;
  QString access_token;
  QList<PlexConnection> connections;
};

struct PlexSection {
  QString key;
  QString title;
  // Changes whenever the section's contents change.  Taken from
  // contentChangedAt if the server reports it, otherwise the newer of
  // updatedAt and scannedAt.
  qint64 changed_at = 0;
};

struct PlexTracksPage {
  SongList songs;
  int size = 0;        // Number of tracks in this page.
  int total_size = 0;  // Number of tracks in the whole section.
};

// A song that differs between the local table and the server.
struct PlexDiff {
  SongList add;
  SongList update;  // Fetched songs carrying the existing row id.
  SongList remove;  // Existing songs.
};

// One event read from a server-sent-events stream.
struct PlexServerEvent {
  QString event;
  QByteArray data;
};

namespace PlexParser {

extern const char* kScheme;

// Servers from https://plex.tv/api/v2/resources.  Entries that don't provide
// "server" are skipped.
QList<PlexServer> ParseResources(const QJsonDocument& doc);

// Connections in the order they should be tried: local, then remote, then
// relay.  Order within each group is preserved.
QList<PlexConnection> OrderConnections(const QList<PlexConnection>& in);

// Round-trips a server's connections through the settings.
QByteArray SerializeConnections(const QList<PlexConnection>& connections);
QList<PlexConnection> ParseConnections(const QByteArray& json);

// Music ("artist" type) sections from /library/sections.
QList<PlexSection> ParseMusicSections(const QJsonDocument& doc);

// Tracks from /library/sections/<key>/all?type=10.  Artwork URLs are built
// against base_url and carry token.
PlexTracksPage ParseTracks(const QJsonDocument& doc, const QString& machine_id,
                           const QString& section_key, const QUrl& base_url,
                           const QString& token);

// plex://<machine_id>/<rating_key>?part=<part_key>&section=<section_key>
QUrl MakeSongUrl(const QString& machine_id, const QString& rating_key,
                 const QString& part_key, const QString& section_key);
QString MachineIdFromUrl(const QUrl& url);
QString RatingKeyFromUrl(const QUrl& url);
QString PartKeyFromUrl(const QUrl& url);
QString SectionKeyFromUrl(const QUrl& url);

// Works out what has to change in the local table so existing matches
// fetched.  Songs are matched by rating key.
PlexDiff ComputeDiff(const SongList& existing, const SongList& fetched);

}  // namespace PlexParser

// Splits a text/event-stream body into events.  Data may arrive in arbitrary
// chunks, so partial lines are kept until the rest arrives.
class PlexEventStreamParser {
 public:
  QList<PlexServerEvent> Feed(const QByteArray& chunk);

 private:
  QByteArray buffer_;
  QString event_;
  QByteArray data_;
};

#endif  // INTERNET_PLEX_PLEXPARSER_H_
