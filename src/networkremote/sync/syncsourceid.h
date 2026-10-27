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
#ifndef NETWORKREMOTE_SYNC_SYNCSOURCEID_H_
#define NETWORKREMOTE_SYNC_SYNCSOURCEID_H_

#include <QString>
#include <QStringList>

// Names something a remote's sync rule points at, in a form that stays the
// same across connections and restarts: "library/album/<albumartist>/<album>",
// "playlist/<id>", "smart/<backend>/<uuid>", "browse/<service>/<key>". Each
// part after the first ones is percent-encoded, so names may hold slashes.
class SyncSourceId {
 public:
  enum class Kind {
    Invalid,
    Song,
    Album,
    Artist,
    Playlist,
    SmartPlaylist,
    InternetNode,
  };

  SyncSourceId() = default;

  static SyncSourceId ForSong(int song_id);
  static SyncSourceId ForAlbum(const QString& album_artist,
                               const QString& album);
  static SyncSourceId ForArtist(const QString& artist);
  static SyncSourceId ForPlaylist(int playlist_id);
  static SyncSourceId ForSmartPlaylist(const QString& backend,
                                       const QString& uuid);
  static SyncSourceId ForInternetNode(const QString& service,
                                      const QString& key);

  // An Invalid id for anything that isn't one of the forms above.
  static SyncSourceId Parse(const QString& source_id);

  Kind kind() const { return kind_; }
  bool is_valid() const { return kind_ != Kind::Invalid; }

  // Song and Playlist: the song or playlist id.
  int id() const { return id_; }
  // The other kinds' parts, in the order the For* functions take them.
  const QStringList& parts() const { return parts_; }

  QString ToString() const;

  bool operator==(const SyncSourceId& other) const {
    return kind_ == other.kind_ && id_ == other.id_ && parts_ == other.parts_;
  }
  bool operator!=(const SyncSourceId& other) const { return !(*this == other); }

 private:
  SyncSourceId(Kind kind, int id, const QStringList& parts)
      : kind_(kind), id_(id), parts_(parts) {}

  Kind kind_ = Kind::Invalid;
  int id_ = -1;
  QStringList parts_;
};

#endif  // NETWORKREMOTE_SYNC_SYNCSOURCEID_H_
