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
#include "syncsourceid.h"

#include <QUrl>

namespace {

struct Prefix {
  SyncSourceId::Kind kind;
  const char* prefix;
  // How many parts follow the prefix. An id counts as one.
  int part_count;
  bool numeric;
};

const Prefix kPrefixes[] = {
    {SyncSourceId::Kind::Song, "library/song", 1, true},
    {SyncSourceId::Kind::Album, "library/album", 2, false},
    {SyncSourceId::Kind::Artist, "library/artist", 1, false},
    {SyncSourceId::Kind::Playlist, "playlist", 1, true},
    {SyncSourceId::Kind::SmartPlaylist, "smart", 2, false},
    {SyncSourceId::Kind::InternetNode, "browse", 2, false},
};

const Prefix* PrefixFor(SyncSourceId::Kind kind) {
  for (const Prefix& prefix : kPrefixes) {
    if (prefix.kind == kind) return &prefix;
  }
  return nullptr;
}

QString Encode(const QString& part) {
  return QString::fromLatin1(QUrl::toPercentEncoding(part));
}

QString Decode(const QString& part) {
  return QUrl::fromPercentEncoding(part.toLatin1());
}

}  // namespace

SyncSourceId SyncSourceId::ForSong(int song_id) {
  if (song_id < 0) return SyncSourceId();
  return SyncSourceId(Kind::Song, song_id, {});
}

SyncSourceId SyncSourceId::ForAlbum(const QString& album_artist,
                                    const QString& album) {
  // A song with no album artist is filed under its artist, so an album
  // always has one of them.
  if (album_artist.isEmpty() || album.isEmpty()) return SyncSourceId();
  return SyncSourceId(Kind::Album, -1, {album_artist, album});
}

SyncSourceId SyncSourceId::ForArtist(const QString& artist) {
  if (artist.isEmpty()) return SyncSourceId();
  return SyncSourceId(Kind::Artist, -1, {artist});
}

SyncSourceId SyncSourceId::ForPlaylist(int playlist_id) {
  if (playlist_id < 0) return SyncSourceId();
  return SyncSourceId(Kind::Playlist, playlist_id, {});
}

SyncSourceId SyncSourceId::ForSmartPlaylist(const QString& backend,
                                            const QString& uuid) {
  if (backend.isEmpty() || uuid.isEmpty()) return SyncSourceId();
  return SyncSourceId(Kind::SmartPlaylist, -1, {backend, uuid});
}

SyncSourceId SyncSourceId::ForInternetNode(const QString& service,
                                           const QString& key) {
  if (service.isEmpty() || key.isEmpty()) return SyncSourceId();
  return SyncSourceId(Kind::InternetNode, -1, {service, key});
}

SyncSourceId SyncSourceId::Parse(const QString& source_id) {
  for (const Prefix& prefix : kPrefixes) {
    const QString start = QString::fromLatin1(prefix.prefix) + '/';
    if (!source_id.startsWith(start)) continue;

    const QStringList encoded = source_id.mid(start.size()).split('/');
    if (encoded.size() != prefix.part_count) return SyncSourceId();

    QStringList parts;
    for (const QString& part : encoded) {
      if (part.isEmpty()) return SyncSourceId();
      parts << Decode(part);
    }

    if (prefix.numeric) {
      bool ok = false;
      const int id = parts[0].toInt(&ok);
      // Only the form ToString writes, so each source has one id.
      if (!ok || id < 0 || QString::number(id) != parts[0]) {
        return SyncSourceId();
      }
      return SyncSourceId(prefix.kind, id, {});
    }
    return SyncSourceId(prefix.kind, -1, parts);
  }
  return SyncSourceId();
}

QString SyncSourceId::ToString() const {
  const Prefix* prefix = PrefixFor(kind_);
  if (!prefix) return QString();

  QString ret = QString::fromLatin1(prefix->prefix);
  if (prefix->numeric) return ret + '/' + QString::number(id_);
  for (const QString& part : parts_) ret += '/' + Encode(part);
  return ret;
}
