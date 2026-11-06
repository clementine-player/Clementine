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

#include "networkremote/remotefiles.h"

#include <QDir>
#include <QFileInfo>
#include <QUrl>

#include "core/song.h"

namespace RemoteFiles {

bool IsWithin(const QDir& root, const QFileInfo& file) {
  // Relative to the root, a path outside it starts with "..", or stays
  // absolute when it's on another drive.
  const QString relative =
      root.relativeFilePath(QDir::cleanPath(file.absoluteFilePath()));
  return !relative.isEmpty() && relative != "." && relative != ".." &&
         !relative.startsWith("../") && !QDir::isAbsolutePath(relative);
}

bool MayAdd(const QUrl& url, const QString& files_root,
            const std::function<bool(const QUrl&)>& in_library) {
  if (url.scheme().isEmpty()) return false;
  if (!url.isLocalFile()) return true;

  if (!files_root.isEmpty()) {
    const QDir root(files_root);
    if (root.exists() && IsWithin(root, QFileInfo(url.toLocalFile())))
      return true;
  }
  return in_library(url);
}

bool IsSafeCover(const QString& art) {
  const QString lower = art.toLower();
  return art.isEmpty() || art == Song::kManuallyUnsetCover ||
         art == Song::kEmbeddedCover || lower.startsWith("http://") ||
         lower.startsWith("https://");
}

}  // namespace RemoteFiles
