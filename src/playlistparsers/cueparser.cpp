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

#include "cueparser.h"

#include <QBuffer>
#include <QStringBuilder>
#include <QRegExp>
#include <QTextStream>
#include <QtDebug>

const char* CueParser::kFileLineRegExp = "(\\S+)\\s+(?:\"([^\"]+)\"|(\\S+))\\s*(?:\"([^\"]+)\"|(\\S+))?";
const char* CueParser::kIndexRegExp = "(\\d{2}):(\\d{2}):(\\d{2})";

const char* CueParser::kPerformer = "performer";
const char* CueParser::kTitle = "title";
const char* CueParser::kFile = "file";
const char* CueParser::kTrack = "track";
const char* CueParser::kIndex = "index";
const char* CueParser::kAudioTrackType = "audio";

// TODO: utf and regexps (check on Zucchero - there's something wrong)

CueParser::CueParser(LibraryBackendInterface* library, QObject* parent)
    : ParserBase(library, parent)
{
}

SongList CueParser::Load(QIODevice* device, const QDir& dir) const {
  SongList ret;

  QTextStream text_stream(device);
  QString dir_path = dir.absolutePath();

  QString line;

  QString album_artist;
  QString album;
  QString file;

  // header
  while (!(line = text_stream.readLine()).isNull()) {
    QStringList splitted = SplitCueLine(line);

    // uninteresting or incorrect line
    if(splitted.size() < 2) {
      continue;
    }

    QString line_name = splitted[0].toLower();
    QString line_value = splitted[1];

    // PERFORMER
    if(line_name == kPerformer) {

      album_artist = line_value;

    // TITLE
    } else if(line_name == kTitle) {

      album = line_value;

    // FILE
    } else if(line_name == kFile) {

      file = QDir::isAbsolutePath(line_value)
                ? line_value
                : dir.absoluteFilePath(line_value);

    // end of the header -> go into the track mode
    } else if(line_name == kTrack) {

      break;

    }

    // just ignore the rest of possible field types for now...
  }

  if(line.isNull()) {
    qWarning() << "the .cue file from " << dir_path << " defines no tracks!";
    return ret;
  }

  QString track_type;
  QString index;
  QString artist;
  QString title;

  QList<CueEntry> entries;

  // tracks
  do {
    QStringList splitted = SplitCueLine(line);

    // uninteresting or incorrect line
    if(splitted.size() < 2) {
      continue;
    }

    QString line_name = splitted[0].toLower();
    QString line_value = splitted[1];
    QString line_additional = splitted.size() > 2 ? splitted[2].toLower() : "";

    if(line_name == kTrack) {

      // the beginning of another track's definition - we're saving the current one
      // for later (if it's valid of course)
      if(!index.isEmpty() && (track_type.isEmpty() || track_type == kAudioTrackType)) {
        entries.append(CueEntry(file, index, title, artist, album_artist, album));
      }

      // clear the state
      track_type = index = artist = title = "";

      if(!line_additional.isEmpty()) {
        track_type = line_additional;
      }

    } else if(line_name == kIndex) {

      // we need the index's position field
      if(!line_additional.isEmpty()) {

        // if there's none "01" index, we'll just take the first one
        // also, we'll take the "01" index even if it's the last one
        if(line_value == "01" || index.isEmpty()) {

          index = line_additional;

        }

      }

    } else if(line_name == kPerformer) {

      artist = line_value;

    } else if(line_name == kTitle) {

      title = line_value;

    }

    // just ignore the rest of possible field types for now...
  } while(!(line = text_stream.readLine()).isNull());

  // we didn't add the last song yet...
  if(!index.isEmpty() && (track_type.isEmpty() || track_type == kAudioTrackType)) {
    entries.append(CueEntry(file, index, title, artist, album_artist, album));
  }

  // finalize parsing songs
  for(int i = 0; i < entries.length(); i++) {
    CueEntry entry = entries.at(i);

    Song current;
    if (!ParseTrackLocation(entry.file, dir, &current)) {
      qWarning() << "failed to parse location in .cue file from " << dir_path;
    } else {
      // look for the section in library
      Song song = LoadLibrarySong(current.filename(), IndexToMarker(entry.index));
      if (!song.is_valid()) {
        song.InitFromFile(current.filename(), -1);
      }

      // overwrite the stuff, we may have read from the file or library, using
      // the current .cue metadata
      song.set_track(i + 1);
      if(i + 1 < entries.size()) {
        // incorrect indices?
        if(!UpdateSong(entry, entries.at(i + 1).index, &song)) {
          continue;
        }
      } else {
        // incorrect index?
        if(!UpdateLastSong(entry, &song)) {
          continue;
        }
      }

      ret << song;
    }
  }

  return ret;
}

// This and the kFileLineRegExp do most of the "dirty" work, namely: splitting the raw .cue
// line into logical parts and getting rid of all the unnecessary whitespaces and quoting.
QStringList CueParser::SplitCueLine(const QString& line) const {
  QRegExp line_regexp(kFileLineRegExp);
  if(!line_regexp.exactMatch(line.trimmed())) {
    return QStringList();
  }

  // let's remove the empty entries while we're at it
  return line_regexp.capturedTexts().filter(QRegExp(".+")).mid(1, -1);
}

// Updates the song with data from the .cue entry. This one mustn't be used for the
// last song in the .cue file.
bool CueParser::UpdateSong(const CueEntry& entry, const QString& next_index, Song* song) const {
  int beginning = IndexToMarker(entry.index);
  int end = IndexToMarker(next_index);

  // incorrect indices (we won't be able to calculate beginning or end)
  if(beginning == -1 || end == -1) {
    return false;
  }

  song->Init(entry.title, entry.PrettyArtist(),
             entry.album, beginning, end);
  song->set_albumartist(entry.album_artist);

  return true;
}

// Updates the song with data from the .cue entry. This one must be used only for the
// last song in the .cue file.
bool CueParser::UpdateLastSong(const CueEntry& entry, Song* song) const {
  int beginning = IndexToMarker(entry.index);

  // incorrect index (we won't be able to calculate beginning)
  if(beginning == -1) {
    return false;
  }

  song->set_title(entry.title);
  song->set_artist(entry.PrettyArtist());
  song->set_album(entry.album);
  song->set_albumartist(entry.album_artist);

  // we don't do anything with the end here because it's already set to
  // the end of the media file (if it exists)
  song->set_beginning(beginning);

  return true;
}

int CueParser::IndexToMarker(const QString& index) const {
  QRegExp index_regexp(kIndexRegExp);
  if(!index_regexp.exactMatch(index)) {
    return -1;
  }

  QStringList splitted = index_regexp.capturedTexts().mid(1, -1);
  // TODO: use frames when #1166 is fixed
  return splitted.at(0).toInt() * 60 + splitted.at(1).toInt();
}

void CueParser::Save(const SongList &songs, QIODevice *device, const QDir &dir) const {
  // TODO
}

// Looks for a track starting with one of the .cue's keywords.
bool CueParser::TryMagic(const QByteArray &data) const {
  QStringList splitted = QString::fromUtf8(data.constData()).split('\n');

  for(int i = 0; i < splitted.length(); i++) {
    QString line = splitted.at(i).trimmed();
    if(line.startsWith(kPerformer, Qt::CaseInsensitive) ||
       line.startsWith(kTitle, Qt::CaseInsensitive) ||
       line.startsWith(kFile, Qt::CaseInsensitive) ||
       line.startsWith(kTrack, Qt::CaseInsensitive)) {
      return true;
    }
  }

  return false;
}
