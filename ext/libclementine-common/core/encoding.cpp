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

#include "encoding.h"
#include "core/logging.h"

#include <QTextCodec>
#include <QtDebug>

#include <fileref.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <flacfile.h>

#include "core/logging.h"
#include "engines/enginebase.h"

UniversalEncodingHandler::UniversalEncodingHandler()
  : nsUniversalDetector(NS_FILTER_ALL),
    current_codec_(NULL) {
}

UniversalEncodingHandler::UniversalEncodingHandler(uint32_t language_filter)
  : nsUniversalDetector(language_filter),
    current_codec_(NULL) {
}

TagLib::String UniversalEncodingHandler::parse(const TagLib::ByteVector& data) const {
  const_cast<UniversalEncodingHandler*>(this)->Reset();
  const_cast<UniversalEncodingHandler*>(this)->HandleData(data.data(), data.size());
  const_cast<UniversalEncodingHandler*>(this)->DataEnd();

  if (!current_codec_) {
    return TagLib::String(data);  // Latin-1
  } else {
    // Detected codec -> QString (UTF-16) -> UTF8 -> UTF16-BE (TagLib::String)
    // That's probably expensive.
    QString unicode = current_codec_->toUnicode(data.data(), data.size());
    return TagLib::String(unicode.toUtf8().constData(), TagLib::String::UTF8);
  }
}

/*
TagLib::ByteVector UniversalEncodingHandler::render(const TagLib::String& s) const {
  // TODO: what should we do here?
  // 1. Coerce to ASCII
  // 2. Just write UTF8
  // 3. Write what we read
  // 4. Nothing and rewrite the tag as ID3v2 & UTF8
  return TagLib::ByteVector();
}
*/

void UniversalEncodingHandler::Report(const char* charset) {
  if (qstrcmp(charset, "ASCII") == 0) {
    current_codec_ = 0;
    return;
  }

  QTextCodec* codec = QTextCodec::codecForName(charset);
  current_codec_ = codec;
}

QTextCodec* UniversalEncodingHandler::Guess(const char* data) {
  Reset();
  HandleData(data, qstrlen(data));
  DataEnd();

  if (!current_codec_) {
    // Windows-1251 heuristic.
    const uchar* d = reinterpret_cast<const uchar*>(data);
    int repeats = 0;
    while (uchar x = *d++) {
      if (x >= 0xc0) {
        ++repeats;
      } else {
        repeats = 0;
      }
      if (repeats > 3) {
        qLog(Warning) << "Heuristic guessed windows-1251";
        current_codec_ = QTextCodec::codecForName("windows-1251");
      }
    }
  }
  return current_codec_;
}

QTextCodec* UniversalEncodingHandler::Guess(const TagLib::FileRef& fileref) {
  const TagLib::Tag& tag = *fileref.tag();
  QHash<QTextCodec*, int> usages;
  Guess(tag.title(), &usages);
  Guess(tag.artist(), &usages);
  Guess(tag.album(), &usages);
  Guess(tag.comment(), &usages);
  Guess(tag.genre(), &usages);

  if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref.file())) {
    if (file->ID3v2Tag()) {
      if (!file->ID3v2Tag()->frameListMap()["TCOM"].isEmpty())
        Guess(file->ID3v2Tag()->frameListMap()["TCOM"].front()->toString(), &usages);

      if (!file->ID3v2Tag()->frameListMap()["TPE2"].isEmpty()) // non-standard: Apple, Microsoft
        Guess(file->ID3v2Tag()->frameListMap()["TPE2"].front()->toString(), &usages);
    }
  } else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref.file())) {
    if (file->tag()) {
      if (!file->tag()->fieldListMap()["COMPOSER"].isEmpty() )
        Guess(file->tag()->fieldListMap()["COMPOSER"].front(), &usages);
    }
  } else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref.file())) {
    if (file->xiphComment()) {
      if (!file->xiphComment()->fieldListMap()["COMPOSER"].isEmpty())
        Guess(file->xiphComment()->fieldListMap()["COMPOSER"].front(), &usages);
    }
  }

  if (usages.isEmpty()) {
    return NULL;
  }

  QHash<QTextCodec*, int>::const_iterator max =
      std::max_element(usages.constBegin(), usages.constEnd());
  return max.key();
}

void UniversalEncodingHandler::Guess(const TagLib::String& input,
                                     QHash<QTextCodec*, int>* usages) {
  if (input.isEmpty()) {
    return;  // Empty strings don't vote.
  }

  QTextCodec* codec = Guess(input);
  if (codec) {
    // Ascii doesn't vote either
    ++(*usages)[codec];  // Qt automatically initialises ints to 0.
  }
}

QTextCodec* UniversalEncodingHandler::Guess(const TagLib::String& input) {
  if (input.isAscii()) {
    return NULL;
  }
  if (input.isLatin1()) {
    qLog(Warning) << "Extended ASCII... possibly should be CP866 or windows-1251 instead";
    std::string broken = input.toCString(true);
    std::string fixed = QString::fromUtf8(broken.c_str()).toStdString();
    QTextCodec* codec = Guess(fixed.c_str());
    return codec;
  }
  return QTextCodec::codecForName("UTF-8");
}

QTextCodec* UniversalEncodingHandler::Guess(const Engine::SimpleMetaBundle& bundle) {
  QHash<QTextCodec*, int> usages;
  ++usages[Guess(bundle, &Engine::SimpleMetaBundle::title)];
  ++usages[Guess(bundle, &Engine::SimpleMetaBundle::artist)];
  ++usages[Guess(bundle, &Engine::SimpleMetaBundle::album)];
  ++usages[Guess(bundle, &Engine::SimpleMetaBundle::comment)];
  ++usages[Guess(bundle, &Engine::SimpleMetaBundle::genre)];

  usages.remove(NULL);  // Remove votes for ASCII.
  QHash<QTextCodec*, int>::const_iterator max =
      std::max_element(usages.constBegin(), usages.constEnd());
  if (max != usages.constEnd()) {
    return max.key();
  }
  return NULL;
}

QString UniversalEncodingHandler::FixEncoding(const TagLib::String& input) {
  if (input.isLatin1() && !input.isAscii()) {
    qLog(Warning) << "Extended ASCII... possibly should be CP866 or windows-1251 instead";
    std::string broken = input.toCString(true);
    std::string fixed;
    if (broken.size() > input.size()) {
      fixed = QString::fromUtf8(broken.c_str()).toStdString();
      QTextCodec* codec = Guess(fixed.c_str());
      if (!codec) {
        qLog(Debug) << "Could not guess encoding. Using extended ASCII.";
      } else {
        qLog(Debug) << "Guessed:" << codec->name();
        QString foo = codec->toUnicode(fixed.c_str());
        return foo.trimmed();
      }
    }
  }
  return TStringToQString(input).trimmed();
}
