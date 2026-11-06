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

class QDir;
class QFileInfo;

// The network remote's Files tab shows what's under one root folder, and a
// client may add to the playlist or download only files from there.
namespace RemoteFiles {

// Whether `file` is within `root`. The file names a client sends are joined
// to a folder within the root, so a name with "..", or an absolute path,
// could otherwise name any file on the computer. Symbolic links within the
// root are followed, as the Files tab follows them.
bool IsWithin(const QDir& root, const QFileInfo& file);

}  // namespace RemoteFiles

#endif  // NETWORKREMOTE_REMOTEFILES_H
