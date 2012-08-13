/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "fmpsparser.h"
#include "tagreaderworker.h"
#include "core/encoding.h"
#include "core/logging.h"
#include "core/timeconstants.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QTextCodec>
#include <QUrl>

#include <aifffile.h>
#include <asffile.h>
#include <attachedpictureframe.h>
#include <commentsframe.h>
#include <fileref.h>
#include <flacfile.h>
#include <id3v2tag.h>
#include <mp4file.h>
#include <mp4tag.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <speexfile.h>
#include <tag.h>
#include <textidentificationframe.h>
#include <trueaudiofile.h>
#include <tstring.h>
#include <vorbisfile.h>
#include <wavfile.h>

#include <boost/scoped_ptr.hpp>
#include <sys/stat.h>

// Taglib added support for FLAC pictures in 1.7.0
#if (TAGLIB_MAJOR_VERSION > 1) || (TAGLIB_MAJOR_VERSION == 1 && TAGLIB_MINOR_VERSION >= 7)
# define TAGLIB_HAS_FLAC_PICTURELIST
#endif

#ifdef HAVE_GOOGLE_DRIVE
# include "googledrivestream.h"
#endif


using boost::scoped_ptr;


class FileRefFactory {
 public:
  virtual ~FileRefFactory() {}
  virtual TagLib::FileRef* GetFileRef(const QString& filename) = 0;
};

class TagLibFileRefFactory : public FileRefFactory {
 public:
  virtual TagLib::FileRef* GetFileRef(const QString& filename) {
    #ifdef Q_OS_WIN32
      return new TagLib::FileRef(filename.toStdWString().c_str());
    #else
      return new TagLib::FileRef(QFile::encodeName(filename).constData());
    #endif
  }
};

namespace {

TagLib::String StdStringToTaglibString(const std::string& s) {
  return TagLib::String(s.c_str(), TagLib::String::UTF8);
}

TagLib::String QStringToTaglibString(const QString& s) {
  return TagLib::String(s.toUtf8().constData(), TagLib::String::UTF8);
}

}


TagReaderWorker::TagReaderWorker(QIODevice* socket, QObject* parent)
  : AbstractMessageHandler<pb::tagreader::Message>(socket, parent),
    factory_(new TagLibFileRefFactory),
    network_(new QNetworkAccessManager),
    kEmbeddedCover("(embedded)")
{
}

void TagReaderWorker::MessageArrived(const pb::tagreader::Message& message) {
  pb::tagreader::Message reply;

#if 0
  // Crash every few requests
  if (qrand() % 10 == 0) {
    qLog(Debug) << "Crashing on request ID" << message.id();
    abort();
  }
#endif

  if (message.has_read_file_request()) {
    ReadFile(QStringFromStdString(message.read_file_request().filename()),
             reply.mutable_read_file_response()->mutable_metadata());
  } else if (message.has_save_file_request()) {
    reply.mutable_save_file_response()->set_success(
          SaveFile(QStringFromStdString(message.save_file_request().filename()),
                   message.save_file_request().metadata()));
  } else if (message.has_is_media_file_request()) {
    reply.mutable_is_media_file_response()->set_success(
          IsMediaFile(QStringFromStdString(message.is_media_file_request().filename())));
  } else if (message.has_load_embedded_art_request()) {
    QByteArray data = LoadEmbeddedArt(
          QStringFromStdString(message.load_embedded_art_request().filename()));
    reply.mutable_load_embedded_art_response()->set_data(
          data.constData(), data.size());
  } else if (message.has_read_google_drive_request()) {
#ifdef HAVE_GOOGLE_DRIVE
    const pb::tagreader::ReadGoogleDriveRequest& req =
        message.read_google_drive_request();
    if (!ReadGoogleDrive(
        QUrl::fromEncoded(QByteArray(req.download_url().data(),
                                     req.download_url().size())),
        QStringFromStdString(req.title()),
        req.size(),
        QStringFromStdString(req.mime_type()),
        QStringFromStdString(req.access_token()),
        reply.mutable_read_google_drive_response()->mutable_metadata())) {
      reply.mutable_read_google_drive_response()->clear_metadata();
    }
#endif
  }

  SendReply(message, &reply);
}

