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

#include <algorithm>

#include <sys/stat.h>

#include <taglib/aifffile.h>
#include <taglib/asffile.h>
#include <taglib/commentsframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mpcfile.h>
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/speexfile.h>
#include <taglib/tag.h>
#include <taglib/textidentificationframe.h>
#include <taglib/trueaudiofile.h>
#include <taglib/tstring.h>
#include <taglib/vorbisfile.h>
#include <taglib/wavfile.h>

#include "radio/fixlastfm.h"
#include <lastfm/Track>

#include <QFile>
#include <QFileInfo>
#include <QSqlQuery>
#include <QtConcurrentRun>
#include <QTextCodec>
#include <QTime>
#include <QVariant>

#ifdef Q_OS_WIN32
# include <mswmdm.h>
# include <QUuid>
#endif // Q_OS_WIN32

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
using boost::scoped_ptr;

#include "albumcoverloader.h"
#include "encoding.h"
#include "utilities.h"

#include "engines/enginebase.h"
#include "library/sqlrow.h"
#include "widgets/trackslider.h"

static QStringList Prepend(const QString& text, const QStringList& list) {
  QStringList ret(list);
  for (int i=0 ; i<ret.count() ; ++i)
    ret[i].prepend(text);
  return ret;
}

static QStringList Updateify(const QStringList& list) {
  QStringList ret(list);
  for (int i=0 ; i<ret.count() ; ++i)
    ret[i].prepend(ret[i] + " = :");
  return ret;
}

const QStringList Song::kColumns = QStringList()
    << "title" << "album" << "artist" << "albumartist" << "composer" << "track"
    << "disc" << "bpm" << "year" << "genre" << "comment" << "compilation"
    << "length" << "bitrate" << "samplerate" << "directory" << "filename"
    << "mtime" << "ctime" << "filesize" << "sampler" << "art_automatic"
    << "art_manual" << "filetype" << "playcount" << "lastplayed" << "rating"
    << "forced_compilation_on" << "forced_compilation_off"
    << "effective_compilation";

const QString Song::kColumnSpec = Song::kColumns.join(", ");
const QString Song::kBindSpec = Prepend(":", Song::kColumns).join(", ");
const QString Song::kUpdateSpec = Updateify(Song::kColumns).join(", ");


const QStringList Song::kFtsColumns = QStringList()
    << "ftstitle" << "ftsalbum" << "ftsartist" << "ftsalbumartist"
    << "ftscomposer" << "ftsgenre" << "ftscomment";

const QString Song::kFtsColumnSpec = Song::kFtsColumns.join(", ");
const QString Song::kFtsBindSpec = Prepend(":", Song::kFtsColumns).join(", ");
const QString Song::kFtsUpdateSpec = Updateify(Song::kFtsColumns).join(", ");

QString Song::JoinSpec(const QString& table) {
  return Prepend(table + ".", kColumns).join(", ");
}

QString Song::TextForFiletype(FileType type) {
  switch (type) {
    case Song::Type_Asf:       return QObject::tr("Windows Media audio");
    case Song::Type_Flac:      return QObject::tr("Flac");
    case Song::Type_Mp4:       return QObject::tr("MP4 AAC");
    case Song::Type_Mpc:       return QObject::tr("MPC");
    case Song::Type_Mpeg:      return QObject::tr("MP3"); // Not technically correct
    case Song::Type_OggFlac:   return QObject::tr("Ogg Flac");
    case Song::Type_OggSpeex:  return QObject::tr("Ogg Speex");
    case Song::Type_OggVorbis: return QObject::tr("Ogg Vorbis");
    case Song::Type_Aiff:      return QObject::tr("AIFF");
    case Song::Type_Wav:       return QObject::tr("Wav");
    case Song::Type_TrueAudio: return QObject::tr("TrueAudio");

    case Song::Type_Stream:    return QObject::tr("Stream");

    case Song::Type_Unknown:
    default:
      return QObject::tr("Unknown");
  }
}


static TagLib::String QStringToTaglibString(const QString& s);

