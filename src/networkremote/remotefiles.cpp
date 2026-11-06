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
#include "library/librarybackend.h"

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

void DropUnsafeCovers(Song* song) {
  if (!IsSafeCover(song->art_automatic())) song->set_art_automatic(QString());
  if (!IsSafeCover(song->art_manual())) song->set_art_manual(QString());
}

void TrustCovers(LibraryBackendInterface* library, Song* song) {
  if (song->url().isLocalFile()) {
    const SongList library_songs = library->GetSongsByUrl(song->url());
    if (!library_songs.isEmpty()) {
      // A file can hold several songs, as a cue sheet's do.
      const Song* found = &library_songs.first();
      for (const Song& library_song : library_songs) {
        if (library_song.beginning_nanosec() == song->beginning_nanosec())
          found = &library_song;
      }
      song->set_art_automatic(found->art_automatic());
      song->set_art_manual(found->art_manual());
      return;
    }
  }
  DropUnsafeCovers(song);
}

bool AcceptSong(LibraryBackendInterface* library, const QString& files_root,
                Song* song) {
  if (!MayAdd(song->url(), files_root, [library](const QUrl& file) {
        return !library->GetSongsByUrl(file).isEmpty();
      }))
    return false;
  TrustCovers(library, song);
  return true;
}

}  // namespace RemoteFiles
