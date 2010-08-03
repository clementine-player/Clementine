#include "encoding.h"

#include <QTextCodec>
#include <QtDebug>

#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/flacfile.h>

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
        qWarning() << "Heuristic guessed windows-1251";
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

  QHash<QTextCodec*, int>::const_iterator max = std::max_element(usages.begin(), usages.end());
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
    qWarning() << "Extended ASCII... possibly should be CP866 or windows-1251 instead";
    std::string broken = input.toCString(true);
    std::string fixed = QString::fromUtf8(broken.c_str()).toStdString();
    QTextCodec* codec = Guess(fixed.c_str());
    return codec;
  }
  return QTextCodec::codecForName("UTF-8");
}

QString UniversalEncodingHandler::FixEncoding(const TagLib::String& input) {
  if (input.isLatin1() && !input.isAscii()) {
    qWarning() << "Extended ASCII... possibly should be CP866 or windows-1251 instead";
    std::string broken = input.toCString(true);
    std::string fixed;
    if (broken.size() > input.size()) {
      fixed = QString::fromUtf8(broken.c_str()).toStdString();
      QTextCodec* codec = Guess(fixed.c_str());
      if (!codec) {
        qDebug() << "Could not guess encoding. Using extended ASCII.";
      } else {
        qDebug() << "Guessed:" << codec->name();
        QString foo = codec->toUnicode(fixed.c_str());
        return foo.trimmed();
      }
    }
  }
  return TStringToQString(input).trimmed();
}
