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

#include "parserbase.h"

#include <QUrl>

ParserBase::ParserBase(QObject *parent)
  : QObject(parent)
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
      return true;
    } else {
      return false;
    }
  }

  // Should be a local path.
  if (QDir::isAbsolutePath(filename_or_url)) {
    // Absolute path.
    // Fix windows \, eg. C:\foo -> C:/foo.
    QString proper_path = QDir::fromNativeSeparators(filename_or_url);
    if (!QFile::exists(proper_path)) {
      return false;
    }
    song->set_filename(proper_path);
  } else {
    // Relative path.
    QString proper_path = QDir::fromNativeSeparators(filename_or_url);
    QString absolute_path = dir.absoluteFilePath(proper_path);
    song->set_filename(absolute_path);
  }
  song->InitFromFile(song->filename(), -1);
  return true;
}
