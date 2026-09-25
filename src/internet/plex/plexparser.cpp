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

#include "plexparser.h"

#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>

#include "core/timeconstants.h"

namespace PlexParser {

const char* kScheme = "plex";

namespace {

const int kCoverArtSize = 600;

// Plex sends ids and timestamps as either strings or numbers depending on
// the endpoint, so accept both.
QString StringValue(const QJsonValue& value) {
  if (value.isString()) return value.toString();
  if (value.isDouble()) return QString::number(qint64(value.toDouble()));
  return QString();
}

qint64 IntValue(const QJsonValue& value) {
  if (value.isDouble()) return qint64(value.toDouble());
  if (value.isString()) return value.toString().toLongLong();
  return 0;
}

// The database stores zero or negative numbers as -1 ("unset"), so parse them
// the same way.  Otherwise a song never compares equal to its stored copy.
qint64 OrUnset(qint64 value) { return value > 0 ? value : -1; }

QUrl CoverArtUrl(const QString& thumb, const QUrl& base_url,
                 const QString& token) {
  QUrl url(base_url.toString() + "/photo/:/transcode");
  QUrlQuery query;
  query.addQueryItem("width", QString::number(kCoverArtSize));
  query.addQueryItem("height", QString::number(kCoverArtSize));
  query.addQueryItem("minSize", "1");
  query.addQueryItem("upscale", "1");
  query.addQueryItem("url",
                     QString::fromLatin1(QUrl::toPercentEncoding(thumb)));
  query.addQueryItem("X-Plex-Token", token);
  url.setQuery(query);
  return url;
}

Song ParseTrack(const QJsonObject& track, const QString& machine_id,
                const QString& section_key, const QUrl& base_url,
                const QString& token) {
  const QJsonArray media = track["Media"].toArray();
  if (media.isEmpty()) return Song();
  const QJsonObject first_media = media.first().toObject();
  const QJsonArray parts = first_media["Part"].toArray();
  if (parts.isEmpty()) return Song();
  const QJsonObject part = parts.first().toObject();

  const QString rating_key = StringValue(track["ratingKey"]);
  const QString part_key = part["key"].toString();
  if (rating_key.isEmpty() || part_key.isEmpty()) return Song();

  Song song;
  song.set_title(track["title"].toString());
  song.set_album(track["parentTitle"].toString());

  // grandparentTitle is the album artist.  originalTitle is only present when
  // the track artist differs from it, e.g. on compilations.
  const QString album_artist = track["grandparentTitle"].toString();
  const QString track_artist = track["originalTitle"].toString();
  song.set_albumartist(album_artist);
  song.set_artist(track_artist.isEmpty() ? album_artist : track_artist);

  song.set_track(OrUnset(IntValue(track["index"])));
  song.set_disc(OrUnset(IntValue(track["parentIndex"])));
  const qint64 year = IntValue(track["parentYear"]);
  song.set_year(OrUnset(year ? year : IntValue(track["year"])));
  song.set_length_nanosec(OrUnset(IntValue(track["duration"]) * kNsecPerMsec));
  song.set_bitrate(OrUnset(IntValue(first_media["bitrate"])));
  song.set_filesize(IntValue(part["size"]));
  song.set_ctime(IntValue(track["addedAt"]));
  song.set_mtime(IntValue(track["updatedAt"]));
  song.set_playcount(IntValue(track["viewCount"]));
  if (track["userRating"].toDouble() > 0) {
    song.set_rating(track["userRating"].toDouble() / 10.0);
  }

  QString thumb = track["parentThumb"].toString();
  if (thumb.isEmpty()) thumb = track["thumb"].toString();
  if (!thumb.isEmpty()) {
    song.set_art_automatic(CoverArtUrl(thumb, base_url, token).toString());
  }

  song.set_url(MakeSongUrl(machine_id, rating_key, part_key, section_key));
  song.set_basefilename(QFileInfo(part["file"].toString()).fileName());
  // The filetype is left as Type_Unknown so the tag editor stays away from
  // songs that live on the server.
  song.set_directory_id(0);
  return song;
}

// Fields that, if changed on the server, mean the local row needs updating.
bool SongDiffers(const Song& a, const Song& b) {
  return a.url() != b.url() || a.mtime() != b.mtime() ||
         a.title() != b.title() || a.artist() != b.artist() ||
         a.albumartist() != b.albumartist() || a.album() != b.album() ||
         a.track() != b.track() || a.disc() != b.disc() ||
         a.year() != b.year() || a.length_nanosec() != b.length_nanosec() ||
         a.playcount() != b.playcount() || a.rating() != b.rating() ||
         a.art_automatic() != b.art_automatic();
}

QList<PlexConnection> ParseConnectionArray(const QJsonArray& array) {
  QList<PlexConnection> ret;
  for (const QJsonValue& value : array) {
    const QJsonObject connection = value.toObject();
    PlexConnection conn;
    conn.uri = QUrl(connection["uri"].toString());
    conn.local = connection["local"].toBool();
    conn.relay = connection["relay"].toBool();
    if (conn.uri.isValid()) ret << conn;
  }
  return ret;
}

}  // namespace

QList<PlexServer> ParseResources(const QJsonDocument& doc) {
  QList<PlexServer> ret;
  for (const QJsonValue& value : doc.array()) {
    const QJsonObject resource = value.toObject();
    if (!resource["provides"].toString().split(',').contains("server")) {
      continue;
    }

    PlexServer server;
    server.name = resource["name"].toString();
    server.machine_id = resource["clientIdentifier"].toString();
    server.access_token = resource["accessToken"].toString();
    server.connections =
        ParseConnectionArray(resource["connections"].toArray());
    if (!server.machine_id.isEmpty()) ret << server;
  }
  return ret;
}

QList<PlexConnection> OrderConnections(const QList<PlexConnection>& in) {
  QList<PlexConnection> local, remote, relay;
  for (const PlexConnection& c : in) {
    if (c.relay) {
      relay << c;
    } else if (c.local) {
      local << c;
    } else {
      remote << c;
    }
  }
  return local + remote + relay;
}

QByteArray SerializeConnections(const QList<PlexConnection>& connections) {
  QJsonArray array;
  for (const PlexConnection& c : connections) {
    array.append(QJsonObject{
        {"uri", c.uri.toString()}, {"local", c.local}, {"relay", c.relay}});
  }
  return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

QList<PlexConnection> ParseConnections(const QByteArray& json) {
  return ParseConnectionArray(QJsonDocument::fromJson(json).array());
}

QList<PlexSection> ParseMusicSections(const QJsonDocument& doc) {
  QList<PlexSection> ret;
  const QJsonObject container = doc.object()["MediaContainer"].toObject();
  for (const QJsonValue& value : container["Directory"].toArray()) {
    const QJsonObject directory = value.toObject();
    if (directory["type"].toString() != "artist") continue;

    PlexSection section;
    section.key = StringValue(directory["key"]);
    section.title = directory["title"].toString();
    section.changed_at = IntValue(directory["contentChangedAt"]);
    if (!section.changed_at) {
      section.changed_at = qMax(IntValue(directory["updatedAt"]),
                                IntValue(directory["scannedAt"]));
    }
    if (!section.key.isEmpty()) ret << section;
  }
  return ret;
}

PlexTracksPage ParseTracks(const QJsonDocument& doc, const QString& machine_id,
                           const QString& section_key, const QUrl& base_url,
                           const QString& token) {
  PlexTracksPage page;
  const QJsonObject container = doc.object()["MediaContainer"].toObject();
  const QJsonArray metadata = container["Metadata"].toArray();
  page.size = metadata.size();
  page.total_size = container.contains("totalSize")
                        ? IntValue(container["totalSize"])
                        : page.size;

  for (const QJsonValue& value : metadata) {
    Song song =
        ParseTrack(value.toObject(), machine_id, section_key, base_url, token);
    if (song.url().isValid()) page.songs << song;
  }
  return page;
}

QUrl MakeSongUrl(const QString& machine_id, const QString& rating_key,
                 const QString& part_key, const QString& section_key) {
  QUrl url;
  url.setScheme(kScheme);
  url.setHost(machine_id);
  url.setPath("/" + rating_key);
  QUrlQuery query;
  query.addQueryItem("part",
                     QString::fromLatin1(QUrl::toPercentEncoding(part_key)));
  query.addQueryItem("section", section_key);
  url.setQuery(query);
  return url;
}

QString MachineIdFromUrl(const QUrl& url) { return url.host(); }

QString RatingKeyFromUrl(const QUrl& url) { return url.path().mid(1); }

QString PartKeyFromUrl(const QUrl& url) {
  return QUrlQuery(url).queryItemValue("part", QUrl::FullyDecoded);
}

QString SectionKeyFromUrl(const QUrl& url) {
  return QUrlQuery(url).queryItemValue("section");
}

PlexDiff ComputeDiff(const SongList& existing, const SongList& fetched) {
  QHash<QString, Song> existing_by_key;
  for (const Song& song : existing) {
    existing_by_key.insert(RatingKeyFromUrl(song.url()), song);
  }

  PlexDiff diff;
  for (const Song& song : fetched) {
    const QString key = RatingKeyFromUrl(song.url());
    auto it = existing_by_key.find(key);
    if (it == existing_by_key.end()) {
      diff.add << song;
      continue;
    }

    if (SongDiffers(*it, song)) {
      Song updated = song;
      updated.set_id(it->id());
      diff.update << updated;
    }
    existing_by_key.erase(it);
  }

  for (const Song& song : existing_by_key) diff.remove << song;
  return diff;
}

}  // namespace PlexParser

QList<PlexServerEvent> PlexEventStreamParser::Feed(const QByteArray& chunk) {
  QList<PlexServerEvent> ret;
  buffer_.append(chunk);

  int newline;
  while ((newline = buffer_.indexOf('\n')) != -1) {
    QByteArray line = buffer_.left(newline);
    buffer_.remove(0, newline + 1);
    if (line.endsWith('\r')) line.chop(1);

    if (line.isEmpty()) {
      // A blank line ends the event.
      if (!event_.isEmpty() || !data_.isEmpty()) {
        ret << PlexServerEvent{event_, data_};
      }
      event_.clear();
      data_.clear();
    } else if (line.startsWith("event:")) {
      event_ = QString::fromUtf8(line.mid(6).trimmed());
    } else if (line.startsWith("data:")) {
      if (!data_.isEmpty()) data_.append('\n');
      data_.append(line.mid(5).trimmed());
    }
    // Comments (":...") and other fields such as id: are ignored.
  }
  return ret;
}
