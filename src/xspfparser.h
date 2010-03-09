#ifndef XSPFPARSER_H
#define XSPFPARSER_H

#include "song.h"

#include <QObject>

class QIODevice;
class QXmlStreamReader;

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
