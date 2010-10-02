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

#ifndef ULTIMATELYRICSREADER_H
#define ULTIMATELYRICSREADER_H

#include <QObject>
#include <QXmlStreamReader>

#include "htmlscraper.h"

class QIODevice;

class UltimateLyricsReader : public QObject {
  Q_OBJECT

public:
  UltimateLyricsReader(NetworkAccessManager* network, QObject* parent = 0);

  QList<LyricProvider*> Parse(const QString& filename) const;
  QList<LyricProvider*> ParseDevice(QIODevice* device) const;

private:
  LyricProvider* ParseProvider(QXmlStreamReader* reader) const;
  HtmlScraper::Rule ParseRule(QXmlStreamReader* reader) const;
  QString ParseInvalidIndicator(QXmlStreamReader* reader) const;

private:
  NetworkAccessManager* network_;
};

#endif // ULTIMATELYRICSREADER_H