void TagReaderWorker::ReadFile(const QString& filename,
                               pb::tagreader::SongMetadata* song) const {
  const QByteArray url(QUrl::fromLocalFile(filename).toEncoded());
  const QFileInfo info(filename);

  qLog(Debug) << "Reading tags from" << filename;

  song->set_basefilename(DataCommaSizeFromQString(info.fileName()));
  song->set_url(url.constData(), url.size());
  song->set_filesize(info.size());
  song->set_mtime(info.lastModified().toTime_t());
  song->set_ctime(info.created().toTime_t());

  scoped_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));
  if(fileref->isNull()) {
    qLog(Info) << "TagLib hasn't been able to read " << filename << " file";
    return;
  }

  // This is single byte encoding, therefore can't be CJK.
  UniversalEncodingHandler detector(NS_FILTER_NON_CJK);

  TagLib::Tag* tag = fileref->tag();
  QTextCodec* codec = NULL;
  if (tag) {
    TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref->file());
    if (file && (file->ID3v2Tag() || file->ID3v1Tag())) {
      codec = detector.Guess(*fileref);
    }
    if (codec &&
        codec->name() != "UTF-8" &&
        codec->name() != "ISO-8859-1") {
      // Mark tags where we detect an unusual codec as suspicious.
      song->set_suspicious_tags(true);
    }


    Decode(tag->title(), NULL, song->mutable_title());
    Decode(tag->artist(), NULL, song->mutable_artist());
    Decode(tag->album(), NULL, song->mutable_album());
    Decode(tag->genre(), NULL, song->mutable_genre());
    song->set_year(tag->year());
    song->set_track(tag->track());
    song->set_valid(true);
  }

  QString disc;
  QString compilation;
  if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    if (file->ID3v2Tag()) {
      const TagLib::ID3v2::FrameListMap& map = file->ID3v2Tag()->frameListMap();

      if (!map["TPOS"].isEmpty())
        disc = TStringToQString(map["TPOS"].front()->toString()).trimmed();

      if (!map["TBPM"].isEmpty())
        song->set_bpm(TStringToQString(map["TBPM"].front()->toString()).trimmed().toFloat());

      if (!map["TCOM"].isEmpty())
        Decode(map["TCOM"].front()->toString(), NULL, song->mutable_composer());

      if (!map["TPE2"].isEmpty()) // non-standard: Apple, Microsoft
        Decode(map["TPE2"].front()->toString(), NULL, song->mutable_albumartist());

      if (!map["TCMP"].isEmpty())
        compilation = TStringToQString(map["TCMP"].front()->toString()).trimmed();

      if (!map["APIC"].isEmpty())
        song->set_art_automatic(kEmbeddedCover);

      // Find a suitable comment tag.  For now we ignore iTunNORM comments.
      for (int i=0 ; i<map["COMM"].size() ; ++i) {
        const TagLib::ID3v2::CommentsFrame* frame =
            dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(map["COMM"][i]);

        if (frame && TStringToQString(frame->description()) != "iTunNORM") {
          Decode(frame->text(), NULL, song->mutable_comment());
          break;
        }
      }

      // Parse FMPS frames
      for (int i=0 ; i<map["TXXX"].size() ; ++i) {
        const TagLib::ID3v2::UserTextIdentificationFrame* frame =
            dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(map["TXXX"][i]);

        if (frame && frame->description().startsWith("FMPS_")) {
          ParseFMPSFrame(TStringToQString(frame->description()),
                         TStringToQString(frame->fieldList()[1]),
                         song);
        }
      }
    }
  } else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref->file())) {
    if (file->tag()) {
      ParseOggTag(file->tag()->fieldListMap(), NULL, &disc, &compilation, song);
    }
    Decode(tag->comment(), NULL, song->mutable_comment());
  } else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    if ( file->xiphComment() ) {
      ParseOggTag(file->xiphComment()->fieldListMap(), NULL, &disc, &compilation, song);
#ifdef TAGLIB_HAS_FLAC_PICTURELIST
      if (!file->pictureList().isEmpty()) {
        song->set_art_automatic(kEmbeddedCover);
      }
#endif
    }
    Decode(tag->comment(), NULL, song->mutable_comment());
  } else if (TagLib::MP4::File* file = dynamic_cast<TagLib::MP4::File*>(fileref->file())) {
    if (file->tag()) {
      TagLib::MP4::Tag* mp4_tag = file->tag();
      const TagLib::MP4::ItemListMap& items = mp4_tag->itemListMap();

      // Find album artists
      TagLib::MP4::ItemListMap::ConstIterator it = items.find("aART");
      if (it != items.end()) {
        TagLib::StringList album_artists = it->second.toStringList();
        if (!album_artists.isEmpty()) {
          Decode(album_artists.front(), NULL, song->mutable_albumartist());
        }
      }

      // Find album cover art
      if (items.find("covr") != items.end()) {
        song->set_art_automatic(kEmbeddedCover);
      }
    }
  } else if (tag) {
    Decode(tag->comment(), NULL, song->mutable_comment());
  }

  if (!disc.isEmpty()) {
    const int i = disc.indexOf('/');
    if (i != -1) {
      // disc.right( i ).toInt() is total number of discs, we don't use this at the moment
      song->set_disc(disc.left(i).toInt());
    } else {
      song->set_disc(disc.toInt());
    }
  }

  if (compilation.isEmpty()) {
    // well, it wasn't set, but if the artist is VA assume it's a compilation
    if (QStringFromStdString(song->artist()).toLower() == "various artists") {
      song->set_compilation(true);
    }
  } else {
    song->set_compilation(compilation.toInt() == 1);
  }

  if (fileref->audioProperties()) {
    song->set_bitrate(fileref->audioProperties()->bitrate());
    song->set_samplerate(fileref->audioProperties()->sampleRate());
    song->set_length_nanosec(fileref->audioProperties()->length() * kNsecPerSec);
  }

  // Get the filetype if we can
  song->set_type(GuessFileType(fileref.get()));

  // Set integer fields to -1 if they're not valid
  #define SetDefault(field) if (song->field() <= 0) { song->set_##field(-1); }
  SetDefault(track);
  SetDefault(disc);
  SetDefault(bpm);
  SetDefault(year);
  SetDefault(bitrate);
  SetDefault(samplerate);
  SetDefault(lastplayed);
  #undef SetDefault
}

