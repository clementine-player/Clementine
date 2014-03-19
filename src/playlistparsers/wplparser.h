/* This file is part of Clementine.
   Copyright 2013, David Sansome <me@davidsansome.com>

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

#ifndef WPLPARSER_H
#define WPLPARSER_H

#include "xmlparser.h"

class WplParser : public XMLParser {
 public:
  WplParser(LibraryBackendInterface* library, QObject* parent = nullptr);

  QString name() const { return "WPL"; }
  QStringList file_extensions() const { return QStringList() << "wpl"; }
  QString mime_type() const { return "application/vnd.ms-wpl"; }

  bool TryMagic(const QByteArray& data) const;

  SongList Load(QIODevice* device, const QString& playlist_path,
                const QDir& dir) const;
  void Save(const SongList& songs, QIODevice* device, const QDir& dir) const;

 private:
  void ParseSeq(const QDir& dir, QXmlStreamReader* reader,
                SongList* songs) const;
  void WriteMeta(const QString& name, const QString& content,
                 QXmlStreamWriter* writer) const;
};

#endif  // WPLPARSER_H
