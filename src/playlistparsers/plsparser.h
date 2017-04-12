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

#ifndef PLSPARSER_H
#define PLSPARSER_H

#include "parserbase.h"

class PLSParser : public ParserBase {
  Q_OBJECT

 public:
  PLSParser(LibraryBackendInterface* library, QObject* parent = nullptr);

  QString name() const { return "PLS"; }
  QStringList file_extensions() const { return QStringList() << "pls"; }
  QString mime_type() const { return "audio/x-scpls"; }

  bool TryMagic(const QByteArray& data) const;

  SongList Load(QIODevice* device, const QString& playlist_path = "",
                const QDir& dir = QDir()) const;
  void Save(const SongList& songs, QIODevice* device, const QDir& dir = QDir(),
            Playlist::Path path_type = Playlist::Path_Automatic) const;
};

#endif  // PLSPARSER_H
