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

#ifndef ULTIMATELYRICSREADER_H
#define ULTIMATELYRICSREADER_H

#include "ultimatelyricsprovider.h"

#include <QObject>
#include <QXmlStreamReader>

class QIODevice;
class QThread;

class UltimateLyricsReader : public QObject {
  Q_OBJECT

 public:
  UltimateLyricsReader(QObject* parent = nullptr);

  QList<SongInfoProvider*> Parse(const QString& filename) const;
  QList<SongInfoProvider*> ParseDevice(QIODevice* device) const;

  void SetThread(QThread* thread);

 private:
  SongInfoProvider* ParseProvider(QXmlStreamReader* reader) const;
  UltimateLyricsProvider::Rule ParseRule(QXmlStreamReader* reader) const;
  QString ParseInvalidIndicator(QXmlStreamReader* reader) const;

  QThread* thread_;
};

#endif  // ULTIMATELYRICSREADER_H