void TagReaderWorker::Decode(const TagLib::String& tag, const QTextCodec* codec,
                             std::string* output) {
  QString tmp;

  if (codec && tag.isLatin1()) {  // Never override UTF-8.
    const std::string fixed = QString::fromUtf8(tag.toCString(true)).toStdString();
    tmp = codec->toUnicode(fixed.c_str()).trimmed();
  } else {
    tmp = TStringToQString(tag).trimmed();
  }

  output->assign(DataCommaSizeFromQString(tmp));
}

void TagReaderWorker::Decode(const QString& tag, const QTextCodec* codec,
                             std::string* output) {
  if (!codec) {
    output->assign(DataCommaSizeFromQString(tag));
  } else {
    const QString decoded(codec->toUnicode(tag.toUtf8()));
    output->assign(DataCommaSizeFromQString(decoded));
  }
}

void TagReaderWorker::ParseFMPSFrame(const QString& name, const QString& value,
                                     pb::tagreader::SongMetadata* song) const {
  FMPSParser parser;
  if (!parser.Parse(value) || parser.is_empty())
    return;

  QVariant var;
  if (name == "FMPS_Rating") {
    var = parser.result()[0][0];
    if (var.type() == QVariant::Double) {
      song->set_rating(var.toDouble());
    }
  } else if (name == "FMPS_Rating_User") {
    // Take a user rating only if there's no rating already set
    if (song->rating() == -1 && parser.result()[0].count() >= 2) {
      var = parser.result()[0][1];
      if (var.type() == QVariant::Double) {
        song->set_rating(var.toDouble());
      }
    }
  } else if (name == "FMPS_PlayCount") {
    var = parser.result()[0][0];
    if (var.type() == QVariant::Double) {
      song->set_playcount(var.toDouble());
    }
  } else if (name == "FMPS_PlayCount_User") {
    // Take a user rating only if there's no playcount already set
    if (song->rating() == -1 && parser.result()[0].count() >= 2) {
      var = parser.result()[0][1];
      if (var.type() == QVariant::Double) {
        song->set_playcount(var.toDouble());
      }
    }
  }
}

