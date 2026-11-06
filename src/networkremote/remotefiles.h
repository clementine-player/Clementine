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

#ifndef NETWORKREMOTE_REMOTEFILES_H
#define NETWORKREMOTE_REMOTEFILES_H

#include <functional>

class QDir;
class QFileInfo;
class QString;
class QUrl;

// The network remote's Files tab shows what's under one root folder, and a
// client may add to the playlist or download only files from there.
namespace RemoteFiles {

// Whether `file` is within `root`. The file names a client sends are joined
// to a folder within the root, so a name with "..", or an absolute path,
// could otherwise name any file on the computer. Symbolic links within the
// root are followed, as the Files tab follows them.
bool IsWithin(const QDir& root, const QFileInfo& file);

// Whether a client may add `url` to a playlist. A file on this computer must
// be in the library, or within `files_root`, the Files tab's root folder
// (empty when the Files tab is off): Clementine plays, streams and sends to
// remotes whatever is on its playlists, so a client that could add any file
// could read any file. `in_library` says whether a file's URL is in the
// library. URLs that aren't files, such as streams, may be added. A URL
// without a scheme is refused, since it would be taken as a path.
bool MayAdd(const QUrl& url, const QString& files_root,
            const std::function<bool(const QUrl&)>& in_library);

// Whether a song's cover, as a client describes it, may be loaded: none, the
// image in the song's own file, or one on the web. Clementine loads any
// other value as a file and sends the image to remotes, so a client could
// otherwise read any image on the computer.
bool IsSafeCover(const QString& art);

}  // namespace RemoteFiles

#endif  // NETWORKREMOTE_REMOTEFILES_H
