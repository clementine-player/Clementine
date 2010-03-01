#include "m3uparser.h"

#include <QtDebug>

M3UParser::M3UParser(QIODevice* device, const QDir& directory, QObject* parent)
    : QObject(parent),
      device_(device),
      type_(STANDARD),
      directory_(directory) {
}

const QList<Song>& M3UParser::Parse() {
  QString line = QString::fromLatin1(device_->readLine());
  if (line == "#EXTM3U") {
    // This is in extended M3U format.
    type_ = EXTENDED;
    line = QString::fromLatin1(device_->readLine());
  }

  do {
    if (line.startsWith('#')) {
      // Extended info or comment.
      if (type_ == EXTENDED && line.startsWith("#EXT")) {
        if (!ParseMetadata(line, &current_metadata_)) {
          qWarning() << "Failed to parse metadata: " << line;
        }
      }
    } else {
      // Track location.
      QUrl url;
      if (!ParseTrackLocation(line, &url)) {
        qWarning() << "Failed to parse location: " << line;
      }
    }
    line = QString::fromLatin1(device_->readLine());
  } while (device_->canReadLine());

  return songs_;
}

bool M3UParser::ParseMetadata(const QString& line, M3UParser::Metadata* metadata) const {
  // Extended info, eg.
  // #EXTINF:123,Sample Artist - Sample title
  QString info = line.section(':', 1);
  QString l = info.section(',', 0, 0);
  bool ok = false;
  int length = l.toInt(&ok);
  if (!ok) {
    return false;
  }
  QString track_info = info.section(',', 1);
  QStringList list = track_info.split('-');
  if (list.size() <= 1) {
    return false;
  }
  metadata->artist = list[0].trimmed();
  metadata->title = list[1].trimmed();
  metadata->length = length;
  return true;
}

bool M3UParser::ParseTrackLocation(const QString& line, QUrl* url) const {
  if (line.contains(QRegExp("^[a-z]+://"))) {
    // Looks like a url.
    QUrl temp(line);
    if (temp.isValid()) {
      *url = temp;
      return true;
    } else {
      return false;
    }
  }

  // Should be a local path.
  if (QDir::isAbsolutePath(line)) {
    // Absolute path.
    // Fix windows \, eg. C:\foo -> C:/foo.
    QString proper_path = QDir::fromNativeSeparators(line);
    // C:/foo -> /C:/foo.
    if (!proper_path.startsWith('/')) {
      proper_path.prepend("/");
    }
    *url = "file://" + proper_path;
    return true;
  } else {
    // Relative path.
    QString proper_path = QDir::fromNativeSeparators(line);
    QString absolute_path = directory_.absoluteFilePath(proper_path);
    // C:/foo -> /C:/foo.
    if (!absolute_path.startsWith('/')) {
      absolute_path.prepend("/");
    }
    *url = "file://" + absolute_path;
    return true;
  }
}
