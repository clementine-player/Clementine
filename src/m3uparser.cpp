#include "m3uparser.h"

#include <QtDebug>

M3UParser::M3UParser(QIODevice* device, const QDir& directory, QObject* parent)
    : QObject(parent),
      device_(device),
      type_(STANDARD),
      directory_(directory) {
}

const QList<Song>& M3UParser::Parse() {
  QString line = QString::fromLatin1(device_->readLine()).trimmed();
  if (line.startsWith("#EXTM3U")) {
    // This is in extended M3U format.
    type_ = EXTENDED;
    line = QString::fromLatin1(device_->readLine()).trimmed();
  }

  forever {
    if (line.startsWith('#')) {
      // Extended info or comment.
      if (type_ == EXTENDED && line.startsWith("#EXT")) {
        if (!ParseMetadata(line, &current_metadata_)) {
          qWarning() << "Failed to parse metadata: " << line;
          continue;
        }
      }
    } else {
      Song song;
      song.set_title(current_metadata_.title);
      song.set_artist(current_metadata_.artist);
      song.set_length(current_metadata_.length);
      // Track location.
      QString location;
      if (!ParseTrackLocation(line, &song)) {
        qWarning() << "Failed to parse location: " << line;
      } else {
        songs_ << song;
        current_metadata_.artist.clear();
        current_metadata_.title.clear();
        current_metadata_.length = -1;
      }
    }
    if (device_->atEnd()) {
      break;
    }
    line = QString::fromLatin1(device_->readLine()).trimmed();
  }

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

bool M3UParser::ParseTrackLocation(const QString& line, Song* song) const {
  if (line.contains(QRegExp("^[a-z]+://"))) {
    // Looks like a url.
    QUrl temp(line);
    if (temp.isValid()) {
      song->set_filename(temp.toString());
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
    if (!QFile::exists(proper_path)) {
      return false;
    }
    song->set_filename(proper_path);
  } else {
    // Relative path.
    QString proper_path = QDir::fromNativeSeparators(line);
    QString absolute_path = directory_.absoluteFilePath(proper_path);
    if (!QFile::exists(absolute_path)) {
      return false;
    }
    song->set_filename(absolute_path);
  }
  song->InitFromFile(song->filename(), -1);
  return true;
}
