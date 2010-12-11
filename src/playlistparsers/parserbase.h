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

#ifndef PARSERBASE_H
#define PARSERBASE_H

#include <QObject>
#include <QDir>

#include "core/song.h"

class LibraryBackendInterface;

class ParserBase : public QObject {
  Q_OBJECT

public:
  ParserBase(LibraryBackendInterface* library, QObject *parent = 0);

  virtual QString name() const = 0;
  virtual QStringList file_extensions() const = 0;
  virtual QString mime_type() const { return QString(); }

  virtual bool TryMagic(const QByteArray& data) const = 0;

  virtual SongList Load(QIODevice* device, const QDir& dir = QDir()) const = 0;
  virtual void Save(const SongList& songs, QIODevice* device, const QDir& dir = QDir()) const = 0;

protected:
  // Takes a URL, relative path or absolute path, and returns an absolute path.
  // Resolves relative paths to "dir".
  bool ParseTrackLocation(const QString& filename_or_url, const QDir& dir,
                          Song* song) const;

  // Takes a URL, relative path or absolute path, and in the case of absolute
  // paths makes them relative to dir if they are subdirectories.
  QString MakeRelativeTo(const QString& filename_or_url, const QDir& dir) const;

  // Takes a URL or absolute path and returns a URL
  QString MakeUrl(const QString& filename_or_url) const;

  // Converts the URL or path to a canonical path and searches the library for
  // a song with that path.  If one is found, returns it, otherwise returns an
  // invalid song.
  Song LoadLibrarySong(const QString& filename_or_url) const;

private:
  LibraryBackendInterface* library_;
};

#endif // PARSERBASE_H
