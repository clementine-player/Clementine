/* This file is part of Clementine.
   Copyright 2009-2012, 2014, David Sansome <me@davidsansome.com>
   Copyright 2010-2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2010, 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011-2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2011, Angus Gratton <gus@projectgus.com>
   Copyright 2012, Kacper "mattrick" Banasik <mattrick@jabster.pl>
   Copyright 2013, Martin Brodbeck <martin@brodbeck-online.de>
   Copyright 2013-2014, Andreas <asfa194@gmail.com>
   Copyright 2013, Joel Bradshaw <cincodenada@gmail.com>
   Copyright 2013, Uwe Klotz <uwe.klotz@gmail.com>
   Copyright 2013, Mateusz Kowalczyk <fuuzetsu@fuuzetsu.co.uk>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include <algorithm>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLatin1Literal>
#include <QSharedData>
#include <QSqlQuery>
#include <QTextCodec>
#include <QTime>
#include <QVariant>
#include <QtConcurrentRun>

#ifdef HAVE_LIBLASTFM
#include "internet/lastfm/fixlastfm.h"
#ifdef HAVE_LIBLASTFM1
#include <lastfm/Track.h>
#else
#include <lastfm/Track>
#endif
#endif

#include <fileref.h>
#include <id3v1genres.h>

#ifdef HAVE_LIBGPOD
#include <gpod/itdb.h>
#endif

#ifdef HAVE_LIBMTP
#include <libmtp.h>
#endif

#include "core/application.h"
#include "core/logging.h"
#include "core/messagehandler.h"
#include "core/mpris_common.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "covers/albumcoverloader.h"
#include "engines/enginebase.h"
#include "gmereader.h"
#include "library/sqlrow.h"
#include "tagreadermessages.pb.h"
#include "widgets/trackslider.h"

const QStringList Song::kColumns = QStringList() << "title"
                                                 << "album"
                                                 << "artist"
                                                 << "albumartist"
                                                 << "composer"
                                                 << "track"
                                                 << "disc"
                                                 << "bpm"
                                                 << "year"
                                                 << "genre"
                                                 << "comment"
                                                 << "compilation"
                                                 << "bitrate"
                                                 << "samplerate"
                                                 << "directory"
                                                 << "filename"
                                                 << "mtime"
                                                 << "ctime"
                                                 << "filesize"
                                                 << "sampler"
                                                 << "art_automatic"
                                                 << "art_manual"
                                                 << "filetype"
                                                 << "playcount"
                                                 << "lastplayed"
                                                 << "rating"
                                                 << "forced_compilation_on"
                                                 << "forced_compilation_off"
                                                 << "effective_compilation"
                                                 << "skipcount"
                                                 << "score"
                                                 << "beginning"
                                                 << "length"
                                                 << "cue_path"
                                                 << "unavailable"
                                                 << "effective_albumartist"
                                                 << "etag"
                                                 << "performer"
                                                 << "grouping"
                                                 << "lyrics"
                                                 << "originalyear"
                                                 << "effective_originalyear";

const QStringList Song::kIntColumns = QStringList() << "track"
                                                    << "disc"
                                                    << "year"
                                                    << "originalyear"
                                                    << "playcount"
                                                    << "skipcount"
                                                    << "score"
                                                    << "bitrate"
                                                    << "samplerate";

const QStringList Song::kFloatColumns = QStringList() << "rating"
                                                      << "bpm";

const QStringList Song::kDateColumns = QStringList() << "lastplayed"
                                                     << "mtime"
                                                     << "ctime";

const QString Song::kColumnSpec = Song::kColumns.join(", ");
const QString Song::kBindSpec =
    Utilities::Prepend(":", Song::kColumns).join(", ");
const QString Song::kUpdateSpec =
    Utilities::Updateify(Song::kColumns).join(", ");

const QStringList Song::kFtsColumns = QStringList() << "ftstitle"
                                                    << "ftsalbum"
                                                    << "ftsartist"
                                                    << "ftsalbumartist"
                                                    << "ftscomposer"
                                                    << "ftsperformer"
                                                    << "ftsgrouping"
                                                    << "ftsgenre"
                                                    << "ftscomment"
                                                    << "ftsyear";

const QString Song::kFtsColumnSpec = Song::kFtsColumns.join(", ");
const QString Song::kFtsBindSpec =
    Utilities::Prepend(":", Song::kFtsColumns).join(", ");
const QString Song::kFtsUpdateSpec =
    Utilities::Updateify(Song::kFtsColumns).join(", ");

const QString Song::kManuallyUnsetCover = "(unset)";
const QString Song::kEmbeddedCover = "(embedded)";

struct Song::Private : public QSharedData {
  Private();

  bool valid_;
  int id_;

  QString title_;
  QString album_;
  QString artist_;
  QString albumartist_;
  QString composer_;
  QString performer_;
  QString grouping_;
  QString lyrics_;
  int track_;
  int disc_;
  float bpm_;
  int year_;
  int originalyear_;
  QString genre_;
  QString comment_;
  bool compilation_;             // From the file tag
  bool sampler_;                 // From the library scanner
  bool forced_compilation_on_;   // Set by the user
  bool forced_compilation_off_;  // Set by the user

  // A unique album ID
  // Used to distinguish between albums from providers that have multiple
  // versions of a given album with the same title (e.g. Spotify).
  // This is never persisted, it is only stored temporarily for global search
  // results.
  int album_id_;

  float rating_;
  int playcount_;
  int skipcount_;
  int lastplayed_;
  int score_;

  // The beginning of the song in seconds. In case of single-part media
  // streams, this will equal to 0. In case of multi-part streams on the
  // other hand, this will mark the beginning of a section represented by
  // this Song object. This is always greater than 0.
  qint64 beginning_;
  // The end of the song in seconds. In case of single-part media
  // streams, this will equal to the song's length. In case of multi-part
  // streams on the other hand, this will mark the end of a section
  // represented by this Song object.
  // This may be negative indicating that the length of this song is
  // unknown.
  qint64 end_;

  int bitrate_;
  int samplerate_;

  int directory_id_;
  QUrl url_;
  QString basefilename_;
  int mtime_;
  int ctime_;
  int filesize_;
  FileType filetype_;

  // If the song has a CUE, this contains it's path.
  QString cue_path_;

  // Filenames to album art for this song.
  QString art_automatic_;  // Guessed by LibraryWatcher
  QString art_manual_;     // Set by the user - should take priority

  QImage image_;

  // Whether this song was loaded from a file using taglib.
  bool init_from_file_;
  // Whether our encoding guesser thinks these tags might be incorrectly
  // encoded.
  bool suspicious_tags_;

  // Whether the song does not exist on the file system anymore, but is still
  // stored in the database so as to remember the user's metadata.
  bool unavailable_;

  QString etag_;
};

Song::Private::Private()
    : valid_(false),
      id_(-1),
      track_(-1),
      disc_(-1),
      bpm_(-1),
      year_(-1),
      originalyear_(-1),
      compilation_(false),
      sampler_(false),
      forced_compilation_on_(false),
      forced_compilation_off_(false),
      album_id_(-1),
      rating_(-1.0),
      playcount_(0),
      skipcount_(0),
      lastplayed_(-1),
      score_(0),
      beginning_(0),
      end_(-1),
      bitrate_(-1),
      samplerate_(-1),
      directory_id_(-1),
      mtime_(-1),
      ctime_(-1),
      filesize_(-1),
      filetype_(Type_Unknown),
      init_from_file_(false),
      suspicious_tags_(false),
      unavailable_(false) {}

Song::Song() : d(new Private) {}

Song::Song(const Song& other) : d(other.d) {}

Song::~Song() {}

Song& Song::operator=(const Song& other) {
  d = other.d;
  return *this;
}

bool Song::is_valid() const { return d->valid_; }
bool Song::is_unavailable() const { return d->unavailable_; }
int Song::id() const { return d->id_; }
const QString& Song::title() const { return d->title_; }
const QString& Song::album() const { return d->album_; }
const QString& Song::effective_album() const {
  // This value is useful for singles, which are one-track albums on their own.
  return d->album_.isEmpty() ? d->title_ : d->album_;
}
const QString& Song::artist() const { return d->artist_; }
const QString& Song::albumartist() const { return d->albumartist_; }
const QString& Song::effective_albumartist() const {
  return d->albumartist_.isEmpty() ? d->artist_ : d->albumartist_;
}
const QString& Song::playlist_albumartist() const {
  return is_compilation() ? d->albumartist_ : effective_albumartist();
}
const QString& Song::composer() const { return d->composer_; }
const QString& Song::performer() const { return d->performer_; }
const QString& Song::grouping() const { return d->grouping_; }
const QString& Song::lyrics() const { return d->lyrics_; }
int Song::track() const { return d->track_; }
int Song::disc() const { return d->disc_; }
float Song::bpm() const { return d->bpm_; }
int Song::year() const { return d->year_; }
int Song::originalyear() const { return d->originalyear_; }
int Song::effective_originalyear() const {
  return d->originalyear_ < 0 ? d->year_ : d->originalyear_;
}
const QString& Song::genre() const { return d->genre_; }
const QString& Song::comment() const { return d->comment_; }
bool Song::is_compilation() const {
  return (d->compilation_ || d->sampler_ || d->forced_compilation_on_) &&
         !d->forced_compilation_off_;
}
float Song::rating() const { return d->rating_; }
int Song::playcount() const { return d->playcount_; }
int Song::skipcount() const { return d->skipcount_; }
int Song::lastplayed() const { return d->lastplayed_; }
int Song::score() const { return d->score_; }
const QString& Song::cue_path() const { return d->cue_path_; }
bool Song::has_cue() const { return !d->cue_path_.isEmpty(); }
int Song::album_id() const { return d->album_id_; }
qint64 Song::beginning_nanosec() const { return d->beginning_; }
qint64 Song::end_nanosec() const { return d->end_; }
qint64 Song::length_nanosec() const { return d->end_ - d->beginning_; }
int Song::bitrate() const { return d->bitrate_; }
int Song::samplerate() const { return d->samplerate_; }
int Song::directory_id() const { return d->directory_id_; }
const QUrl& Song::url() const { return d->url_; }
const QString& Song::basefilename() const { return d->basefilename_; }
uint Song::mtime() const { return d->mtime_; }
uint Song::ctime() const { return d->ctime_; }
int Song::filesize() const { return d->filesize_; }
Song::FileType Song::filetype() const { return d->filetype_; }
bool Song::is_stream() const { return d->filetype_ == Type_Stream; }
bool Song::is_cdda() const { return d->filetype_ == Type_Cdda; }
bool Song::is_library_song() const {
  return !is_cdda() && !is_stream() && id() != -1;
}
const QString& Song::art_automatic() const { return d->art_automatic_; }
const QString& Song::art_manual() const { return d->art_manual_; }
const QString& Song::etag() const { return d->etag_; }
bool Song::has_manually_unset_cover() const {
  return d->art_manual_ == kManuallyUnsetCover;
}
void Song::manually_unset_cover() { d->art_manual_ = kManuallyUnsetCover; }
bool Song::has_embedded_cover() const {
  return d->art_automatic_ == kEmbeddedCover;
}
void Song::set_embedded_cover() { d->art_automatic_ = kEmbeddedCover; }
const QImage& Song::image() const { return d->image_; }
void Song::set_id(int id) { d->id_ = id; }
void Song::set_valid(bool v) { d->valid_ = v; }
void Song::set_title(const QString& v) { d->title_ = v; }
void Song::set_album(const QString& v) { d->album_ = v; }
void Song::set_artist(const QString& v) { d->artist_ = v; }
void Song::set_albumartist(const QString& v) { d->albumartist_ = v; }
void Song::set_composer(const QString& v) { d->composer_ = v; }
void Song::set_performer(const QString& v) { d->performer_ = v; }
void Song::set_grouping(const QString& v) { d->grouping_ = v; }
void Song::set_lyrics(const QString& v) { d->lyrics_ = v; }
void Song::set_track(int v) { d->track_ = v; }
void Song::set_disc(int v) { d->disc_ = v; }
void Song::set_bpm(float v) { d->bpm_ = v; }
void Song::set_year(int v) { d->year_ = v; }
void Song::set_originalyear(int v) { d->originalyear_ = v; }
void Song::set_genre(const QString& v) { d->genre_ = v; }
void Song::set_comment(const QString& v) { d->comment_ = v; }
void Song::set_compilation(bool v) { d->compilation_ = v; }
void Song::set_sampler(bool v) { d->sampler_ = v; }
void Song::set_album_id(int v) { d->album_id_ = v; }
void Song::set_beginning_nanosec(qint64 v) { d->beginning_ = qMax(0ll, v); }
void Song::set_end_nanosec(qint64 v) { d->end_ = v; }
void Song::set_length_nanosec(qint64 v) { d->end_ = d->beginning_ + v; }
void Song::set_bitrate(int v) { d->bitrate_ = v; }
void Song::set_samplerate(int v) { d->samplerate_ = v; }
void Song::set_mtime(int v) { d->mtime_ = v; }
void Song::set_ctime(int v) { d->ctime_ = v; }
void Song::set_filesize(int v) { d->filesize_ = v; }
void Song::set_filetype(FileType v) { d->filetype_ = v; }
void Song::set_art_automatic(const QString& v) { d->art_automatic_ = v; }
void Song::set_art_manual(const QString& v) { d->art_manual_ = v; }
void Song::set_image(const QImage& i) { d->image_ = i; }
void Song::set_forced_compilation_on(bool v) { d->forced_compilation_on_ = v; }
void Song::set_forced_compilation_off(bool v) {
  d->forced_compilation_off_ = v;
}
void Song::set_rating(float v) { d->rating_ = v; }
void Song::set_playcount(int v) { d->playcount_ = v; }
void Song::set_skipcount(int v) { d->skipcount_ = v; }
void Song::set_lastplayed(int v) { d->lastplayed_ = v; }
void Song::set_score(int v) { d->score_ = qBound(0, v, 100); }
void Song::set_cue_path(const QString& v) { d->cue_path_ = v; }
void Song::set_unavailable(bool v) { d->unavailable_ = v; }
void Song::set_etag(const QString& etag) { d->etag_ = etag; }

void Song::set_url(const QUrl& v) {
  if (Application::kIsPortable) {
    QUrl base =
        QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/");
    d->url_ = base.resolved(v);
  } else {
    d->url_ = v;
  }
}

void Song::set_basefilename(const QString& v) { d->basefilename_ = v; }
void Song::set_directory_id(int v) { d->directory_id_ = v; }

QString Song::JoinSpec(const QString& table) {
  return Utilities::Prepend(table + ".", kColumns).join(", ");
}

QString Song::TextForFiletype(FileType type) {
  switch (type) {
    case Song::Type_Asf:
      return QObject::tr("Windows Media audio");
    case Song::Type_Flac:
      return QObject::tr("FLAC");
    case Song::Type_Mp4:
      return QObject::tr("MP4 AAC");
    case Song::Type_Mpc:
      return QObject::tr("MPC");
    case Song::Type_Mpeg:
      return QObject::tr("MP3");  // Not technically correct
    case Song::Type_OggFlac:
      return QObject::tr("Ogg FLAC");
    case Song::Type_OggSpeex:
      return QObject::tr("Ogg Speex");
    case Song::Type_OggVorbis:
      return QObject::tr("Ogg Vorbis");
    case Song::Type_OggOpus:
      return QObject::tr("Ogg Opus");
    case Song::Type_Aiff:
      return QObject::tr("AIFF");
    case Song::Type_Wav:
      return QObject::tr("Wav");
    case Song::Type_WavPack:
      return QObject::tr("WavPack");
    case Song::Type_TrueAudio:
      return QObject::tr("TrueAudio");
    case Song::Type_Cdda:
      return QObject::tr("CDDA");
    case Song::Type_Spc:
      return QObject::tr("SNES SPC700");
    case Song::Type_VGM:
      return QObject::tr("VGM");

    case Song::Type_Stream:
      return QObject::tr("Stream");

    case Song::Type_Unknown:
    default:
      return QObject::tr("Unknown");
  }
}

bool Song::IsFileLossless() const {
  switch (filetype()) {
    case Song::Type_Aiff:
    case Song::Type_Flac:
    case Song::Type_OggFlac:
    case Song::Type_Wav:
    case Song::Type_WavPack:
      return true;
    default:
      return false;
  }
}

int CompareSongsName(const Song& song1, const Song& song2) {
  return song1.PrettyTitleWithArtist().localeAwareCompare(
             song2.PrettyTitleWithArtist()) < 0;
}

void Song::SortSongsListAlphabetically(SongList* songs) {
  Q_ASSERT(songs);
  std::sort(songs->begin(), songs->end(), CompareSongsName);
}

void Song::Init(const QString& title, const QString& artist,
                const QString& album, qint64 length_nanosec) {
  d->valid_ = true;

  d->title_ = title;
  d->artist_ = artist;
  d->album_ = album;

  set_length_nanosec(length_nanosec);
}

void Song::Init(const QString& title, const QString& artist,
                const QString& album, qint64 beginning, qint64 end) {
  d->valid_ = true;

  d->title_ = title;
  d->artist_ = artist;
  d->album_ = album;

  d->beginning_ = beginning;
  d->end_ = end;
}

void Song::set_genre_id3(int id) {
  set_genre(TStringToQString(TagLib::ID3v1::genre(id)));
}

QString Song::Decode(const QString& tag, const QTextCodec* codec) {
  if (!codec) {
    return tag;
  }

  return codec->toUnicode(tag.toUtf8());
}

void Song::InitFromProtobuf(const pb::tagreader::SongMetadata& pb) {
  d->init_from_file_ = true;
  d->valid_ = pb.valid();
  d->title_ = QStringFromStdString(pb.title());
  d->album_ = QStringFromStdString(pb.album());
  d->artist_ = QStringFromStdString(pb.artist());
  d->albumartist_ = QStringFromStdString(pb.albumartist());
  d->composer_ = QStringFromStdString(pb.composer());
  d->performer_ = QStringFromStdString(pb.performer());
  d->grouping_ = QStringFromStdString(pb.grouping());
  d->lyrics_ = QStringFromStdString(pb.lyrics());
  d->track_ = pb.track();
  d->disc_ = pb.disc();
  d->bpm_ = pb.bpm();
  d->year_ = pb.year();
  d->originalyear_ = pb.originalyear();
  d->genre_ = QStringFromStdString(pb.genre());
  d->comment_ = QStringFromStdString(pb.comment());
  d->compilation_ = pb.compilation();
  d->skipcount_ = pb.skipcount();
  d->lastplayed_ = pb.lastplayed();
  d->score_ = pb.score();
  set_length_nanosec(pb.length_nanosec());
  d->bitrate_ = pb.bitrate();
  d->samplerate_ = pb.samplerate();
  set_url(QUrl::fromEncoded(QByteArray(pb.url().data(), pb.url().size())));
  d->basefilename_ = QStringFromStdString(pb.basefilename());
  d->mtime_ = pb.mtime();
  d->ctime_ = pb.ctime();
  d->filesize_ = pb.filesize();
  d->suspicious_tags_ = pb.suspicious_tags();
  d->filetype_ = static_cast<FileType>(pb.type());
  d->etag_ = QStringFromStdString(pb.etag());

  if (pb.has_art_automatic()) {
    d->art_automatic_ = QStringFromStdString(pb.art_automatic());
  }

  if (pb.has_rating()) {
    d->rating_ = pb.rating();
  }

  if (pb.has_playcount()) {
    d->playcount_ = pb.playcount();
  }

  InitArtManual();
}

void Song::ToProtobuf(pb::tagreader::SongMetadata* pb) const {
  const QByteArray url(d->url_.toEncoded());

  pb->set_valid(d->valid_);
  pb->set_title(DataCommaSizeFromQString(d->title_));
  pb->set_album(DataCommaSizeFromQString(d->album_));
  pb->set_artist(DataCommaSizeFromQString(d->artist_));
  pb->set_albumartist(DataCommaSizeFromQString(d->albumartist_));
  pb->set_composer(DataCommaSizeFromQString(d->composer_));
  pb->set_performer(DataCommaSizeFromQString(d->performer_));
  pb->set_grouping(DataCommaSizeFromQString(d->grouping_));
  pb->set_lyrics(DataCommaSizeFromQString(d->lyrics_));
  pb->set_track(d->track_);
  pb->set_disc(d->disc_);
  pb->set_bpm(d->bpm_);
  pb->set_year(d->year_);
  pb->set_originalyear(d->originalyear_);
  pb->set_genre(DataCommaSizeFromQString(d->genre_));
  pb->set_comment(DataCommaSizeFromQString(d->comment_));
  pb->set_compilation(d->compilation_);
  pb->set_rating(d->rating_);
  pb->set_playcount(d->playcount_);
  pb->set_skipcount(d->skipcount_);
  pb->set_lastplayed(d->lastplayed_);
  pb->set_score(d->score_);
  pb->set_length_nanosec(length_nanosec());
  pb->set_bitrate(d->bitrate_);
  pb->set_samplerate(d->samplerate_);
  pb->set_url(url.constData(), url.size());
  pb->set_basefilename(DataCommaSizeFromQString(d->basefilename_));
  pb->set_mtime(d->mtime_);
  pb->set_ctime(d->ctime_);
  pb->set_filesize(d->filesize_);
  pb->set_suspicious_tags(d->suspicious_tags_);
  pb->set_art_automatic(DataCommaSizeFromQString(d->art_automatic_));
  pb->set_type(static_cast<pb::tagreader::SongMetadata_Type>(d->filetype_));
}

void Song::InitFromQuery(const SqlRow& q, bool reliable_metadata, int col) {
  d->valid_ = true;
  d->init_from_file_ = reliable_metadata;

#define tostr(n) (q.value(n).isNull() ? QString::null : q.value(n).toString())
#define toint(n) (q.value(n).isNull() ? -1 : q.value(n).toInt())
#define tolonglong(n) (q.value(n).isNull() ? -1 : q.value(n).toLongLong())
#define tofloat(n) (q.value(n).isNull() ? -1 : q.value(n).toDouble())

  d->id_ = toint(col + 0);
  d->title_ = tostr(col + 1);
  d->album_ = tostr(col + 2);
  d->artist_ = tostr(col + 3);
  d->albumartist_ = tostr(col + 4);
  d->composer_ = tostr(col + 5);
  d->track_ = toint(col + 6);
  d->disc_ = toint(col + 7);
  d->bpm_ = tofloat(col + 8);
  d->year_ = toint(col + 9);
  d->originalyear_ = toint(col + 41);
  d->genre_ = tostr(col + 10);
  d->comment_ = tostr(col + 11);
  d->compilation_ = q.value(col + 12).toBool();

  d->bitrate_ = toint(col + 13);
  d->samplerate_ = toint(col + 14);

  d->directory_id_ = toint(col + 15);
  set_url(QUrl::fromEncoded(tostr(col + 16).toUtf8()));
  d->basefilename_ = QFileInfo(d->url_.toLocalFile()).fileName();
  d->mtime_ = toint(col + 17);
  d->ctime_ = toint(col + 18);
  d->filesize_ = toint(col + 19);

  d->sampler_ = q.value(col + 20).toBool();

  d->art_automatic_ = q.value(col + 21).toString();
  d->art_manual_ = q.value(col + 22).toString();

  d->filetype_ = FileType(q.value(col + 23).toInt());
  d->playcount_ = q.value(col + 24).isNull() ? 0 : q.value(col + 24).toInt();
  d->lastplayed_ = toint(col + 25);
  d->rating_ = tofloat(col + 26);

  d->forced_compilation_on_ = q.value(col + 27).toBool();
  d->forced_compilation_off_ = q.value(col + 28).toBool();

  // effective_compilation = 29

  d->skipcount_ = q.value(col + 30).isNull() ? 0 : q.value(col + 30).toInt();
  d->score_ = q.value(col + 31).isNull() ? 0 : q.value(col + 31).toInt();

  // do not move those statements - beginning must be initialized before
  // length is!
  d->beginning_ =
      q.value(col + 32).isNull() ? 0 : q.value(col + 32).toLongLong();
  set_length_nanosec(tolonglong(col + 33));

  d->cue_path_ = tostr(col + 34);
  d->unavailable_ = q.value(col + 35).toBool();

  // effective_albumartist = 36
  // etag = 37

  d->performer_ = tostr(col + 38);
  d->grouping_ = tostr(col + 39);
  d->lyrics_ = tostr(col + 40);

  InitArtManual();

#undef tostr
#undef toint
#undef tolonglong
#undef tofloat
}

void Song::InitFromFilePartial(const QString& filename) {
  set_url(QUrl::fromLocalFile(filename));
  QFileInfo info(filename);
  d->basefilename_ = info.fileName();
  QString suffix = info.suffix().toLower();

  TagLib::FileRef fileref(filename.toUtf8().constData());
  if (fileref.file() || GME::IsSupportedFormat(info))
    d->valid_ = true;
  else {
    d->valid_ = false;
    qLog(Error) << "File" << filename
                << "is not recognized by TagLib as a valid audio file.";
  }
}

void Song::InitArtManual() {
  // If we don't have an art, check if we have one in the cache
  if (d->art_manual_.isEmpty() && d->art_automatic_.isEmpty()) {
    QString filename(Utilities::Sha1CoverHash(d->artist_, d->album_).toHex() +
                     ".jpg");
    QString path(AlbumCoverLoader::ImageCacheDir() + "/" + filename);
    if (QFile::exists(path)) {
      d->art_manual_ = path;
    }
  }
}

#ifdef HAVE_LIBLASTFM
void Song::InitFromLastFM(const lastfm::Track& track) {
  d->valid_ = true;
  d->filetype_ = Type_Stream;

  d->title_ = track.title();
  d->album_ = track.album();
  d->artist_ = track.artist();
  d->track_ = track.trackNumber();

  set_length_nanosec(track.duration() * kNsecPerSec);
}
#endif  // HAVE_LIBLASTFM

#ifdef HAVE_LIBGPOD
void Song::InitFromItdb(const Itdb_Track* track, const QString& prefix) {
  d->valid_ = true;

  d->title_ = QString::fromUtf8(track->title);
  d->album_ = QString::fromUtf8(track->album);
  d->artist_ = QString::fromUtf8(track->artist);
  d->albumartist_ = QString::fromUtf8(track->albumartist);
  d->composer_ = QString::fromUtf8(track->composer);
  d->grouping_ = QString::fromUtf8(track->grouping);
  d->track_ = track->track_nr;
  d->disc_ = track->cd_nr;
  d->bpm_ = track->BPM;
  d->year_ = track->year;
  d->genre_ = QString::fromUtf8(track->genre);
  d->comment_ = QString::fromUtf8(track->comment);
  d->compilation_ = track->compilation;
  set_length_nanosec(track->tracklen * kNsecPerMsec);
  d->bitrate_ = track->bitrate;
  d->samplerate_ = track->samplerate;
  d->mtime_ = track->time_modified;
  d->ctime_ = track->time_added;
  d->filesize_ = track->size;
  d->filetype_ = track->type2 ? Type_Mpeg : Type_Mp4;
  d->rating_ = static_cast<float>(track->rating) / 100;  // 100 = 20 * 5 stars
  d->playcount_ = track->playcount;
  d->skipcount_ = track->skipcount;
  d->lastplayed_ = track->time_played;

  QString filename = QString::fromLocal8Bit(track->ipod_path);
  filename.replace(':', '/');

  if (prefix.contains("://")) {
    set_url(QUrl(prefix + filename));
  } else {
    set_url(QUrl::fromLocalFile(prefix + filename));
  }

  d->basefilename_ = QFileInfo(filename).fileName();
}

void Song::ToItdb(Itdb_Track* track) const {
  track->title = strdup(d->title_.toUtf8().constData());
  track->album = strdup(d->album_.toUtf8().constData());
  track->artist = strdup(d->artist_.toUtf8().constData());
  track->albumartist = strdup(d->albumartist_.toUtf8().constData());
  track->composer = strdup(d->composer_.toUtf8().constData());
  track->grouping = strdup(d->grouping_.toUtf8().constData());
  track->track_nr = d->track_;
  track->cd_nr = d->disc_;
  track->BPM = d->bpm_;
  track->year = d->year_;
  track->genre = strdup(d->genre_.toUtf8().constData());
  track->comment = strdup(d->comment_.toUtf8().constData());
  track->compilation = d->compilation_;
  track->tracklen = length_nanosec() / kNsecPerMsec;
  track->bitrate = d->bitrate_;
  track->samplerate = d->samplerate_;
  track->time_modified = d->mtime_;
  track->time_added = d->ctime_;
  track->size = d->filesize_;
  track->type1 = 0;
  track->type2 = d->filetype_ == Type_Mp4 ? 0 : 1;
  track->mediatype = 1;              // Audio
  track->rating = d->rating_ * 100;  // 100 = 20 * 5 stars
  track->playcount = d->playcount_;
  track->skipcount = d->skipcount_;
  track->time_played = d->lastplayed_;
}
#endif

#ifdef HAVE_LIBMTP
void Song::InitFromMTP(const LIBMTP_track_t* track, const QString& host) {
  d->valid_ = true;

  d->title_ = QString::fromUtf8(track->title);
  d->artist_ = QString::fromUtf8(track->artist);
  d->album_ = QString::fromUtf8(track->album);
  d->composer_ = QString::fromUtf8(track->composer);
  d->genre_ = QString::fromUtf8(track->genre);
  d->url_ = QUrl(QString("mtp://%1/%2").arg(host, track->item_id));
  d->basefilename_ = QString::number(track->item_id);

  d->track_ = track->tracknumber;
  set_length_nanosec(track->duration * kNsecPerMsec);
  d->samplerate_ = track->samplerate;
  d->bitrate_ = track->bitrate;
  d->filesize_ = track->filesize;
  d->mtime_ = track->modificationdate;
  d->ctime_ = track->modificationdate;

  d->rating_ = static_cast<float>(track->rating) / 100;
  d->playcount_ = track->usecount;

  switch (track->filetype) {
    case LIBMTP_FILETYPE_WAV:
      d->filetype_ = Type_Wav;
      break;
    case LIBMTP_FILETYPE_MP3:
      d->filetype_ = Type_Mpeg;
      break;
    case LIBMTP_FILETYPE_WMA:
      d->filetype_ = Type_Asf;
      break;
    case LIBMTP_FILETYPE_OGG:
      d->filetype_ = Type_OggVorbis;
      break;
    case LIBMTP_FILETYPE_MP4:
      d->filetype_ = Type_Mp4;
      break;
    case LIBMTP_FILETYPE_AAC:
      d->filetype_ = Type_Mp4;
      break;
    case LIBMTP_FILETYPE_FLAC:
      d->filetype_ = Type_OggFlac;
      break;
    case LIBMTP_FILETYPE_MP2:
      d->filetype_ = Type_Mpeg;
      break;
    case LIBMTP_FILETYPE_M4A:
      d->filetype_ = Type_Mp4;
      break;
    default:
      d->filetype_ = Type_Unknown;
      break;
  }
}

void Song::ToMTP(LIBMTP_track_t* track) const {
  track->item_id = 0;
  track->parent_id = 0;
  track->storage_id = 0;

  track->title = strdup(d->title_.toUtf8().constData());
  track->artist = strdup(d->artist_.toUtf8().constData());
  track->album = strdup(d->album_.toUtf8().constData());
  track->composer = strdup(d->composer_.toUtf8().constData());
  track->genre = strdup(d->genre_.toUtf8().constData());
  track->title = strdup(d->title_.toUtf8().constData());
  track->date = nullptr;

  track->filename = strdup(d->basefilename_.toUtf8().constData());

  track->tracknumber = d->track_;
  track->duration = length_nanosec() / kNsecPerMsec;
  track->samplerate = d->samplerate_;
  track->nochannels = 0;
  track->wavecodec = 0;
  track->bitrate = d->bitrate_;
  track->bitratetype = 0;
  track->rating = d->rating_ * 100;
  track->usecount = d->playcount_;
  track->filesize = d->filesize_;
  track->modificationdate = d->mtime_;

  switch (d->filetype_) {
    case Type_Asf:
      track->filetype = LIBMTP_FILETYPE_ASF;
      break;
    case Type_Mp4:
      track->filetype = LIBMTP_FILETYPE_MP4;
      break;
    case Type_Mpeg:
      track->filetype = LIBMTP_FILETYPE_MP3;
      break;
    case Type_Flac:
    case Type_OggFlac:
      track->filetype = LIBMTP_FILETYPE_FLAC;
      break;
    case Type_OggSpeex:
    case Type_OggVorbis:
      track->filetype = LIBMTP_FILETYPE_OGG;
      break;
    case Type_Wav:
      track->filetype = LIBMTP_FILETYPE_WAV;
      break;
    default:
      track->filetype = LIBMTP_FILETYPE_UNDEF_AUDIO;
      break;
  }
}
#endif

void Song::MergeFromSimpleMetaBundle(const Engine::SimpleMetaBundle& bundle) {
  if (d->init_from_file_ || d->url_.scheme() == "file") {
    // This Song was already loaded using taglib. Our tags are probably better
    // than the engine's.  Note: init_from_file_ is used for non-file:// URLs
    // when the metadata is known to be good, like from Jamendo.
    return;
  }

  d->valid_ = true;
  if (!bundle.title.isEmpty()) d->title_ = bundle.title;
  if (!bundle.artist.isEmpty()) d->artist_ = bundle.artist;
  if (!bundle.album.isEmpty()) d->album_ = bundle.album;
  if (!bundle.comment.isEmpty()) d->comment_ = bundle.comment;
  if (!bundle.genre.isEmpty()) d->genre_ = bundle.genre;
  if (!bundle.bitrate.isEmpty()) d->bitrate_ = bundle.bitrate.toInt();
  if (!bundle.samplerate.isEmpty()) d->samplerate_ = bundle.samplerate.toInt();
  if (!bundle.length.isEmpty()) set_length_nanosec(bundle.length.toLongLong());
  if (!bundle.year.isEmpty()) d->year_ = bundle.year.toInt();
  if (!bundle.tracknr.isEmpty()) d->track_ = bundle.tracknr.toInt();
}

void Song::BindToQuery(QSqlQuery* query) const {
#define strval(x) (x.isNull() ? "" : x)
#define intval(x) (x <= 0 ? -1 : x)
#define notnullintval(x) (x == -1 ? QVariant() : x)

  // Remember to bind these in the same order as kBindSpec

  query->bindValue(":title", strval(d->title_));
  query->bindValue(":album", strval(d->album_));
  query->bindValue(":artist", strval(d->artist_));
  query->bindValue(":albumartist", strval(d->albumartist_));
  query->bindValue(":composer", strval(d->composer_));
  query->bindValue(":track", intval(d->track_));
  query->bindValue(":disc", intval(d->disc_));
  query->bindValue(":bpm", intval(d->bpm_));
  query->bindValue(":year", intval(d->year_));
  query->bindValue(":genre", strval(d->genre_));
  query->bindValue(":comment", strval(d->comment_));
  query->bindValue(":compilation", d->compilation_ ? 1 : 0);

  query->bindValue(":bitrate", intval(d->bitrate_));
  query->bindValue(":samplerate", intval(d->samplerate_));

  query->bindValue(":directory", notnullintval(d->directory_id_));

  if (Application::kIsPortable &&
      Utilities::UrlOnSameDriveAsClementine(d->url_)) {
    query->bindValue(
        ":filename",
        Utilities::GetRelativePathToClementineBin(d->url_).toEncoded());
  } else {
    query->bindValue(":filename", d->url_.toEncoded());
  }

  query->bindValue(":mtime", notnullintval(d->mtime_));
  query->bindValue(":ctime", notnullintval(d->ctime_));
  query->bindValue(":filesize", notnullintval(d->filesize_));

  query->bindValue(":sampler", d->sampler_ ? 1 : 0);
  query->bindValue(":art_automatic", d->art_automatic_);
  query->bindValue(":art_manual", d->art_manual_);

  query->bindValue(":filetype", d->filetype_);
  query->bindValue(":playcount", d->playcount_);
  query->bindValue(":lastplayed", intval(d->lastplayed_));
  query->bindValue(":rating", intval(d->rating_));

  query->bindValue(":forced_compilation_on", d->forced_compilation_on_ ? 1 : 0);
  query->bindValue(":forced_compilation_off",
                   d->forced_compilation_off_ ? 1 : 0);

  query->bindValue(":effective_compilation", is_compilation() ? 1 : 0);

  query->bindValue(":skipcount", d->skipcount_);
  query->bindValue(":score", d->score_);

  query->bindValue(":beginning", d->beginning_);
  query->bindValue(":length", intval(length_nanosec()));

  query->bindValue(":cue_path", d->cue_path_);
  query->bindValue(":unavailable", d->unavailable_ ? 1 : 0);
  query->bindValue(":effective_albumartist", this->effective_albumartist());

  query->bindValue(":etag", strval(d->etag_));

  query->bindValue(":performer", strval(d->performer_));
  query->bindValue(":grouping", strval(d->grouping_));
  query->bindValue(":lyrics", strval(d->lyrics_));
  query->bindValue(":originalyear", intval(d->originalyear_));
  query->bindValue(":effective_originalyear",
                   intval(this->effective_originalyear()));

#undef intval
#undef notnullintval
#undef strval
}

void Song::BindToFtsQuery(QSqlQuery* query) const {
  query->bindValue(":ftstitle", d->title_);
  query->bindValue(":ftsalbum", d->album_);
  query->bindValue(":ftsartist", d->artist_);
  query->bindValue(":ftsalbumartist", d->albumartist_);
  query->bindValue(":ftscomposer", d->composer_);
  query->bindValue(":ftsperformer", d->performer_);
  query->bindValue(":ftsgrouping", d->grouping_);
  query->bindValue(":ftsgenre", d->genre_);
  query->bindValue(":ftscomment", d->comment_);
  query->bindValue(":ftsyear", d->year_);
}

#ifdef HAVE_LIBLASTFM
void Song::ToLastFM(lastfm::Track* track, bool prefer_album_artist) const {
  lastfm::MutableTrack mtrack(*track);

  if (prefer_album_artist && !d->albumartist_.isEmpty()) {
    mtrack.setArtist(d->albumartist_);
  } else {
    mtrack.setArtist(d->artist_);
  }
#if LASTFM_MAJOR_VERSION >= 1
  mtrack.setAlbumArtist(d->albumartist_);
#endif
  mtrack.setAlbum(d->album_);
  mtrack.setTitle(d->title_);
  mtrack.setDuration(length_nanosec() / kNsecPerSec);
  mtrack.setTrackNumber(d->track_);

  if (d->filetype_ == Type_Stream && d->end_ == -1) {
    mtrack.setSource(lastfm::Track::NonPersonalisedBroadcast);
  } else {
    mtrack.setSource(lastfm::Track::Player);
  }
}
#endif  // HAVE_LIBLASTFM

QString Song::PrettyRating() const {
  float rating = d->rating_;

  if (rating == -1.0f) return "0";

  return QString::number(static_cast<int>(rating * 100));
}

QString Song::PrettyTitle() const {
  QString title(d->title_);

  if (title.isEmpty()) title = d->basefilename_;
  if (title.isEmpty()) title = d->url_.toString();

  return title;
}

QString Song::PrettyTitleWithArtist() const {
  QString title(PrettyTitle());

  if (!d->artist_.isEmpty()) title = d->artist_ + " - " + title;

  return title;
}

QString Song::PrettyLength() const {
  if (length_nanosec() == -1) return QString::null;

  return Utilities::PrettyTimeNanosec(length_nanosec());
}

QString Song::PrettyYear() const {
  if (d->year_ == -1) return QString::null;

  return QString::number(d->year_);
}

QString Song::TitleWithCompilationArtist() const {
  QString title(d->title_);

  if (title.isEmpty()) title = d->basefilename_;

  if (is_compilation() && !d->artist_.isEmpty() &&
      !d->artist_.toLower().contains("various"))
    title = d->artist_ + " - " + title;

  return title;
}

bool Song::IsMetadataEqual(const Song& other) const {
  return d->title_ == other.d->title_ && d->album_ == other.d->album_ &&
         d->artist_ == other.d->artist_ &&
         d->albumartist_ == other.d->albumartist_ &&
         d->composer_ == other.d->composer_ &&
         d->performer_ == other.d->performer_ &&
         d->grouping_ == other.d->grouping_ && d->track_ == other.d->track_ &&
         d->disc_ == other.d->disc_ && qFuzzyCompare(d->bpm_, other.d->bpm_) &&
         d->year_ == other.d->year_ &&
         d->originalyear_ == other.d->originalyear_ &&
         d->genre_ == other.d->genre_ && d->comment_ == other.d->comment_ &&
         d->compilation_ == other.d->compilation_ &&
         d->beginning_ == other.d->beginning_ &&
         length_nanosec() == other.length_nanosec() &&
         d->bitrate_ == other.d->bitrate_ &&
         d->samplerate_ == other.d->samplerate_ &&
         d->art_automatic_ == other.d->art_automatic_ &&
         d->art_manual_ == other.d->art_manual_ &&
         d->rating_ == other.d->rating_ && d->cue_path_ == other.d->cue_path_ &&
         d->lyrics_ == other.d->lyrics_;
}

bool Song::IsEditable() const {
  return d->valid_ && !d->url_.isEmpty() && !is_stream() &&
         d->filetype_ != Type_Unknown && !has_cue();
}

bool Song::operator==(const Song& other) const {
  // TODO(Paweł Bara): this isn't working for radios
  return url() == other.url() &&
         beginning_nanosec() == other.beginning_nanosec();
}

uint qHash(const Song& song) {
  // Should compare the same fields as operator==
  return qHash(song.url().toString()) ^ qHash(song.beginning_nanosec());
}

bool Song::IsSimilar(const Song& other) const {
  return title().compare(other.title(), Qt::CaseInsensitive) == 0 &&
         artist().compare(other.artist(), Qt::CaseInsensitive) == 0;
}

uint HashSimilar(const Song& song) {
  // Should compare the same fields as function IsSimilar
  return qHash(song.title().toLower()) ^ qHash(song.artist().toLower());
}

bool Song::IsOnSameAlbum(const Song& other) const {
  if (is_compilation() != other.is_compilation()) return false;

  if (has_cue() && other.has_cue() && cue_path() == other.cue_path())
    return true;

  if (is_compilation() && album() == other.album()) return true;

  return effective_album() == other.effective_album() &&
         effective_albumartist() == other.effective_albumartist();
}

QString Song::AlbumKey() const {
  return QString("%1|%2|%3")
      .arg(is_compilation() ? "_compilation" : effective_albumartist(),
           has_cue() ? cue_path() : "", effective_album());
}

void Song::ToXesam(QVariantMap* map) const {
  using mpris::AddMetadata;
  using mpris::AddMetadataAsList;
  using mpris::AsMPRISDateTimeType;

  AddMetadata("xesam:url", url().toString(), map);
  AddMetadata("xesam:title", PrettyTitle(), map);
  AddMetadataAsList("xesam:artist", artist(), map);
  AddMetadata("xesam:album", album(), map);
  AddMetadataAsList("xesam:albumArtist", albumartist(), map);
  AddMetadata("mpris:length", length_nanosec() / kNsecPerUsec, map);
  AddMetadata("xesam:trackNumber", track(), map);
  AddMetadataAsList("xesam:genre", genre(), map);
  AddMetadata("xesam:discNumber", disc(), map);
  AddMetadataAsList("xesam:comment", comment(), map);
  AddMetadata("xesam:contentCreated", AsMPRISDateTimeType(ctime()), map);
  AddMetadata("xesam:lastUsed", AsMPRISDateTimeType(lastplayed()), map);
  AddMetadata("xesam:audioBPM", static_cast<int>(bpm()), map);
  AddMetadataAsList("xesam:composer", composer(), map);
  AddMetadata("xesam:useCount", playcount(), map);
  AddMetadata("xesam:autoRating", score(), map);
  if (rating() != -1.0) {
    AddMetadata("xesam:userRating", rating(), map);
  }
}

void Song::MergeUserSetData(const Song& other) {
  set_playcount(other.playcount());
  set_skipcount(other.skipcount());
  set_lastplayed(other.lastplayed());
  set_score(other.score());
  set_art_manual(other.art_manual());
  if (rating() == -1.0f) {
    set_rating(other.rating());
  }
}
