#include "song.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/flacfile.h>

#include <lastfm/Track>

#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QSqlQuery>
#include <QVariant>

const char* Song::kColumnSpec =
    "title, album, artist, albumartist, composer, "
    "track, disc, bpm, year, genre, comment, compilation, "
    "length, bitrate, samplerate, directory, filename, "
    "mtime, ctime, filesize";

const char* Song::kBindSpec =
    ":title, :album, :artist, :albumartist, :composer, "
    ":track, :disc, :bpm, :year, :genre, :comment, :compilation, "
    ":length, :bitrate, :samplerate, :directory_id, :filename, "
    ":mtime, :ctime, :filesize";

const char* Song::kUpdateSpec =
    "title = :title, album = :album, artist = :artist, "
    "albumartist = :albumartist, composer = :composer, track = :track, "
    "disc = :disc, bpm = :bpm, year = :year, genre = :genre, "
    "comment = :comment, compilation = :compilation, length = :length, "
    "bitrate = :bitrate, samplerate = :samplerate, "
    "directory = :directory_id, filename = :filename, mtime = :mtime, "
    "ctime = :ctime, filesize = :filesize";

Song::Song()
  : valid_(false),
    id_(-1),
    track_(-1),
    disc_(-1),
    bpm_(-1),
    year_(-1),
    compilation_(false),
    length_(-1),
    bitrate_(-1),
    samplerate_(-1),
    directory_id_(-1),
    mtime_(-1),
    ctime_(-1),
    filesize_(-1)
{
}

void Song::InitFromFile(const QString& filename, int directory_id) {
  filename_ = filename;
  directory_id_ = directory_id;

  TagLib::FileRef fileref = TagLib::FileRef(QFile::encodeName(filename).constData());

  if( fileref.isNull() )
    return;

  QFileInfo info(filename);
  filesize_ = info.size();
  mtime_ = info.lastModified().toTime_t();
  ctime_ = info.created().toTime_t();

  TagLib::Tag* tag = fileref.tag();
  if (tag) {
    #define strip(x) TStringToQString( x ).trimmed()
    title_ = strip(tag->title());
    artist_ = strip(tag->artist());
    album_ = strip(tag->album());
    comment_ = strip(tag->comment());
    genre_ = strip(tag->genre());
    year_ = tag->year();
    track_ = tag->track();
    #undef strip

    valid_ = true;
  }

  QString disc;
  QString compilation;
  if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref.file())) {
    if (file->ID3v2Tag()) {
      if (!file->ID3v2Tag()->frameListMap()["TPOS"].isEmpty())
        disc = TStringToQString(file->ID3v2Tag()->frameListMap()["TPOS"].front()->toString()).trimmed();

      if (!file->ID3v2Tag()->frameListMap()["TBPM"].isEmpty())
        bpm_ = TStringToQString(file->ID3v2Tag()->frameListMap()["TBPM"].front()->toString()).trimmed().toFloat();

      if (!file->ID3v2Tag()->frameListMap()["TCOM"].isEmpty())
        composer_ = TStringToQString(file->ID3v2Tag()->frameListMap()["TCOM"].front()->toString()).trimmed();

      if (!file->ID3v2Tag()->frameListMap()["TPE2"].isEmpty()) // non-standard: Apple, Microsoft
        albumartist_ = TStringToQString(file->ID3v2Tag()->frameListMap()["TPE2"].front()->toString()).trimmed();

      if (!file->ID3v2Tag()->frameListMap()["TCMP"].isEmpty())
        compilation = TStringToQString(file->ID3v2Tag()->frameListMap()["TCMP"].front()->toString()).trimmed();
    }
  }
  else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref.file())) {
    if (file->tag()) {
      if ( !file->tag()->fieldListMap()["COMPOSER"].isEmpty() )
        composer_ = TStringToQString(file->tag()->fieldListMap()["COMPOSER"].front()).trimmed();

      if ( !file->tag()->fieldListMap()["BPM"].isEmpty() )
        bpm_ = TStringToQString(file->tag()->fieldListMap()["BPM"].front()).trimmed().toFloat();

      if ( !file->tag()->fieldListMap()["DISCNUMBER"].isEmpty() )
        disc = TStringToQString(file->tag()->fieldListMap()["DISCNUMBER"].front()).trimmed();

      if ( !file->tag()->fieldListMap()["COMPILATION"].isEmpty() )
        compilation = TStringToQString(file->tag()->fieldListMap()["COMPILATION"].front()).trimmed();
    }
  }
  else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref.file())) {
    if ( file->xiphComment() ) {
      if (!file->xiphComment()->fieldListMap()["COMPOSER"].isEmpty())
        composer_ = TStringToQString( file->xiphComment()->fieldListMap()["COMPOSER"].front() ).trimmed();

      if (!file->xiphComment()->fieldListMap()["BPM"].isEmpty() )
        bpm_ = TStringToQString( file->xiphComment()->fieldListMap()["BPM"].front() ).trimmed().toFloat();

      if (!file->xiphComment()->fieldListMap()["DISCNUMBER"].isEmpty() )
        disc = TStringToQString( file->xiphComment()->fieldListMap()["DISCNUMBER"].front() ).trimmed();

      if (!file->xiphComment()->fieldListMap()["COMPILATION"].isEmpty() )
        compilation = TStringToQString( file->xiphComment()->fieldListMap()["COMPILATION"].front() ).trimmed();
    }
  }

  if ( !disc.isEmpty() )   {
    int i = disc.indexOf('/');
    if ( i != -1 )
      // disc.right( i ).toInt() is total number of discs, we don't use this at the moment
      disc_ = disc.left( i ).toInt();
    else
      disc_ = disc.toInt();
  }

  if ( compilation.isEmpty() ) {
    // well, it wasn't set, but if the artist is VA assume it's a compilation
    if ( artist_.toLower()  == "various artists" )
      compilation_ = true;
  } else {
    int i = compilation.toInt();
    compilation_ = (i == 1);
  }

  bitrate_    = fileref.audioProperties()->bitrate();
  length_     = fileref.audioProperties()->length();
  samplerate_ = fileref.audioProperties()->sampleRate();
}

