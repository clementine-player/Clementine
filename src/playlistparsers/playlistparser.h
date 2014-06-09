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

#ifndef PLAYLISTPARSER_H
#define PLAYLISTPARSER_H

#include <QDir>
#include <QObject>

#include "core/song.h"

class ParserBase;
class LibraryBackendInterface;

class PlaylistParser : public QObject {
  Q_OBJECT

 public:
  PlaylistParser(LibraryBackendInterface* library, QObject* parent = nullptr);

  static const int kMagicSize;

  QStringList file_extensions() const;
  QString filters() const;

  QString default_extension() const;
  QString default_filter() const;

  ParserBase* ParserForMagic(const QByteArray& data,
                             const QString& mime_type = QString()) const;
  ParserBase* ParserForExtension(const QString& suffix) const;

  SongList LoadFromFile(const QString& filename) const;
  SongList LoadFromDevice(QIODevice* device,
                          const QString& path_hint = QString(),
                          const QDir& dir_hint = QDir()) const;
  void Save(const SongList& songs, const QString& filename) const;

 private:
  QString FilterForParser(const ParserBase* parser,
                          QStringList* all_extensions = nullptr) const;

 private:
  QList<ParserBase*> parsers_;
  ParserBase* default_parser_;
};

#endif  // PLAYLISTPARSER_H
