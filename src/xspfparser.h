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

#ifndef XSPFPARSER_H
#define XSPFPARSER_H

#include "song.h"

#include <QObject>
#include <QXmlStreamReader>

class QIODevice;

class XSPFParser : public QObject {
  Q_OBJECT
 public:
  XSPFParser(QIODevice* device, QObject* parent = 0);
  virtual ~XSPFParser() {}

  const SongList& Parse();

 private:
  bool ParseUntilElement(QXmlStreamReader* reader, const QString& element) const;
  void IgnoreElement(QXmlStreamReader* reader) const;
  Song ParseTrack(QXmlStreamReader* reader) const;

  QIODevice* device_;
  SongList songs_;
};

#endif
