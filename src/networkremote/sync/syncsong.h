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
#ifndef NETWORKREMOTE_SYNC_SYNCSONG_H_
#define NETWORKREMOTE_SYNC_SYNCSONG_H_

#include <QString>

#include "core/urlhandler.h"

class Song;

// What syncing to a network remote needs to know about one song.
namespace SyncSong {

// Whether the song can be downloaded. Anything with an end can be, from
// whichever service: only streams, and songs |handler| refuses, can't.
// |handler| is the UrlHandler for the song's scheme, or null for none.
UrlHandler::Downloadability Downloadability(const Song& song,
                                            const UrlHandler* handler);

// The song's identity across manifests: "library/<id>" for a library song,
// otherwise its percent-encoded URL. Empty for songs that can't be synced.
QString ItemKey(const Song& song);

// Changes when the bytes a remote would get change: when the file changes,
// or when it's to be converted with another |preset| (empty for the
// original file).
QString ItemVersion(const Song& song, const QString& preset = QString());

}  // namespace SyncSong

#endif  // NETWORKREMOTE_SYNC_SYNCSONG_H_
