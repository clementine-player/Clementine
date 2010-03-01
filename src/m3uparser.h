#ifndef M3UPARSER_H
#define M3UPARSER_H

#include <QDir>
#include <QObject>
#include <QUrl>

#include "gtest/gtest_prod.h"

#include "song.h"

class QIODevice;

class M3UParser : public QObject {
  Q_OBJECT
 public:
  M3UParser(QIODevice* device, QDir directory = QDir(), QObject* parent = 0);
  virtual ~M3UParser() {}

  const QList<Song>& Parse();

  struct Metadata {
    QString artist;
    QString title;
    int length;
  };

 private:
  enum M3UType {
    STANDARD = 0,
    EXTENDED,  // Includes extended info (track, artist, etc.)
    LINK       // Points to a directory.
  };

  bool ParseMetadata(QString line, Metadata* metadata) const;
  bool ParseTrackLocation(QString line, QUrl* url) const;

  FRIEND_TEST(M3UParserTest, ParsesMetadata);
  FRIEND_TEST(M3UParserTest, ParsesTrackLocation);

  QIODevice* device_;
  M3UType type_;
  QDir directory_;
  Metadata current_metadata_;

  QList<Song> songs_;
};

#endif  // M3UPARSER_H
