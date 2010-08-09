#ifndef ENCODING_H
#define ENCODING_H

#include <QHash>
#include <QString>

#include <taglib/id3v1tag.h>

#include "nsUniversalDetector.h"

namespace TagLib {
  class FileRef;

  namespace ID3v2 {
    class Tag;
  }
}

namespace Engine {
class SimpleMetaBundle;
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