void TagReaderWorker::ParseOggTag(const TagLib::Ogg::FieldListMap& map,
                                  const QTextCodec* codec,
                                  QString* disc, QString* compilation,
                                  pb::tagreader::SongMetadata* song) const {
  if (!map["COMPOSER"].isEmpty())
    Decode(map["COMPOSER"].front(), codec, song->mutable_composer());

  if (!map["ALBUMARTIST"].isEmpty()) {
    Decode(map["ALBUMARTIST"].front(), codec, song->mutable_albumartist());
  } else if (!map["ALBUM ARTIST"].isEmpty()) {
    Decode(map["ALBUM ARTIST"].front(), codec, song->mutable_albumartist());
  }

  if (!map["BPM"].isEmpty() )
    song->set_bpm(TStringToQString( map["BPM"].front() ).trimmed().toFloat());

  if (!map["DISCNUMBER"].isEmpty() )
    *disc = TStringToQString( map["DISCNUMBER"].front() ).trimmed();

  if (!map["COMPILATION"].isEmpty() )
    *compilation = TStringToQString( map["COMPILATION"].front() ).trimmed();

  if (!map["COVERART"].isEmpty())
    song->set_art_automatic(kEmbeddedCover);
}

pb::tagreader::SongMetadata_Type TagReaderWorker::GuessFileType(
    TagLib::FileRef* fileref) const {
#ifdef TAGLIB_WITH_ASF
  if (dynamic_cast<TagLib::ASF::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_ASF;
#endif
  if (dynamic_cast<TagLib::FLAC::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_FLAC;
#ifdef TAGLIB_WITH_MP4
  if (dynamic_cast<TagLib::MP4::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_MP4;
#endif
  if (dynamic_cast<TagLib::MPC::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_MPC;
  if (dynamic_cast<TagLib::MPEG::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_MPEG;
  if (dynamic_cast<TagLib::Ogg::FLAC::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_OGGFLAC;
  if (dynamic_cast<TagLib::Ogg::Speex::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_OGGSPEEX;
  if (dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_OGGVORBIS;
  if (dynamic_cast<TagLib::RIFF::AIFF::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_AIFF;
  if (dynamic_cast<TagLib::RIFF::WAV::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_WAV;
  if (dynamic_cast<TagLib::TrueAudio::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_TRUEAUDIO;

  return pb::tagreader::SongMetadata_Type_UNKNOWN;
}

bool TagReaderWorker::SaveFile(const QString& filename,
                               const pb::tagreader::SongMetadata& song) const {
  if (filename.isNull())
    return false;

  qLog(Debug) << "Saving tags to" << filename;

  scoped_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));

  if (!fileref || fileref->isNull()) // The file probably doesn't exist
    return false;

  fileref->tag()->setTitle(StdStringToTaglibString(song.title()));
  fileref->tag()->setArtist(StdStringToTaglibString(song.artist()));
  fileref->tag()->setAlbum(StdStringToTaglibString(song.album()));
  fileref->tag()->setGenre(StdStringToTaglibString(song.genre()));
  fileref->tag()->setComment(StdStringToTaglibString(song.comment()));
  fileref->tag()->setYear(song.year());
  fileref->tag()->setTrack(song.track());

  if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    TagLib::ID3v2::Tag* tag = file->ID3v2Tag(true);
    SetTextFrame("TPOS", song.disc() <= 0 -1 ? QString() : QString::number(song.disc()), tag);
    SetTextFrame("TBPM", song.bpm() <= 0 -1 ? QString() : QString::number(song.bpm()), tag);
    SetTextFrame("TCOM", song.composer(), tag);
    SetTextFrame("TPE2", song.albumartist(), tag);
    SetTextFrame("TCMP", std::string(song.compilation() ? "1" : "0"), tag);
  }
  else if (TagLib::Ogg::Vorbis::File* file = dynamic_cast<TagLib::Ogg::Vorbis::File*>(fileref->file())) {
    TagLib::Ogg::XiphComment* tag = file->tag();
    tag->addField("COMPOSER", StdStringToTaglibString(song.composer()), true);
    tag->addField("BPM", QStringToTaglibString(song.bpm() <= 0 -1 ? QString() : QString::number(song.bpm())), true);
    tag->addField("DISCNUMBER", QStringToTaglibString(song.disc() <= 0 -1 ? QString() : QString::number(song.disc())), true);
    tag->addField("COMPILATION", StdStringToTaglibString(song.compilation() ? "1" : "0"), true);
  }
  else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    TagLib::Ogg::XiphComment* tag = file->xiphComment();
    tag->addField("COMPOSER", StdStringToTaglibString(song.composer()), true);
    tag->addField("BPM", QStringToTaglibString(song.bpm() <= 0 -1 ? QString() : QString::number(song.bpm())), true);
    tag->addField("DISCNUMBER", QStringToTaglibString(song.disc() <= 0 -1 ? QString() : QString::number(song.disc())), true);
    tag->addField("COMPILATION", StdStringToTaglibString(song.compilation() ? "1" : "0"), true);
  }

  bool ret = fileref->save();
  #ifdef Q_OS_LINUX
  if (ret) {
    // Linux: inotify doesn't seem to notice the change to the file unless we
    // change the timestamps as well. (this is what touch does)
    utimensat(0, QFile::encodeName(filename).constData(), NULL, 0);
  }
  #endif  // Q_OS_LINUX

  return ret;
}

void TagReaderWorker::SetTextFrame(const char* id, const QString& value,
                                   TagLib::ID3v2::Tag* tag) const {
  const QByteArray utf8(value.toUtf8());
  SetTextFrame(id, std::string(utf8.constData(), utf8.length()), tag);
}

void TagReaderWorker::SetTextFrame(const char* id, const std::string& value,
                                   TagLib::ID3v2::Tag* tag) const {
  TagLib::ByteVector id_vector(id);

  // Remove the frame if it already exists
  while (tag->frameListMap().contains(id_vector) &&
         tag->frameListMap()[id_vector].size() != 0) {
    tag->removeFrame(tag->frameListMap()[id_vector].front());
  }

  // Create and add a new frame
  TagLib::ID3v2::TextIdentificationFrame* frame =
      new TagLib::ID3v2::TextIdentificationFrame(id_vector,
                                                 TagLib::String::UTF8);
  frame->setText(StdStringToTaglibString(value));
  tag->addFrame(frame);
}

bool TagReaderWorker::IsMediaFile(const QString& filename) const {
  qLog(Debug) << "Checking for valid file" << filename;

  scoped_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));
  return !fileref->isNull() && fileref->tag();
}

QByteArray TagReaderWorker::LoadEmbeddedArt(const QString& filename) const {
  if (filename.isEmpty())
    return QByteArray();

  qLog(Debug) << "Loading art from" << filename;

#ifdef Q_OS_WIN32
  TagLib::FileRef ref(filename.toStdWString().c_str());
#else
  TagLib::FileRef ref(QFile::encodeName(filename).constData());
#endif

  if (ref.isNull() || !ref.file())
    return QByteArray();

  // MP3
  TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(ref.file());
  if (file && file->ID3v2Tag()) {
    TagLib::ID3v2::FrameList apic_frames = file->ID3v2Tag()->frameListMap()["APIC"];
    if (apic_frames.isEmpty())
      return QByteArray();

    TagLib::ID3v2::AttachedPictureFrame* pic =
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(apic_frames.front());

    return QByteArray((const char*) pic->picture().data(), pic->picture().size());
  }

  // Ogg vorbis/speex
  TagLib::Ogg::XiphComment* xiph_comment =
      dynamic_cast<TagLib::Ogg::XiphComment*>(ref.file()->tag());

  if (xiph_comment) {
    TagLib::Ogg::FieldListMap map = xiph_comment->fieldListMap();

    // Ogg lacks a definitive standard for embedding cover art, but it seems
    // b64 encoding a field called COVERART is the general convention
    if (!map.contains("COVERART"))
      return QByteArray();

    return QByteArray::fromBase64(map["COVERART"].toString().toCString());
  }

#ifdef TAGLIB_HAS_FLAC_PICTURELIST
  // Flac
  TagLib::FLAC::File* flac_file = dynamic_cast<TagLib::FLAC::File*>(ref.file());
  if (flac_file && flac_file->xiphComment()) {
    TagLib::List<TagLib::FLAC::Picture*> pics = flac_file->pictureList();
    if (!pics.isEmpty()) {
      // Use the first picture in the file - this could be made cleverer and
      // pick the front cover if it's present.

      std::list<TagLib::FLAC::Picture*>::iterator it = pics.begin();
      TagLib::FLAC::Picture* picture = *it;

      return QByteArray(picture->data().data(), picture->data().size());
    }
  }
#endif

  // MP4/AAC
  TagLib::MP4::File* aac_file = dynamic_cast<TagLib::MP4::File*>(ref.file());
  if (aac_file) {
    TagLib::MP4::Tag* tag = aac_file->tag();
    const TagLib::MP4::ItemListMap& items = tag->itemListMap();
    TagLib::MP4::ItemListMap::ConstIterator it = items.find("covr");
    if (it != items.end()) {
      const TagLib::MP4::CoverArtList& art_list = it->second.toCoverArtList();

      if (!art_list.isEmpty()) {
        // Just take the first one for now
        const TagLib::MP4::CoverArt& art = art_list.front();
        return QByteArray(art.data().data(), art.data().size());
      }
    }
  }

  return QByteArray();
}

void TagReaderWorker::DeviceClosed() {
  AbstractMessageHandler<pb::tagreader::Message>::DeviceClosed();

  qApp->exit();
}

#ifdef HAVE_GOOGLE_DRIVE
bool TagReaderWorker::ReadGoogleDrive(const QUrl& download_url,
                                      const QString& title,
                                      int size,
                                      const QString& mime_type,
                                      const QString& access_token,
                                      pb::tagreader::SongMetadata* song) const {
  qLog(Debug) << "Loading tags from" << title;

  GoogleDriveStream* stream = new GoogleDriveStream(
      download_url, title, size, access_token, network_);
  stream->Precache();
  scoped_ptr<TagLib::File> tag;
  if (mime_type == "audio/mpeg" && title.endsWith(".mp3")) {
    tag.reset(new TagLib::MPEG::File(
        stream,  // Takes ownership.
        TagLib::ID3v2::FrameFactory::instance(),
        TagLib::AudioProperties::Accurate));
  } else if (mime_type == "audio/mpeg" && title.endsWith(".m4a")) {
    tag.reset(new TagLib::MP4::File(
        stream,
        true,
        TagLib::AudioProperties::Accurate));
  } else if (mime_type == "application/ogg") {
    tag.reset(new TagLib::Ogg::Vorbis::File(
        stream,
        true,
        TagLib::AudioProperties::Accurate));
  } else if (mime_type == "application/x-flac") {
    tag.reset(new TagLib::FLAC::File(
        stream,
        TagLib::ID3v2::FrameFactory::instance(),
        true,
        TagLib::AudioProperties::Accurate));
  } else {
    qLog(Debug) << "Unknown mime type for tagging:" << mime_type;
    return false;
  }

  if (stream->num_requests() > 2) {
    // Warn if pre-caching failed.
    qLog(Warning) << "Total requests for file:" << title
                  << stream->num_requests()
                  << stream->cached_bytes();
  }

  if (tag->tag()) {
    song->set_title(tag->tag()->title().toCString(true));
    song->set_artist(tag->tag()->artist().toCString(true));
    song->set_album(tag->tag()->album().toCString(true));
    song->set_filesize(size);

    if (tag->tag()->track() != 0) {
      song->set_track(tag->tag()->track());
    }
    if (tag->tag()->year() != 0) {
      song->set_year(tag->tag()->year());
    }

    song->set_type(pb::tagreader::SongMetadata_Type_STREAM);

    if (tag->audioProperties()) {
      song->set_length_nanosec(tag->audioProperties()->length() * kNsecPerSec);
    }
    return true;
  }

  return false;
}
#endif // HAVE_GOOGLE_DRIVE