TagLibFileRefFactory Song::kDefaultFactory;

QMutex Song::taglib_mutex_;


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
    filetype_(Type_Unknown),
    init_from_file_(false)
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

QString Song::Decode(const TagLib::String& tag, const QTextCodec* codec) {
  if (codec && tag.isLatin1()) {  // Never override UTF-8.
    const std::string fixed = QString::fromUtf8(tag.toCString(true)).toStdString();
    return codec->toUnicode(fixed.c_str()).trimmed();
  } else {
    return TStringToQString(tag).trimmed();
  }
}

QString Song::Decode(const QString& tag, const QTextCodec* codec) {
  if (!codec) {
    return tag;
  }

  return codec->toUnicode(tag.toUtf8());
}

void Song::InitFromFile(const QString& filename, int directory_id) {
#ifndef QT_NO_DEBUG_OUTPUT
  if (qApp->thread() == QThread::currentThread())
    qWarning() << Q_FUNC_INFO << "on GUI thread!";
#endif
  d->filename_ = filename;
  d->directory_id_ = directory_id;

  QMutexLocker l(&taglib_mutex_);
  scoped_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));

  if(fileref->isNull())
    return;

  d->init_from_file_ = true;

  QFileInfo info(filename);
  d->basefilename_ = info.fileName();
  d->filesize_ = info.size();
  d->mtime_ = info.lastModified().toTime_t();
  d->ctime_ = info.created().toTime_t();

  // This is single byte encoding, therefore can't be CJK.
  UniversalEncodingHandler detector(NS_FILTER_NON_CJK);

  TagLib::Tag* tag = fileref->tag();
  QTextCodec* codec = NULL;
  if (tag) {
    codec = detector.Guess(*fileref);
    d->title_ = Decode(tag->title(), codec);
    d->artist_ = Decode(tag->artist(), codec);
    d->album_ = Decode(tag->album(), codec);
    d->genre_ = Decode(tag->genre(), codec);
    d->year_ = tag->year();
    d->track_ = tag->track();

    d->valid_ = true;
  }

  QString disc;
  QString compilation;
  if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    if (file->ID3v2Tag()) {
      const TagLib::ID3v2::FrameListMap& map = file->ID3v2Tag()->frameListMap();

      if (!map["TPOS"].isEmpty())
        disc = TStringToQString(map["TPOS"].front()->toString()).trimmed();

      if (!map["TBPM"].isEmpty())
        d->bpm_ = TStringToQString(map["TBPM"].front()->toString()).trimmed().toFloat();

      if (!map["TCOM"].isEmpty())
        d->composer_ = Decode(map["TCOM"].front()->toString(), codec);

      if (!map["TPE2"].isEmpty()) // non-standard: Apple, Microsoft
        d->albumartist_ = Decode(map["TPE2"].front()->toString(), codec);

      if (!map["TCMP"].isEmpty())
        compilation = TStringToQString(map["TCMP"].front()->toString()).trimmed();

      if (!map["APIC"].isEmpty())
        d->art_automatic_ = AlbumCoverLoader::kEmbeddedCover;

      // Find a suitable comment tag.  For now we ignore iTunNORM comments.
      for (int i=0 ; i<map["COMM"].size() ; ++i) {
        const TagLib::ID3v2::CommentsFrame* frame =
            dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(map["COMM"][i]);

        if (TStringToQString(frame->description()) != "iTunNORM") {
          d->comment_ = Decode(frame->text(), codec);
          break;
        }
      }
    }
  } else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref->file())) {
    if (file->tag()) {
      ParseOggTag(file->tag()->fieldListMap(), codec, &disc, &compilation);
    }
    d->comment_ = Decode(tag->comment(), codec);
  } else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    if ( file->xiphComment() ) {
      ParseOggTag(file->xiphComment()->fieldListMap(), codec, &disc, &compilation);
    }
    d->comment_ = Decode(tag->comment(), codec);
  } else if (tag) {
    d->comment_ = Decode(tag->comment(), codec);
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

void Song::ParseOggTag(const TagLib::Ogg::FieldListMap& map, const QTextCodec* codec,
                       QString* disc, QString* compilation) {
  if (!map["COMPOSER"].isEmpty())
    d->composer_ = Decode(map["COMPOSER"].front(), codec);

  if (!map["ALBUMARTIST"].isEmpty())
    d->albumartist_ = Decode(map["ALBUMARTIST"].front(), codec);

  if (!map["BPM"].isEmpty() )
    d->bpm_ = TStringToQString( map["BPM"].front() ).trimmed().toFloat();

  if (!map["DISCNUMBER"].isEmpty() )
    *disc = TStringToQString( map["DISCNUMBER"].front() ).trimmed();

  if (!map["COMPILATION"].isEmpty() )
    *compilation = TStringToQString( map["COMPILATION"].front() ).trimmed();
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

void Song::InitFromQuery(const SqlRow& q, int col) {
  d->valid_ = true;

  d->init_from_file_ = true;

  #define tostr(n) (q.value(n).isNull() ? QString::null : q.value(n).toString())
  #define toint(n) (q.value(n).isNull() ? -1 : q.value(n).toInt())
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
  d->genre_ = tostr(col + 10);
  d->comment_ = tostr(col + 11);
  d->compilation_ = q.value(col + 12).toBool();

  d->length_ = toint(col + 13);
  d->bitrate_ = toint(col + 14);
  d->samplerate_ = toint(col + 15);

  d->directory_id_ = toint(col + 16);
  d->filename_ = tostr(col + 17);
  d->basefilename_ = QFileInfo(d->filename_).fileName();
  d->mtime_ = toint(col + 18);
  d->ctime_ = toint(col + 19);
  d->filesize_ = toint(col + 20);

  d->sampler_ = q.value(col + 21).toBool();

  d->art_automatic_ = q.value(col + 22).toString();
  d->art_manual_ = q.value(col + 23).toString();

  d->filetype_ = FileType(q.value(col + 24).toInt());
  // playcount = 25
  // lastplayed = 26
  // rating = 27

  d->forced_compilation_on_ = q.value(col + 28).toBool();
  d->forced_compilation_off_ = q.value(col + 29).toBool();

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

#ifdef HAVE_LIBGPOD
  void Song::InitFromItdb(const Itdb_Track* track) {
    d->valid_ = true;

    d->title_ = QString::fromUtf8(track->title);
    d->album_ = QString::fromUtf8(track->album);
    d->artist_ = QString::fromUtf8(track->artist);
    d->albumartist_ = QString::fromUtf8(track->albumartist);
    d->composer_ = QString::fromUtf8(track->composer);
    d->track_ = track->track_nr;
    d->disc_ = track->cd_nr;
    d->bpm_ = track->BPM;
    d->year_ = track->year;
    d->genre_ = QString::fromUtf8(track->genre);
    d->comment_ = QString::fromUtf8(track->comment);
    d->compilation_ = track->compilation;
    d->length_ = track->tracklen / 1000;
    d->bitrate_ = track->bitrate;
    d->samplerate_ = track->samplerate;
    d->mtime_ = track->time_modified;
    d->ctime_ = track->time_added;
    d->filesize_ = track->size;
    d->filetype_ = track->type2 ? Type_Mpeg : Type_Mp4;

    d->filename_ = QString::fromLocal8Bit(track->ipod_path);
    d->filename_.replace(':', '/');
    d->basefilename_ = QFileInfo(d->filename_).fileName();
  }

  void Song::ToItdb(Itdb_Track *track) const {
    track->title = strdup(d->title_.toUtf8().constData());
    track->album = strdup(d->album_.toUtf8().constData());
    track->artist = strdup(d->artist_.toUtf8().constData());
    track->albumartist = strdup(d->albumartist_.toUtf8().constData());
    track->composer = strdup(d->composer_.toUtf8().constData());
    track->track_nr = d->track_;
    track->cd_nr = d->disc_;
    track->BPM = d->bpm_;
    track->year = d->year_;
    track->genre = strdup(d->genre_.toUtf8().constData());
    track->comment = strdup(d->comment_.toUtf8().constData());
    track->compilation = d->compilation_;
    track->tracklen = d->length_ * 1000;
    track->bitrate = d->bitrate_;
    track->samplerate = d->samplerate_;
    track->time_modified = d->mtime_;
    track->time_added = d->ctime_;
    track->size = d->filesize_;
    track->type1 = 0;
    track->type2 = d->filetype_ == Type_Mp4 ? 0 : 1;
    track->mediatype = 1; // Audio
  }
#endif

#ifdef HAVE_LIBMTP
  void Song::InitFromMTP(const LIBMTP_track_t* track) {
    d->valid_ = true;

    d->title_ = QString::fromUtf8(track->title);
    d->artist_ = QString::fromUtf8(track->artist);
    d->album_ = QString::fromUtf8(track->album);
    d->composer_ = QString::fromUtf8(track->composer);
    d->genre_ = QString::fromUtf8(track->genre);
    d->filename_ = QString::number(track->item_id);
    d->basefilename_ = d->filename_;

    d->track_ = track->tracknumber;
    d->length_ = track->duration / 1000;
    d->samplerate_ = track->samplerate;
    d->bitrate_ = track->bitrate;
    d->filesize_ = track->filesize;
    d->mtime_ = track->modificationdate;
    d->ctime_ = track->modificationdate;

    switch (track->filetype) {
      case LIBMTP_FILETYPE_WAV:  d->filetype_ = Type_Wav;       break;
      case LIBMTP_FILETYPE_MP3:  d->filetype_ = Type_Mpeg;      break;
      case LIBMTP_FILETYPE_WMA:  d->filetype_ = Type_Asf;       break;
      case LIBMTP_FILETYPE_OGG:  d->filetype_ = Type_OggVorbis; break;
      case LIBMTP_FILETYPE_MP4:  d->filetype_ = Type_Mp4;       break;
      case LIBMTP_FILETYPE_AAC:  d->filetype_ = Type_Mp4;       break;
      case LIBMTP_FILETYPE_FLAC: d->filetype_ = Type_OggFlac;   break;
      case LIBMTP_FILETYPE_MP2:  d->filetype_ = Type_Mpeg;      break;
      case LIBMTP_FILETYPE_M4A:  d->filetype_ = Type_Mp4;       break;
      default:                   d->filetype_ = Type_Unknown;   break;
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
    track->date = NULL;

    track->filename = strdup(d->basefilename_.toUtf8().constData());

    track->tracknumber = d->track_;
    track->duration = d->length_ * 1000;
    track->samplerate = d->samplerate_;
    track->nochannels = 0;
    track->wavecodec = 0;
    track->bitrate = d->bitrate_;
    track->bitratetype = 0;
    track->rating = 0;
    track->usecount = 0;
    track->filesize = d->filesize_;
    track->modificationdate = d->mtime_;

    switch (d->filetype_) {
      case Type_Asf:       track->filetype = LIBMTP_FILETYPE_ASF;         break;
      case Type_Mp4:       track->filetype = LIBMTP_FILETYPE_MP4;         break;
      case Type_Mpeg:      track->filetype = LIBMTP_FILETYPE_MP3;         break;
      case Type_Flac:
      case Type_OggFlac:   track->filetype = LIBMTP_FILETYPE_FLAC;        break;
      case Type_OggSpeex:
      case Type_OggVorbis: track->filetype = LIBMTP_FILETYPE_OGG;         break;
      case Type_Wav:       track->filetype = LIBMTP_FILETYPE_WAV;         break;
      default:             track->filetype = LIBMTP_FILETYPE_UNDEF_AUDIO; break;
    }
  }
#endif

#ifdef Q_OS_WIN32
  static void AddWmdmItem(IWMDMMetaData* metadata, const wchar_t* name,
                          const QVariant& value) {
    switch (value.type()) {
      case QVariant::Int:
      case QVariant::UInt: {
        DWORD data = value.toUInt();
        metadata->AddItem(WMDM_TYPE_DWORD, name, (BYTE*)&data, sizeof(data));
        break;
      }
      case QVariant::String: {
        ScopedWCharArray data(value.toString());
        metadata->AddItem(WMDM_TYPE_STRING, name, (BYTE*)data.get(), data.bytes());
        break;
      }
      case QVariant::ByteArray: {
        QByteArray data = value.toByteArray();
        metadata->AddItem(WMDM_TYPE_BINARY, name, (BYTE*)data.constData(), data.size());
        break;
      }
      case QVariant::Bool: {
        int data = value.toBool();
        metadata->AddItem(WMDM_TYPE_BOOL, name, (BYTE*)&data, sizeof(data));
        break;
      }
      case QVariant::LongLong:
      case QVariant::ULongLong: {
        quint64 data = value.toULongLong();
        metadata->AddItem(WMDM_TYPE_QWORD, name, (BYTE*)&data, sizeof(data));
        break;
      }
      default:
        qWarning() << "Type" << value.type() << "not handled";
        Q_ASSERT(0);
        break;
    }
  }

  static QVariant ReadWmdmValue(int type, uchar* data, uint length) {
    switch (type) {
    case WMDM_TYPE_DWORD:
      return QVariant::fromValue(uint(*reinterpret_cast<DWORD*>(data)));
    case WMDM_TYPE_WORD:
      return QVariant::fromValue(uint(*reinterpret_cast<WORD*>(data)));
    case WMDM_TYPE_QWORD:
      return QVariant::fromValue(qulonglong(*reinterpret_cast<quint64*>(data)));
    case WMDM_TYPE_STRING:
      return QString::fromWCharArray(reinterpret_cast<wchar_t*>(data), length/2);
    case WMDM_TYPE_BINARY:
      return QByteArray(reinterpret_cast<char*>(data), length);
    case WMDM_TYPE_BOOL:
      return bool(*reinterpret_cast<int*>(data));
    case WMDM_TYPE_GUID:
      return QUuid(*reinterpret_cast<GUID*>(data)).toString();
    }
    return QVariant();
  }

  void Song::InitFromWmdm(IWMDMMetaData* metadata) {
    bool non_consumable = false;
    int format = 0;

    // How much metadata is there?
    uint count = 0;
    metadata->GetItemCount(&count);

    for (int i=0 ; i<count ; ++i) {
      // Get this metadata item
      wchar_t* name = NULL;
      WMDM_TAG_DATATYPE type;
      BYTE* value = NULL;
      uint length = 0;

      metadata->QueryByIndex(i, &name, &type, &value, &length);

      QVariant item_value = ReadWmdmValue(type, value, length);

      // Store it in the song if it's something we recognise
      if (wcscmp(name, g_wszWMDMTitle) == 0)
        d->title_ = item_value.toString();

      else if (wcscmp(name, g_wszWMDMAuthor) == 0)
        d->artist_ = item_value.toString();

      else if (wcscmp(name, g_wszWMDMDescription) == 0)
        d->comment_ = item_value.toString();

      else if (wcscmp(name, g_wszWMDMAlbumTitle) == 0)
        d->album_ = item_value.toString();

      else if (wcscmp(name, g_wszWMDMTrack) == 0)
        d->track_ = item_value.toInt();

      else if (wcscmp(name, g_wszWMDMGenre) == 0)
        d->genre_ = item_value.toString();

      else if (wcscmp(name, g_wszWMDMYear) == 0)
        d->year_ = item_value.toInt();

      else if (wcscmp(name, g_wszWMDMComposer) == 0)
        d->composer_ = item_value.toString();

      else if (wcscmp(name, g_wszWMDMBitrate) == 0)
        d->bitrate_ = item_value.toInt();

      else if (wcscmp(name, g_wszWMDMFileName) == 0)
        d->filename_ = item_value.toString();

      else if (wcscmp(name, g_wszWMDMDuration) == 0)
        d->length_ = item_value.toULongLong() / 10000000ll;

      else if (wcscmp(name, L"WMDM/FileSize") == 0)
        d->filesize_ = item_value.toULongLong();

      else if (wcscmp(name, L"WMDM/NonConsumable") == 0)
        non_consumable = item_value.toBool();

      else if (wcscmp(name, L"WMDM/FormatCode") == 0)
        format = item_value.toInt();

      CoTaskMemFree(name);
      CoTaskMemFree(value);
    }

    // Decide if this is music or not
    if (count == 0 || non_consumable)
      return;

    switch (format) {
    case WMDM_FORMATCODE_AIFF:
      d->filetype_ = Song::Type_Aiff;
      break;

    case WMDM_FORMATCODE_WAVE:
      d->filetype_ = Song::Type_Wav;
      break;

    case WMDM_FORMATCODE_MP2:
    case WMDM_FORMATCODE_MP3:
    case WMDM_FORMATCODE_MPEG:
      d->filetype_ = Song::Type_Mpeg;
      break;

    case WMDM_FORMATCODE_WMA:
    case WMDM_FORMATCODE_ASF:
      d->filetype_ = Song::Type_Asf;
      break;

    case WMDM_FORMATCODE_OGG:
      d->filetype_ = Song::Type_OggVorbis;
      break;

    case WMDM_FORMATCODE_AAC:
    case WMDM_FORMATCODE_MP4:
      d->filetype_ = Song::Type_Mp4;
      break;

    case WMDM_FORMATCODE_FLAC:
      d->filetype_ = Song::Type_Flac;
      break;

    case WMDM_FORMATCODE_AUDIBLE:
    case WMDM_FORMATCODE_UNDEFINEDAUDIO:
      d->filetype_ = Song::Type_Unknown;
      break;

    case WMDM_FORMATCODE_UNDEFINED:
      // WMDM doesn't know what type of file it is, so we start guessing - first
      // check if any of the music metadata fields were defined.  If they were,
      // there's a fairly good chance the file was music.
      if (!d->title_.isEmpty() || !d->artist_.isEmpty() ||
          !d->album_.isEmpty() || !d->comment_.isEmpty() ||
          !d->genre_.isEmpty() || d->track_ != -1 || d->year_ != -1 ||
          d->length_ != -1) {
        d->filetype_ = Song::Type_Unknown;
        break;
      }

      // Make a final guess based on the file extension
      {
        QString ext = d->filename_.section('.', -1, -1).toLower();
        if (ext == "mp3" || ext == "wma" || ext == "flac" || ext == "ogg" ||
            ext == "spx" || ext == "mp4" || ext == "aac" || ext == "m4a")
          break;
      }
      return;

    default:
      return; // It's not music
    }

    d->valid_ = true;
    d->mtime_ = 0;
    d->ctime_ = 0;
  }

  void Song::ToWmdm(IWMDMMetaData* metadata) const {
    AddWmdmItem(metadata, g_wszWMDMTitle, d->title_);
    AddWmdmItem(metadata, g_wszWMDMAuthor, d->artist_);
    AddWmdmItem(metadata, g_wszWMDMDescription, d->comment_);
    AddWmdmItem(metadata, g_wszWMDMAlbumTitle, d->album_);
    AddWmdmItem(metadata, g_wszWMDMTrack, d->track_);
    AddWmdmItem(metadata, g_wszWMDMGenre, d->genre_);
    AddWmdmItem(metadata, g_wszWMDMYear, QString::number(d->year_));
    AddWmdmItem(metadata, g_wszWMDMComposer, d->composer_);
    AddWmdmItem(metadata, g_wszWMDMBitrate, d->bitrate_);
    AddWmdmItem(metadata, g_wszWMDMFileName, d->basefilename_);
    AddWmdmItem(metadata, g_wszWMDMDuration, qint64(d->length_) * 10000000ll);
    AddWmdmItem(metadata, L"WMDM/FileSize", d->filesize_);

    WMDM_FORMATCODE format;
    switch (d->filetype_) {
      case Type_Aiff:      format = WMDM_FORMATCODE_AIFF; break;
      case Type_Wav:       format = WMDM_FORMATCODE_WAVE; break;
      case Type_Mpeg:      format = WMDM_FORMATCODE_MP3;  break;
      case Type_Asf:       format = WMDM_FORMATCODE_ASF;  break;
      case Type_OggFlac:
      case Type_OggSpeex:
      case Type_OggVorbis: format = WMDM_FORMATCODE_OGG;  break;
      case Type_Mp4:       format = WMDM_FORMATCODE_MP4;  break;
      case Type_Flac:      format = WMDM_FORMATCODE_FLAC; break;
      default:             format = WMDM_FORMATCODE_UNDEFINEDAUDIO; break;
    }
    AddWmdmItem(metadata, L"WMDM/FormatCode", format);
  }
#endif // Q_OS_WIN32

void Song::MergeFromSimpleMetaBundle(const Engine::SimpleMetaBundle &bundle) {
  d->valid_ = true;

  if (d->init_from_file_) {
    // This Song was already loaded using taglib. Our tags are probably better than the engine's.
    return;
  }

  UniversalEncodingHandler detector(NS_FILTER_NON_CJK);
  QTextCodec* codec = detector.Guess(bundle);

  if (!bundle.title.isEmpty()) d->title_ = Decode(bundle.title, codec);
  if (!bundle.artist.isEmpty()) d->artist_ = Decode(bundle.artist, codec);
  if (!bundle.album.isEmpty()) d->album_ = Decode(bundle.album, codec);
  if (!bundle.comment.isEmpty()) d->comment_ = Decode(bundle.comment, codec);
  if (!bundle.genre.isEmpty()) d->genre_ = Decode(bundle.genre, codec);
  if (!bundle.bitrate.isEmpty()) d->bitrate_ = bundle.bitrate.toInt();
  if (!bundle.samplerate.isEmpty()) d->samplerate_ = bundle.samplerate.toInt();
  if (!bundle.length.isEmpty()) d->length_ = bundle.length.toInt();
  if (!bundle.year.isEmpty()) d->year_ = bundle.year.toInt();
  if (!bundle.tracknr.isEmpty()) d->track_ = bundle.tracknr.toInt();
}

void Song::BindToQuery(QSqlQuery *query) const {
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

  query->bindValue(":length", intval(d->length_));
  query->bindValue(":bitrate", intval(d->bitrate_));
  query->bindValue(":samplerate", intval(d->samplerate_));

  query->bindValue(":directory_id", notnullintval(d->directory_id_));
  query->bindValue(":filename", d->filename_);
  query->bindValue(":mtime", notnullintval(d->mtime_));
  query->bindValue(":ctime", notnullintval(d->ctime_));
  query->bindValue(":filesize", notnullintval(d->filesize_));

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
  #undef notnullintval
}

void Song::BindToFtsQuery(QSqlQuery *query) const {
  query->bindValue(":ftstitle", d->title_);
  query->bindValue(":ftsalbum", d->album_);
  query->bindValue(":ftsartist", d->artist_);
  query->bindValue(":ftsalbumartist", d->albumartist_);
  query->bindValue(":ftscomposer", d->composer_);
  query->bindValue(":ftsgenre", d->genre_);
  query->bindValue(":ftscomment", d->comment_);
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

  return Utilities::PrettyTime(d->length_);
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

bool Song::IsEditable() const {
  return d->valid_ && !d->filename_.isNull() &&
         d->filetype_ != Type_Stream && d->filetype_ != Type_Unknown;
}

bool Song::Save() const {
  if (d->filename_.isNull())
    return false;

  QMutexLocker l(&taglib_mutex_);
  scoped_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(d->filename_));

  if (!fileref || fileref->isNull()) // The file probably doesn't exist
    return false;

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

bool Song::Save(const Song& song) {
  return song.Save();
}

QFuture<bool> Song::BackgroundSave() const {
  QFuture<bool> future = QtConcurrent::run(&Song::Save, Song(*this));
  return future;
}
