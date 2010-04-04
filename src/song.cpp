/* This file is part of Clementine.

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

#include "song.h"

#include <sys/stat.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/oggfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/flacfile.h>
#include <taglib/asffile.h>
#include <taglib/mp4file.h>
#include <taglib/mpcfile.h>
#include <taglib/aifffile.h>
#include <taglib/wavfile.h>
#include <taglib/speexfile.h>
#include <taglib/trueaudiofile.h>
#include <taglib/textidentificationframe.h>
#include <taglib/xiphcomment.h>

#include "fixlastfm.h"
#include <lastfm/Track>

#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QSqlQuery>
#include <QVariant>

#include <boost/scoped_ptr.hpp>
using boost::scoped_ptr;

#include "trackslider.h"
#include "engines/enginebase.h"
#include "albumcoverloader.h"

const char* Song::kColumnSpec =
    "title, album, artist, albumartist, composer, "
    "track, disc, bpm, year, genre, comment, compilation, "
    "length, bitrate, samplerate, directory, filename, "
    "mtime, ctime, filesize, sampler, art_automatic, art_manual, "
    "filetype, playcount, lastplayed, rating, forced_compilation_on, "
    "forced_compilation_off, effective_compilation";

const char* Song::kBindSpec =
    ":title, :album, :artist, :albumartist, :composer, "
    ":track, :disc, :bpm, :year, :genre, :comment, :compilation, "
    ":length, :bitrate, :samplerate, :directory_id, :filename, "
    ":mtime, :ctime, :filesize, :sampler, :art_automatic, :art_manual, "
    ":filetype, :playcount, :lastplayed, :rating, :forced_compilation_on, "
    ":forced_compilation_off, :effective_compilation";

const char* Song::kUpdateSpec =
    "title = :title, album = :album, artist = :artist, "
    "albumartist = :albumartist, composer = :composer, track = :track, "
    "disc = :disc, bpm = :bpm, year = :year, genre = :genre, "
    "comment = :comment, compilation = :compilation, length = :length, "
    "bitrate = :bitrate, samplerate = :samplerate, "
    "directory = :directory_id, filename = :filename, mtime = :mtime, "
    "ctime = :ctime, filesize = :filesize, sampler = :sampler, "
    "art_automatic = :art_automatic, art_manual = :art_manual, "
    "filetype = :filetype, playcount = :playcount, lastplayed = :lastplayed, "
    "rating = :rating, forced_compilation_on = :forced_compilation_on, "
    "forced_compilation_off = :forced_compilation_off, "
    "effective_compilation = :effective_compilation";

static TagLib::String QStringToTaglibString(const QString& s);

TagLibFileRefFactory Song::kDefaultFactory;

Song::Private::Private()
  : valid_(false),
    id_(-1),
    track_(-1),
    disc_(-1),
    bpm_(-1),
    year_(-1),
    compilation_(false),
    sampler_(false),
    forced_compilation_on_(false),
    forced_compilation_off_(false),
    length_(-1),
    bitrate_(-1),
    samplerate_(-1),
    directory_id_(-1),
    mtime_(-1),
    ctime_(-1),
    filesize_(-1),
    filetype_(Type_Unknown)
{
}

TagLib::FileRef* TagLibFileRefFactory::GetFileRef(const QString& filename) {
  return new TagLib::FileRef(QFile::encodeName(filename).constData());
}

Song::Song()
  : d(new Private),
    factory_(&kDefaultFactory)
{
}

Song::Song(const Song &other)
  : d(other.d),
    factory_(&kDefaultFactory)
{
}

Song::Song(FileRefFactory* factory)
    : d(new Private),
      factory_(factory) {
}

void Song::Init(const QString& title, const QString& artist, const QString& album, int length) {
  d->valid_ = true;
  d->title_ = title;
  d->artist_ = artist;
  d->album_ = album;
  d->length_ = length;
}

void Song::InitFromFile(const QString& filename, int directory_id) {
  d->filename_ = filename;
  d->directory_id_ = directory_id;

  scoped_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));

  if(fileref->isNull())
    return;

  QFileInfo info(filename);
  d->basefilename_ = info.fileName();
  d->filesize_ = info.size();
  d->mtime_ = info.lastModified().toTime_t();
  d->ctime_ = info.created().toTime_t();

  TagLib::Tag* tag = fileref->tag();
  if (tag) {
    #define strip(x) TStringToQString( x ).trimmed()
    d->title_ = strip(tag->title());
    d->artist_ = strip(tag->artist());
    d->album_ = strip(tag->album());
    d->comment_ = strip(tag->comment());
    d->genre_ = strip(tag->genre());
    d->year_ = tag->year();
    d->track_ = tag->track();
    #undef strip

    d->valid_ = true;
  }

  QString disc;
  QString compilation;
  if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    if (file->ID3v2Tag()) {
      if (!file->ID3v2Tag()->frameListMap()["TPOS"].isEmpty())
        disc = TStringToQString(file->ID3v2Tag()->frameListMap()["TPOS"].front()->toString()).trimmed();

      if (!file->ID3v2Tag()->frameListMap()["TBPM"].isEmpty())
        d->bpm_ = TStringToQString(file->ID3v2Tag()->frameListMap()["TBPM"].front()->toString()).trimmed().toFloat();

      if (!file->ID3v2Tag()->frameListMap()["TCOM"].isEmpty())
        d->composer_ = TStringToQString(file->ID3v2Tag()->frameListMap()["TCOM"].front()->toString()).trimmed();

      if (!file->ID3v2Tag()->frameListMap()["TPE2"].isEmpty()) // non-standard: Apple, Microsoft
        d->albumartist_ = TStringToQString(file->ID3v2Tag()->frameListMap()["TPE2"].front()->toString()).trimmed();

      if (!file->ID3v2Tag()->frameListMap()["TCMP"].isEmpty())
        compilation = TStringToQString(file->ID3v2Tag()->frameListMap()["TCMP"].front()->toString()).trimmed();
    }
  }
  else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref->file())) {
    if (file->tag()) {
      if ( !file->tag()->fieldListMap()["COMPOSER"].isEmpty() )
        d->composer_ = TStringToQString(file->tag()->fieldListMap()["COMPOSER"].front()).trimmed();

      if ( !file->tag()->fieldListMap()["BPM"].isEmpty() )
        d->bpm_ = TStringToQString(file->tag()->fieldListMap()["BPM"].front()).trimmed().toFloat();

      if ( !file->tag()->fieldListMap()["DISCNUMBER"].isEmpty() )
        disc = TStringToQString(file->tag()->fieldListMap()["DISCNUMBER"].front()).trimmed();

      if ( !file->tag()->fieldListMap()["COMPILATION"].isEmpty() )
        compilation = TStringToQString(file->tag()->fieldListMap()["COMPILATION"].front()).trimmed();
    }
  }
  else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    if ( file->xiphComment() ) {
      if (!file->xiphComment()->fieldListMap()["COMPOSER"].isEmpty())
        d->composer_ = TStringToQString( file->xiphComment()->fieldListMap()["COMPOSER"].front() ).trimmed();

      if (!file->xiphComment()->fieldListMap()["BPM"].isEmpty() )
        d->bpm_ = TStringToQString( file->xiphComment()->fieldListMap()["BPM"].front() ).trimmed().toFloat();

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
      d->disc_ = disc.left( i ).toInt();
    else
      d->disc_ = disc.toInt();
  }

  if ( compilation.isEmpty() ) {
    // well, it wasn't set, but if the artist is VA assume it's a compilation
    if ( d->artist_.toLower()  == "various artists" )
      d->compilation_ = true;
  } else {
    int i = compilation.toInt();
    d->compilation_ = (i == 1);
  }

  if (fileref->audioProperties()) {
    d->bitrate_    = fileref->audioProperties()->bitrate();
    d->length_     = fileref->audioProperties()->length();
    d->samplerate_ = fileref->audioProperties()->sampleRate();
  }

  // Get the filetype if we can
  GuessFileType(fileref.get());
}

void Song::GuessFileType(TagLib::FileRef* fileref) {
#ifdef TAGLIB_WITH_ASF
  if (dynamic_cast<TagLib::ASF::File*>(fileref->file()))
    d->filetype_ = Type_Asf;
#endif
  if (dynamic_cast<TagLib::FLAC::File*>(fileref->file()))
    d->filetype_ = Type_Flac;
#ifdef TAGLIB_WITH_MP4
  if (dynamic_cast<TagLib::MP4::File*>(fileref->file()))
    d->filetype_ = Type_Mp4;
#endif
  if (dynamic_cast<TagLib::MPC::File*>(fileref->file()))
    d->filetype_ = Type_Mpc;
  if (dynamic_cast<TagLib::MPEG::File*>(fileref->file()))
    d->filetype_ = Type_Mpeg;
  if (dynamic_cast<TagLib::Ogg::FLAC::File*>(fileref->file()))
    d->filetype_ = Type_OggFlac;
  if (dynamic_cast<TagLib::Ogg::Speex::File*>(fileref->file()))
    d->filetype_ = Type_OggSpeex;
  if (dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref->file()))
    d->filetype_ = Type_OggVorbis;
  if (dynamic_cast<TagLib::RIFF::AIFF::File*>(fileref->file()))
    d->filetype_ = Type_Aiff;
  if (dynamic_cast<TagLib::RIFF::WAV::File*>(fileref->file()))
    d->filetype_ = Type_Wav;
  if (dynamic_cast<TagLib::TrueAudio::File*>(fileref->file()))
    d->filetype_ = Type_TrueAudio;
}

void Song::InitFromQuery(const QSqlQuery& q) {
  if (!q.isValid())
    return;

  d->valid_ = true;

  #define tostr(n) (q.value(n).isNull() ? QString::null : q.value(n).toString())
  #define toint(n) (q.value(n).isNull() ? -1 : q.value(n).toInt())
  #define tofloat(n) (q.value(n).isNull() ? -1 : q.value(n).toDouble())

  d->id_ = toint(0);
  d->title_ = tostr(1);
  d->album_ = tostr(2);
  d->artist_ = tostr(3);
  d->albumartist_ = tostr(4);
  d->composer_ = tostr(5);
  d->track_ = toint(6);
  d->disc_ = toint(7);
  d->bpm_ = tofloat(8);
  d->year_ = toint(9);
  d->genre_ = tostr(10);
  d->comment_ = tostr(11);
  d->compilation_ = q.value(12).toBool();

  d->length_ = toint(13);
  d->bitrate_ = toint(14);
  d->samplerate_ = toint(15);

  d->directory_id_ = toint(16);
  d->filename_ = tostr(17);
  d->basefilename_ = QFileInfo(d->filename_).fileName();
  d->mtime_ = toint(18);
  d->ctime_ = toint(19);
  d->filesize_ = toint(20);

  d->sampler_ = q.value(21).toBool();

  d->art_automatic_ = q.value(22).toString();
  d->art_manual_ = q.value(23).toString();

  d->filetype_ = FileType(q.value(24).toInt());
  // playcount = 25
  // lastplayed = 26
  // rating = 27

  d->forced_compilation_on_ = q.value(28).toBool();
  d->forced_compilation_off_ = q.value(29).toBool();

  // effective_compilation = 30

  #undef tostr
  #undef toint
  #undef tofloat
}

void Song::InitFromLastFM(const lastfm::Track& track) {
  d->valid_ = true;
  d->filetype_ = Type_Stream;

  d->title_ = track.title();
  d->album_ = track.album();
  d->artist_ = track.artist();
  d->track_ = track.trackNumber();
  d->length_ = track.duration();
}

void Song::InitFromSimpleMetaBundle(const Engine::SimpleMetaBundle &bundle) {
  d->valid_ = true;

  d->title_ = bundle.title;
  d->artist_ = bundle.artist;
  d->album_ = bundle.album;
  d->comment_ = bundle.comment;
  d->genre_ = bundle.genre;
  d->bitrate_ = bundle.bitrate.toInt();
  d->samplerate_ = bundle.samplerate.toInt();
  d->length_ = bundle.length.toInt();
  d->year_ = bundle.year.toInt();
  d->track_ = bundle.tracknr.toInt();
}

void Song::BindToQuery(QSqlQuery *query) const {
  #define intval(x) (x == -1 ? QVariant() : x)

  // Remember to bind these in the same order as kBindSpec

  query->bindValue(":title", d->title_);
  query->bindValue(":album", d->album_);
  query->bindValue(":artist", d->artist_);
  query->bindValue(":albumartist", d->albumartist_);
  query->bindValue(":composer", d->composer_);
  query->bindValue(":track", intval(d->track_));
  query->bindValue(":disc", intval(d->disc_));
  query->bindValue(":bpm", intval(d->bpm_));
  query->bindValue(":year", intval(d->year_));
  query->bindValue(":genre", d->genre_);
  query->bindValue(":comment", d->comment_);
  query->bindValue(":compilation", d->compilation_ ? 1 : 0);

  query->bindValue(":length", intval(d->length_));
  query->bindValue(":bitrate", intval(d->bitrate_));
  query->bindValue(":samplerate", intval(d->samplerate_));

  query->bindValue(":directory_id", intval(d->directory_id_));
  query->bindValue(":filename", d->filename_);
  query->bindValue(":mtime", intval(d->mtime_));
  query->bindValue(":ctime", intval(d->ctime_));
  query->bindValue(":filesize", intval(d->filesize_));

  query->bindValue(":sampler", d->sampler_ ? 1 : 0);
  query->bindValue(":art_automatic", d->art_automatic_);
  query->bindValue(":art_manual", d->art_manual_);

  query->bindValue(":filetype", d->filetype_);
  query->bindValue(":playcount", 0); // TODO
  query->bindValue(":lastplayed", -1); // TODO
  query->bindValue(":rating", -1);

  query->bindValue(":forced_compilation_on", d->forced_compilation_on_ ? 1 : 0);
  query->bindValue(":forced_compilation_off", d->forced_compilation_off_ ? 1 : 0);

  query->bindValue(":effective_compilation", is_compilation() ? 1 : 0);

  #undef intval
}

void Song::ToLastFM(lastfm::Track* track) const {
  lastfm::MutableTrack mtrack(*track);

  mtrack.setArtist(d->artist_);
  mtrack.setAlbum(d->album_);
  mtrack.setTitle(d->title_);
  mtrack.setDuration(d->length_);
  mtrack.setTrackNumber(d->track_);
  mtrack.setSource(lastfm::Track::Player);
}

QString Song::PrettyTitleWithArtist() const {
  QString title(d->title_);

  if (title.isEmpty())
    title = QFileInfo(d->filename_).baseName();

  if (is_compilation() && !d->artist_.isEmpty() && !d->artist_.toLower().contains("various"))
    title = d->artist_ + " - " + title;

  return title;
}

QString Song::PrettyTitle() const {
  QString title(d->title_);

  if (title.isEmpty())
    title = QFileInfo(d->filename_).baseName();

  return title;
}

QString Song::PrettyLength() const {
  if (d->length_ == -1)
    return QString::null;

  return TrackSlider::PrettyTime(d->length_);
}

bool Song::IsMetadataEqual(const Song& other) const {
  return d->title_ == other.d->title_ &&
         d->album_ == other.d->album_ &&
         d->artist_ == other.d->artist_ &&
         d->albumartist_ == other.d->albumartist_ &&
         d->composer_ == other.d->composer_ &&
         d->track_ == other.d->track_ &&
         d->disc_ == other.d->disc_ &&
         qFuzzyCompare(d->bpm_, other.d->bpm_) &&
         d->year_ == other.d->year_ &&
         d->genre_ == other.d->genre_ &&
         d->comment_ == other.d->comment_ &&
         d->compilation_ == other.d->compilation_ &&
         d->length_ == other.d->length_ &&
         d->bitrate_ == other.d->bitrate_ &&
         d->samplerate_ == other.d->samplerate_ &&
         d->sampler_ == other.d->sampler_ &&
         d->art_automatic_ == other.d->art_automatic_ &&
         d->art_manual_ == other.d->art_manual_;
}

void Song::SetTextFrame(const QString& id, const QString& value,
                        TagLib::ID3v2::Tag* tag) {
  TagLib::ByteVector id_vector = id.toUtf8().constData();

  // Remove the frame if it already exists
  while (tag->frameListMap().contains(id_vector) &&
         tag->frameListMap()[id_vector].size() != 0) {
    tag->removeFrame(tag->frameListMap()[id_vector].front());
  }

  // Create and add a new frame
  TagLib::ID3v2::TextIdentificationFrame* frame =
      new TagLib::ID3v2::TextIdentificationFrame(id.toUtf8().constData(),
                                                 TagLib::String::UTF8);
  frame->setText(QStringToTaglibString(value));
  tag->addFrame(frame);
}

TagLib::String QStringToTaglibString(const QString& s) {
  return TagLib::String(s.toUtf8().constData(), TagLib::String::UTF8);
}

bool Song::Save() const {
  if (d->filename_.isNull())
    return false;

  scoped_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(d->filename_));

  fileref->tag()->setTitle(QStringToTaglibString(d->title_));
  fileref->tag()->setArtist(QStringToTaglibString(d->artist_));
  fileref->tag()->setAlbum(QStringToTaglibString(d->album_));
  fileref->tag()->setGenre(QStringToTaglibString(d->genre_));
  fileref->tag()->setComment(QStringToTaglibString(d->comment_));
  fileref->tag()->setYear(d->year_);
  fileref->tag()->setTrack(d->track_);

  if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    TagLib::ID3v2::Tag* tag = file->ID3v2Tag(true);
    SetTextFrame("TPOS", d->disc_ <= 0 -1 ? QString() : QString::number(d->disc_), tag);
    SetTextFrame("TBPM", d->bpm_ <= 0 -1 ? QString() : QString::number(d->bpm_), tag);
    SetTextFrame("TCOM", d->composer_, tag);
    SetTextFrame("TPE2", d->albumartist_, tag);
    SetTextFrame("TCMP", d->compilation_ ? "1" : "0", tag);
  }
  else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref->file())) {
    TagLib::Ogg::XiphComment* tag = file->tag();
    tag->addField("COMPOSER", QStringToTaglibString(d->composer_), true);
    tag->addField("BPM", QStringToTaglibString(d->bpm_ <= 0 -1 ? QString() : QString::number(d->bpm_)), true);
    tag->addField("DISCNUMBER", QStringToTaglibString(d->disc_ <= 0 -1 ? QString() : QString::number(d->disc_)), true);
    tag->addField("COMPILATION", QStringToTaglibString(d->compilation_ ? "1" : "0"), true);
  }
  else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    TagLib::Ogg::XiphComment* tag = file->xiphComment();
    tag->addField("COMPOSER", QStringToTaglibString(d->composer_), true);
    tag->addField("BPM", QStringToTaglibString(d->bpm_ <= 0 -1 ? QString() : QString::number(d->bpm_)), true);
    tag->addField("DISCNUMBER", QStringToTaglibString(d->disc_ <= 0 -1 ? QString() : QString::number(d->disc_)), true);
    tag->addField("COMPILATION", QStringToTaglibString(d->compilation_ ? "1" : "0"), true);
  }

  bool ret = fileref->save();
  #ifdef Q_OS_LINUX
  if (ret) {
    // Linux: inotify doesn't seem to notice the change to the file unless we
    // change the timestamps as well. (this is what touch does)
    utimensat(0, QFile::encodeName(d->filename_).constData(), NULL, 0);
  }
  #endif  // Q_OS_LINUX

  return ret;
}

QImage Song::GetBestImage() const {
  if (!d->image_.isNull())
    return d->image_;

  QImage art(AlbumCoverLoader::TryLoadImage(d->art_automatic_, d->art_manual_));
  if (!art.isNull())
    return art;

  return QImage(":/nocover.png");
}
