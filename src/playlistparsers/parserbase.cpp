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
#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "library/sqlrow.h"

#include <QUrl>

ParserBase::ParserBase(LibraryBackendInterface* library, QObject *parent)
  : QObject(parent),
    library_(library)
{
}

bool ParserBase::ParseTrackLocation(const QString& filename_or_url,
                                    const QDir& dir, Song* song) const {
  if (filename_or_url.contains(QRegExp("^[a-z]+://"))) {
    // Looks like a url.
    QUrl temp(filename_or_url);
    if (temp.isValid()) {
      song->set_filename(temp.toString());
      song->set_filetype(Song::Type_Stream);
      song->set_valid(true);
      return true;
    } else {
      return false;
    }
  }

  // Should be a local path.
  if (QDir::isAbsolutePath(filename_or_url)) {
    // Absolute path.
    // Fix windows \, eg. C:\foo -> C:/foo.
    song->set_filename(QDir::fromNativeSeparators(filename_or_url));
  } else {
    // Relative path.
    QString proper_path = QDir::fromNativeSeparators(filename_or_url);
    QString absolute_path = dir.absoluteFilePath(proper_path);
    song->set_filename(absolute_path);
  }
  return true;
}

QString ParserBase::MakeRelativeTo(const QString& filename_or_url,
                                   const QDir& dir) const {
  if (filename_or_url.contains(QRegExp("^[a-z]+://")))
    return filename_or_url;

  if (QDir::isAbsolutePath(filename_or_url)) {
    QString relative = dir.relativeFilePath(filename_or_url);

    if (!relative.contains(".."))
      return relative;
  }
  return filename_or_url;
}

QString ParserBase::MakeUrl(const QString& filename_or_url) const {
  if (filename_or_url.contains(QRegExp("^[a-z]+://"))) {
    return filename_or_url;
  }

  return QUrl::fromLocalFile(filename_or_url).toString();
}

Song ParserBase::LoadLibrarySong(const QString& filename_or_url, qint64 beginning) const {
  if (!library_)
    return Song();

  QFileInfo info;

  if (filename_or_url.contains("://"))
    info.setFile(QUrl(filename_or_url).path());
  else
    info.setFile(filename_or_url);

  return library_->GetSongByFilename(info.canonicalFilePath(), beginning);
}