void Song::InitFromQuery(const QSqlQuery& q) {
  valid_ = true;

  #define tostr(n) (q.value(n).isNull() ? QString::null : q.value(n).toString())
  #define toint(n) (q.value(n).isNull() ? -1 : q.value(n).toInt())
  #define tofloat(n) (q.value(n).isNull() ? -1 : q.value(n).toDouble())

  id_ = toint(0);
  title_ = tostr(1);
  album_ = tostr(2);
  artist_ = tostr(3);
  albumartist_ = tostr(4);
  composer_ = tostr(5);
  track_ = toint(6);
  disc_ = toint(7);
  bpm_ = tofloat(8);
  year_ = toint(9);
  genre_ = tostr(10);
  comment_ = tostr(11);
  compilation_ = q.value(12).toBool();

  length_ = toint(13);
  bitrate_ = toint(14);
  samplerate_ = toint(15);

  directory_id_ = toint(16);
  filename_ = tostr(17);
  mtime_ = toint(18);
  ctime_ = toint(19);
  filesize_ = toint(20);

  #undef tostr
  #undef toint
  #undef tofloat
}

void Song::InitFromLastFM(const lastfm::Track& track) {
  valid_ = true;

  title_ = track.title();
  album_ = track.album();
  artist_ = track.artist();
  track_ = track.trackNumber();
  length_ = track.duration();
}

void Song::BindToQuery(QSqlQuery *query) const {
  #define intval(x) (x == -1 ? QVariant() : x)

  query->bindValue(":title", title_);
  query->bindValue(":album", album_);
  query->bindValue(":artist", artist_);
  query->bindValue(":albumartist", albumartist_);
  query->bindValue(":composer", composer_);
  query->bindValue(":track", intval(track_));
  query->bindValue(":disc", intval(disc_));
  query->bindValue(":bpm", intval(bpm_));
  query->bindValue(":year", intval(year_));
  query->bindValue(":genre", genre_);
  query->bindValue(":comment", comment_);
  query->bindValue(":compilation", compilation_ ? 1 : 0);

  query->bindValue(":length", intval(length_));
  query->bindValue(":bitrate", intval(bitrate_));
  query->bindValue(":samplerate", intval(samplerate_));

  query->bindValue(":directory_id", intval(directory_id_));
  query->bindValue(":filename", filename_);
  query->bindValue(":mtime", intval(mtime_));
  query->bindValue(":ctime", intval(ctime_));
  query->bindValue(":filesize", intval(filesize_));

  #undef intval
}

void Song::ToLastFM(lastfm::Track* track) const {
  lastfm::MutableTrack mtrack(*track);

  mtrack.setArtist(artist_);
  mtrack.setAlbum(album_);
  mtrack.setTitle(title_);
  mtrack.setDuration(length_);
  mtrack.setTrackNumber(track_);
}

QString Song::PrettyTitleWithArtist() const {
  QString title(title_);

  if (title.isEmpty())
    title = QFileInfo(filename_).baseName();

  if (!compilation_ && !artist_.isEmpty())
    title = artist_ + " - " + title;

  return title;
}

QString Song::PrettyTitle() const {
  QString title(title_);

  if (title.isEmpty())
    title = QFileInfo(filename_).baseName();

  return title;
}

QString Song::PrettyLength() const {
  if (length_ == -1)
    return QString::null;

  int hours = length_ / (60*60);
  int minutes = (length_ / 60) % 60;
  int seconds = length_ % 60;

  QString text;
  if (hours)
    text.sprintf("%d:%02d:%02d", hours, minutes, seconds);
  else
    text.sprintf("%d:%02d", minutes, seconds);
  return text;
}

bool Song::IsMetadataEqual(const Song& other) const {
  return title_ == other.title_ &&
         album_ == other.album_ &&
         artist_ == other.artist_ &&
         albumartist_ == other.albumartist_ &&
         composer_ == other.composer_ &&
         track_ == other.track_ &&
         disc_ == other.disc_ &&
         qFuzzyCompare(bpm_, other.bpm_) &&
         year_ == other.year_ &&
         genre_ == other.genre_ &&
         comment_ == other.comment_ &&
         compilation_ == other.compilation_ &&
         length_ == other.length_ &&
         bitrate_ == other.bitrate_ &&
         samplerate_ == other.samplerate_;
}
