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

#include "asxparser.h"
#include "asxiniparser.h"
#include "cueparser.h"
#include "m3uparser.h"
#include "playlistparser.h"
#include "plsparser.h"
#include "wplparser.h"
#include "xspfparser.h"
#include "core/logging.h"

#include <algorithm>

#include <QtDebug>

const int PlaylistParser::kMagicSize = 512;

PlaylistParser::PlaylistParser(LibraryBackendInterface* library,
                               QObject* parent)
    : QObject(parent) {
  default_parser_ = new XSPFParser(library, this);
  parsers_ << new M3UParser(library, this);
  parsers_ << default_parser_;
  parsers_ << new PLSParser(library, this);
  parsers_ << new ASXParser(library, this);
  parsers_ << new AsxIniParser(library, this);
  parsers_ << new CueParser(library, this);
  parsers_ << new WplParser(library, this);
}

QStringList PlaylistParser::file_extensions() const {
  QStringList ret;

  for (ParserBase* parser : parsers_) {
    ret << parser->file_extensions();
  }

  std::stable_sort(ret.begin(), ret.end());
  return ret;
}

QStringList PlaylistParser::mime_types() const {
  QStringList ret;

  for (ParserBase* parser : parsers_) {
    if (!parser->mime_type().isEmpty()) ret << parser->mime_type();
  }

  std::stable_sort(ret.begin(), ret.end());
  return ret;
}

QString PlaylistParser::filters() const {
  QStringList filters;
  QStringList all_extensions;
  for (ParserBase* parser : parsers_) {
    filters << FilterForParser(parser, &all_extensions);
  }

  filters.prepend(tr("All playlists (%1)").arg(all_extensions.join(" ")));

  return filters.join(";;");
}

QString PlaylistParser::FilterForParser(const ParserBase* parser,
                                        QStringList* all_extensions) const {
  QStringList extensions;
  for (const QString& extension : parser->file_extensions())
    extensions << "*." + extension;

  if (all_extensions) *all_extensions << extensions;

  return tr("%1 playlists (%2)").arg(parser->name(), extensions.join(" "));
}

QString PlaylistParser::default_extension() const {
  return default_parser_->file_extensions()[0];
}

QString PlaylistParser::default_filter() const {
  return FilterForParser(default_parser_);
}

ParserBase* PlaylistParser::ParserForExtension(const QString& suffix) const {
  for (ParserBase* p : parsers_) {
    if (p->file_extensions().contains(suffix)) return p;
  }
  return nullptr;
}

ParserBase* PlaylistParser::ParserForMimeType(const QString& mime_type) const {
  for (ParserBase* p : parsers_) {
    if (!p->mime_type().isEmpty() &&
        (QString::compare(p->mime_type(), mime_type, Qt::CaseInsensitive) == 0))
      return p;
  }
  return nullptr;
}

ParserBase* PlaylistParser::ParserForMagic(const QByteArray& data,
                                           const QString& mime_type) const {
  for (ParserBase* p : parsers_) {
    if ((!mime_type.isEmpty() && mime_type == p->mime_type()) ||
        p->TryMagic(data))
      return p;
  }
  return nullptr;
}

SongList PlaylistParser::LoadFromFile(const QString& filename) const {
  QFileInfo info(filename);

  // Find a parser that supports this file extension
  ParserBase* parser = ParserForExtension(info.suffix());
  if (!parser) {
    qLog(Warning) << "Unknown filetype:" << filename;
    return SongList();
  }

  // Open the file
  QFile file(filename);
  file.open(QIODevice::ReadOnly);

  return parser->Load(&file, filename, info.absolutePath());
}

SongList PlaylistParser::LoadFromDevice(QIODevice* device,
                                        const QString& path_hint,
                                        const QDir& dir_hint) const {
  // Find a parser that supports this data
  ParserBase* parser = ParserForMagic(device->peek(kMagicSize));
  if (!parser) {
    return SongList();
  }

  return parser->Load(device, path_hint, dir_hint);
}

void PlaylistParser::Save(const SongList& songs, const QString& filename,
                          Playlist::Path path_type) const {
  QFileInfo info(filename);

  // Find a parser that supports this file extension
  ParserBase* parser = ParserForExtension(info.suffix());
  if (!parser) {
    qLog(Warning) << "Unknown filetype:" << filename;
    return;
  }

  // Open the file
  QFile file(filename);
  file.open(QIODevice::WriteOnly);

  return parser->Save(songs, &file, info.absolutePath(), path_type);
}
