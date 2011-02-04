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

#ifndef ENCODING_H
#define ENCODING_H

#include <QHash>
#include <QString>

#include <id3v1tag.h>

#include "nsUniversalDetector.h"

#include "engines/engine_fwd.h"

namespace TagLib {
  class FileRef;

  namespace ID3v2 {
    class Tag;
  }
}

class QTextCodec;

class UniversalEncodingHandler : public TagLib::ID3v1::StringHandler,
                                 nsUniversalDetector {
 public:
  UniversalEncodingHandler();
  explicit UniversalEncodingHandler(uint32_t language_filter);

  // TagLib::ID3v1::StringHandler
  virtual TagLib::String parse(const TagLib::ByteVector& data) const;

  QTextCodec* Guess(const char* data);
  QTextCodec* Guess(const TagLib::FileRef& file);
  QTextCodec* Guess(const TagLib::String& input);
  QTextCodec* Guess(const Engine::SimpleMetaBundle& metadata);

  QString FixEncoding(const TagLib::String& input);
 private:
  template<typename T>
  QTextCodec* Guess(const Engine::SimpleMetaBundle& bundle, T Engine::SimpleMetaBundle::*field) {
    return Guess(static_cast<QString>(bundle.*field).toUtf8().constData());
  }

  // nsUniversalDetector
  virtual void Report(const char* charset);

  void Guess(const TagLib::String& input, QHash<QTextCodec*, int>* usages);

  QTextCodec* current_codec_;
};

#endif
