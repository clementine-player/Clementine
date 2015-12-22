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

#ifndef CUEPARSER_H
#define CUEPARSER_H

#include "parserbase.h"

#include <QRegExp>

// This parser will try to detect the real encoding of a .cue file but there's
// a great chance it will fail so it's probably best to assume that the parser
// is UTF compatible only.
class CueParser : public ParserBase {
  Q_OBJECT

 public:
  static const char* kFileLineRegExp;
  static const char* kIndexRegExp;

  static const char* kPerformer;
  static const char* kTitle;
  static const char* kSongWriter;
  static const char* kFile;
  static const char* kTrack;
  static const char* kIndex;
  static const char* kAudioTrackType;
  static const char* kRem;
  static const char* kGenre;
  static const char* kDate;
  static const char* kDisc;
  
  CueParser(LibraryBackendInterface* library, QObject* parent = nullptr);

  QString name() const { return "CUE"; }
  QStringList file_extensions() const { return QStringList() << "cue"; }
  QString mime_type() const { return "application/x-cue"; }

  bool TryMagic(const QByteArray& data) const;

  SongList Load(QIODevice* device, const QString& playlist_path = "",
                const QDir& dir = QDir()) const;
  void Save(const SongList& songs, QIODevice* device, const QDir& dir = QDir(),
            Playlist::Path path_type = Playlist::Path_Automatic) const;

 private:
  // A single TRACK entry in .cue file.
  struct CueEntry {
    QString file;

    QString index;

    QString title;
    QString artist;
    QString album_artist;
    QString album;

    QString composer;
    QString album_composer;

    QString genre;
    QString date;
    QString disc;

    QString PrettyArtist() const {
      return artist.isEmpty() ? album_artist : artist;
    }
    QString PrettyComposer() const {
      return composer.isEmpty() ? album_composer : composer;
    }

    CueEntry(QString& file, QString& index, QString& title, QString& artist,
             QString& album_artist, QString& album, QString& composer,
             QString& album_composer, QString& genre, QString& date, QString& disc) {
      this->file = file;
      this->index = index;
      this->title = title;
      this->artist = artist;
      this->album_artist = album_artist;
      this->album = album;
      this->composer = composer;
      this->album_composer = album_composer;
      this->genre = genre;
      this->date = date;
      this->disc = disc;
    }
  };

  bool UpdateSong(const CueEntry& entry, const QString& next_index,
                  Song* song) const;
  bool UpdateLastSong(const CueEntry& entry, Song* song) const;

  QStringList SplitCueLine(const QString& line) const;
  qint64 IndexToMarker(const QString& index) const;
};

#endif  // CUEPARSER_H
