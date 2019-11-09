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

#ifndef XMLPARSER_H
#define XMLPARSER_H

#include "parserbase.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class QDomDocument;
class QDomNode;

class XMLParser : public ParserBase {
 protected:
  XMLParser(LibraryBackendInterface* library, QObject* parent);

  class StreamElement {
   public:
    StreamElement(const QString& name, QXmlStreamWriter* stream)
        : stream_(stream) {
      stream->writeStartElement(name);
    }

    ~StreamElement() { stream_->writeEndElement(); }

   private:
    QXmlStreamWriter* stream_;
    Q_DISABLE_COPY(StreamElement)
  };
};

#endif
