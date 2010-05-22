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

#include "playlistparser.h"
#include "xspfparser.h"
#include "m3uparser.h"

#include <QtDebug>

PlaylistParser::PlaylistParser(QObject *parent)
  : QObject(parent)
{
  parsers_ << new M3UParser(this);
  parsers_ << new XSPFParser(this);
}

QStringList PlaylistParser::file_extensions() const {
  QStringList ret;

  foreach (ParserBase* parser, parsers_) {
    ret << parser->file_extensions();
  }

  qStableSort(ret);
  return ret;
}

bool PlaylistParser::can_load(const QString &filename) const {
  return file_extensions().contains(QFileInfo(filename).suffix());
}

ParserBase* PlaylistParser::ParserForExtension(const QString& suffix) const {
  foreach (ParserBase* p, parsers_) {
    if (p->file_extensions().contains(suffix))
      return p;
  }
  return NULL;
}

SongList PlaylistParser::Load(const QString &filename) const {
  QFileInfo info(filename);

  // Find a parser that supports this file extension
  ParserBase* parser = ParserForExtension(info.suffix());
  if (!parser) {
    qWarning() << "Unknown filetype:" << filename;
    return SongList();
  }

  // Open the file
  QFile file(filename);
  file.open(QIODevice::ReadOnly);

  return parser->Load(&file, info.absolutePath());
}

void PlaylistParser::Save(const SongList &songs, const QString &filename) const {
  QFileInfo info(filename);

  // Find a parser that supports this file extension
  ParserBase* parser = ParserForExtension(info.suffix());
  if (!parser) {
    qWarning() << "Unknown filetype:" << filename;
    return;
  }

  // Open the file
  QFile file(filename);
  file.open(QIODevice::WriteOnly);

  return parser->Save(songs, &file, info.absolutePath());
}
