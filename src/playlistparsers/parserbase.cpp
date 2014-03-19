/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "parserbase.h"
#include "core/tagreaderclient.h"
#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "library/sqlrow.h"

#include <QUrl>

ParserBase::ParserBase(LibraryBackendInterface* library, QObject* parent)
    : QObject(parent), library_(library) {}

void ParserBase::LoadSong(const QString& filename_or_url, qint64 beginning,
                          const QDir& dir, Song* song) const {
  if (filename_or_url.isEmpty()) {
    return;
  }

  QString filename = filename_or_url;

  if (filename_or_url.contains(QRegExp("^[a-z]{2,}:"))) {
    QUrl url(filename_or_url);
    if (url.scheme() == "file") {
      filename = url.toLocalFile();
    } else {
      song->set_url(QUrl::fromUserInput(filename_or_url));
      song->set_filetype(Song::Type_Stream);
      song->set_valid(true);
      return;
    }
  }

  // Convert native separators for Windows paths
  filename = QDir::fromNativeSeparators(filename);

  // Make the path absolute
  if (!QDir::isAbsolutePath(filename)) {
    filename = dir.absoluteFilePath(filename);
  }

  // Use the canonical path
  if (QFile::exists(filename)) {
    filename = QFileInfo(filename).canonicalFilePath();
  }

  const QUrl url = QUrl::fromLocalFile(filename);

  // Search in the library
  Song library_song;
  if (library_) {
    library_song = library_->GetSongByUrl(url, beginning);
  }

  // If it was found in the library then use it, otherwise load metadata from
  // disk.
  if (library_song.is_valid()) {
    *song = library_song;
  } else {
    TagReaderClient::Instance()->ReadFileBlocking(filename, song);
  }
}

Song ParserBase::LoadSong(const QString& filename_or_url, qint64 beginning,
                          const QDir& dir) const {
  Song song;
  LoadSong(filename_or_url, beginning, dir, &song);
  return song;
}

QString ParserBase::URLOrRelativeFilename(const QUrl& url,
                                          const QDir& dir) const {
  if (url.scheme() != "file") return url.toString();

  const QString filename = url.toLocalFile();
  if (QDir::isAbsolutePath(filename)) {
    const QString relative = dir.relativeFilePath(filename);

    if (!relative.contains("..")) return relative;
  }
  return filename;
}
