/* This file is part of Clementine.
   Copyright 2013, David Sansome <me@davidsansome.com>

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

#include "tagreader.h"

#include <memory>

#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QTextCodec>
#include <QUrl>
#include <QVector>

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
#ifdef TAGLIB_HAS_OPUS
#include <opusfile.h>
#endif
#include <oggflacfile.h>
#include <popularimeterframe.h>
#include <speexfile.h>
#include <tag.h>
#include <textidentificationframe.h>
#include <trueaudiofile.h>
#include <tstring.h>
#include <unsynchronizedlyricsframe.h>
#include <vorbisfile.h>
#include <wavfile.h>

#include <sys/stat.h>

#include "fmpsparser.h"
#include "core/logging.h"
#include "core/messagehandler.h"
#include "core/timeconstants.h"

// Taglib added support for FLAC pictures in 1.7.0
#if (TAGLIB_MAJOR_VERSION > 1) || \
    (TAGLIB_MAJOR_VERSION == 1 && TAGLIB_MINOR_VERSION >= 7)
#define TAGLIB_HAS_FLAC_PICTURELIST
#endif

#ifdef HAVE_GOOGLE_DRIVE
#include "cloudstream.h"
#endif

#define NumberToASFAttribute(x) \
  TagLib::ASF::Attribute(QStringToTaglibString(QString::number(x)))

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

const char* TagReader::kMP4_FMPS_Rating_ID =
    "----:com.apple.iTunes:FMPS_Rating";
const char* TagReader::kMP4_FMPS_Playcount_ID =
    "----:com.apple.iTunes:FMPS_Playcount";
const char* TagReader::kMP4_FMPS_Score_ID =
    "----:com.apple.iTunes:FMPS_Rating_Amarok_Score";

namespace {
// Tags containing the year the album was originally released (in contrast to
// other tags that contain the release year of the current edition)
const char* kMP4_OriginalYear_ID = "----:com.apple.iTunes:ORIGINAL YEAR";
const char* kASF_OriginalDate_ID = "WM/OriginalReleaseTime";
const char* kASF_OriginalYear_ID = "WM/OriginalReleaseYear";
}

TagReader::TagReader()
    : factory_(new TagLibFileRefFactory),
      network_(new QNetworkAccessManager),
      kEmbeddedCover("(embedded)") {}

void TagReader::ReadFile(const QString& filename,
                         pb::tagreader::SongMetadata* song) const {
  const QByteArray url(QUrl::fromLocalFile(filename).toEncoded());
  const QFileInfo info(filename);

  qLog(Debug) << "Reading tags from" << filename;

  song->set_basefilename(DataCommaSizeFromQString(info.fileName()));
  song->set_url(url.constData(), url.size());
  song->set_filesize(info.size());
  song->set_mtime(info.lastModified().toTime_t());
  song->set_ctime(info.created().toTime_t());

  std::unique_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));
  if (fileref->isNull()) {
    qLog(Info) << "TagLib hasn't been able to read " << filename << " file";
    return;
  }

  TagLib::Tag* tag = fileref->tag();
  if (tag) {
    Decode(tag->title(), nullptr, song->mutable_title());
    Decode(tag->artist(), nullptr, song->mutable_artist());  // TPE1
    Decode(tag->album(), nullptr, song->mutable_album());
    Decode(tag->genre(), nullptr, song->mutable_genre());
    song->set_year(tag->year());
    song->set_track(tag->track());
    song->set_valid(true);
  }

  QString disc;
  QString compilation;
  QString lyrics;

  // Handle all the files which have VorbisComments (Ogg, OPUS, ...) in the same
  // way;
  // apart, so we keep specific behavior for some formats by adding another
  // "else if" block below.
  if (TagLib::Ogg::XiphComment* tag =
          dynamic_cast<TagLib::Ogg::XiphComment*>(fileref->file()->tag())) {
    ParseOggTag(tag->fieldListMap(), nullptr, &disc, &compilation, song);
#if TAGLIB_MAJOR_VERSION >= 1 && TAGLIB_MINOR_VERSION >= 11
    if (!tag->pictureList().isEmpty()) song->set_art_automatic(kEmbeddedCover);
#endif
  }

  if (TagLib::MPEG::File* file =
          dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    if (file->ID3v2Tag()) {
      const TagLib::ID3v2::FrameListMap& map = file->ID3v2Tag()->frameListMap();

      if (!map["TPOS"].isEmpty())
        disc = TStringToQString(map["TPOS"].front()->toString()).trimmed();

      if (!map["TBPM"].isEmpty())
        song->set_bpm(TStringToQString(map["TBPM"].front()->toString())
                          .trimmed()
                          .toFloat());

      if (!map["TCOM"].isEmpty())
        Decode(map["TCOM"].front()->toString(), nullptr,
               song->mutable_composer());

      if (!map["TIT1"].isEmpty())  // content group
        Decode(map["TIT1"].front()->toString(), nullptr,
               song->mutable_grouping());

      if (!map["TOPE"].isEmpty())  // original artist/performer
        Decode(map["TOPE"].front()->toString(), nullptr,
               song->mutable_performer());

      // Skip TPE1 (which is the artist) here because we already fetched it

      if (!map["TPE2"].isEmpty())  // non-standard: Apple, Microsoft
        Decode(map["TPE2"].front()->toString(), nullptr,
               song->mutable_albumartist());

      if (!map["TCMP"].isEmpty())
        compilation =
            TStringToQString(map["TCMP"].front()->toString()).trimmed();

      if (!map["TDOR"].isEmpty()) {
        song->set_originalyear(
            map["TDOR"].front()->toString().substr(0, 4).toInt());
      } else if (!map["TORY"].isEmpty()) {
        song->set_originalyear(
            map["TORY"].front()->toString().substr(0, 4).toInt());
      }

      if (!map["USLT"].isEmpty()) {
        Decode(map["USLT"].front()->toString(), nullptr,
               song->mutable_lyrics());
      } else if (!map["SYLT"].isEmpty()) {
        Decode(map["SYLT"].front()->toString(), nullptr,
               song->mutable_lyrics());
      }

      if (!map["APIC"].isEmpty()) song->set_art_automatic(kEmbeddedCover);

      // Find a suitable comment tag.  For now we ignore iTunNORM comments.
      for (int i = 0; i < map["COMM"].size(); ++i) {
        const TagLib::ID3v2::CommentsFrame* frame =
            dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(map["COMM"][i]);

        if (frame && TStringToQString(frame->description()) != "iTunNORM") {
          Decode(frame->text(), nullptr, song->mutable_comment());
          break;
        }
      }

      // Parse FMPS frames
      for (int i = 0; i < map["TXXX"].size(); ++i) {
        const TagLib::ID3v2::UserTextIdentificationFrame* frame =
            dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(
                map["TXXX"][i]);

        if (frame && frame->description().startsWith("FMPS_")) {
          ParseFMPSFrame(TStringToQString(frame->description()),
                         TStringToQString(frame->fieldList()[1]), song);
        }
      }

      // Check POPM tags
      // We do this after checking FMPS frames, so FMPS have precedence, as we
      // will consider POPM tags iff song has no rating/playcount already set.
      if (!map["POPM"].isEmpty()) {
        const TagLib::ID3v2::PopularimeterFrame* frame =
            dynamic_cast<const TagLib::ID3v2::PopularimeterFrame*>(
                map["POPM"].front());
        if (frame) {
          // Take a user rating only if there's no rating already set
          if (song->rating() <= 0 && frame->rating() > 0) {
            song->set_rating(ConvertPOPMRating(frame->rating()));
          }
          if (song->playcount() <= 0 && frame->counter() > 0) {
            song->set_playcount(frame->counter());
          }
        }
      }
    }
  } else if (TagLib::FLAC::File* file =
                 dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    if (file->xiphComment()) {
      ParseOggTag(file->xiphComment()->fieldListMap(), nullptr, &disc,
                  &compilation, song);
#ifdef TAGLIB_HAS_FLAC_PICTURELIST
      if (!file->pictureList().isEmpty()) {
        song->set_art_automatic(kEmbeddedCover);
      }
#endif
    }
    Decode(tag->comment(), nullptr, song->mutable_comment());
  } else if (TagLib::MP4::File* file =
                 dynamic_cast<TagLib::MP4::File*>(fileref->file())) {
    if (file->tag()) {
      TagLib::MP4::Tag* mp4_tag = file->tag();
      const TagLib::MP4::ItemListMap& items = mp4_tag->itemListMap();

      // Find album artists
      TagLib::MP4::ItemListMap::ConstIterator it = items.find("aART");
      if (it != items.end()) {
        TagLib::StringList album_artists = it->second.toStringList();
        if (!album_artists.isEmpty()) {
          Decode(album_artists.front(), nullptr, song->mutable_albumartist());
        }
      }

      // Find album cover art
      if (items.find("covr") != items.end()) {
        song->set_art_automatic(kEmbeddedCover);
      }

      if (items.contains("disk")) {
        disc = TStringToQString(
            TagLib::String::number(items["disk"].toIntPair().first));
      }

      if (items.contains(kMP4_FMPS_Rating_ID)) {
        float rating =
            TStringToQString(items[kMP4_FMPS_Rating_ID].toStringList().toString(
                                 '\n')).toFloat();
        if (song->rating() <= 0 && rating > 0) {
          song->set_rating(rating);
        }
      }
      if (items.contains(kMP4_FMPS_Playcount_ID)) {
        int playcount =
            TStringToQString(
                items[kMP4_FMPS_Playcount_ID].toStringList().toString('\n'))
                .toFloat();
        if (song->playcount() <= 0 && playcount > 0) {
          song->set_playcount(playcount);
        }
      }
      if (items.contains(kMP4_FMPS_Playcount_ID)) {
        int score = TStringToQString(
                        items[kMP4_FMPS_Score_ID].toStringList().toString('\n'))
                        .toFloat() *
                    100;
        if (song->score() <= 0 && score > 0) {
          song->set_score(score);
        }
      }

      if (items.contains("\251wrt")) {
        Decode(items["\251wrt"].toStringList().toString(", "), nullptr,
               song->mutable_composer());
      }
      if (items.contains("\251grp")) {
        Decode(items["\251grp"].toStringList().toString(" "), nullptr,
               song->mutable_grouping());
      }

      if (items.contains(kMP4_OriginalYear_ID)) {
        song->set_originalyear(
            TStringToQString(
                items[kMP4_OriginalYear_ID].toStringList().toString('\n'))
                .left(4)
                .toInt());
      }

      Decode(mp4_tag->comment(), nullptr, song->mutable_comment());
    }
  }
#ifdef TAGLIB_WITH_ASF
  else if (TagLib::ASF::File* file =
               dynamic_cast<TagLib::ASF::File*>(fileref->file())) {
    const TagLib::ASF::AttributeListMap& attributes_map =
        file->tag()->attributeListMap();
    if (attributes_map.contains("FMPS/Rating")) {
      const TagLib::ASF::AttributeList& attributes =
          attributes_map["FMPS/Rating"];
      if (!attributes.isEmpty()) {
        float rating =
            TStringToQString(attributes.front().toString()).toFloat();
        if (song->rating() <= 0 && rating > 0) {
          song->set_rating(rating);
        }
      }
    }
    if (attributes_map.contains("FMPS/Playcount")) {
      const TagLib::ASF::AttributeList& attributes =
          attributes_map["FMPS/Playcount"];
      if (!attributes.isEmpty()) {
        int playcount = TStringToQString(attributes.front().toString()).toInt();
        if (song->playcount() <= 0 && playcount > 0) {
          song->set_playcount(playcount);
        }
      }
    }
    if (attributes_map.contains("FMPS/Rating_Amarok_Score")) {
      const TagLib::ASF::AttributeList& attributes =
          attributes_map["FMPS/Rating_Amarok_Score"];
      if (!attributes.isEmpty()) {
        int score =
            TStringToQString(attributes.front().toString()).toFloat() * 100;
        if (song->score() <= 0 && score > 0) {
          song->set_score(score);
        }
      }
    }

    if (attributes_map.contains(kASF_OriginalDate_ID)) {
      const TagLib::ASF::AttributeList& attributes =
          attributes_map[kASF_OriginalDate_ID];
      if (!attributes.isEmpty()) {
        song->set_originalyear(
            TStringToQString(attributes.front().toString()).left(4).toInt());
      }
    } else if (attributes_map.contains(kASF_OriginalYear_ID)) {
      const TagLib::ASF::AttributeList& attributes =
          attributes_map[kASF_OriginalYear_ID];
      if (!attributes.isEmpty()) {
        song->set_originalyear(
            TStringToQString(attributes.front().toString()).left(4).toInt());
      }
    }
  }
#endif
  else if (tag) {
    Decode(tag->comment(), nullptr, song->mutable_comment());
  }

  if (!disc.isEmpty()) {
    const int i = disc.indexOf('/');
    if (i != -1) {
      // disc.right( i ).toInt() is total number of discs, we don't use this at
      // the moment
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

  if (!lyrics.isEmpty()) song->set_lyrics(lyrics.toStdString());

  if (fileref->audioProperties()) {
    song->set_bitrate(fileref->audioProperties()->bitrate());
    song->set_samplerate(fileref->audioProperties()->sampleRate());
    song->set_length_nanosec(fileref->audioProperties()->length() *
                             kNsecPerSec);
  }

  // Get the filetype if we can
  song->set_type(GuessFileType(fileref.get()));

// Set integer fields to -1 if they're not valid
#define SetDefault(field)   \
  if (song->field() <= 0) { \
    song->set_##field(-1);  \
  }
  SetDefault(track);
  SetDefault(disc);
  SetDefault(bpm);
  SetDefault(year);
  SetDefault(bitrate);
  SetDefault(samplerate);
  SetDefault(lastplayed);
#undef SetDefault
}

void TagReader::Decode(const TagLib::String& tag, const QTextCodec* codec,
                       std::string* output) {
  QString tmp;

  if (codec && tag.isLatin1()) {  // Never override UTF-8.
    const std::string fixed =
        QString::fromUtf8(tag.toCString(true)).toStdString();
    tmp = codec->toUnicode(fixed.c_str()).trimmed();
  } else {
    tmp = TStringToQString(tag).trimmed();
  }

  output->assign(DataCommaSizeFromQString(tmp));
}

void TagReader::Decode(const QString& tag, const QTextCodec* codec,
                       std::string* output) {
  if (!codec) {
    output->assign(DataCommaSizeFromQString(tag));
  } else {
    const QString decoded(codec->toUnicode(tag.toUtf8()));
    output->assign(DataCommaSizeFromQString(decoded));
  }
}

void TagReader::ParseFMPSFrame(const QString& name, const QString& value,
                               pb::tagreader::SongMetadata* song) const {
  qLog(Debug) << "Parsing FMPSFrame" << name << ", " << value;
  FMPSParser parser;
  if (!parser.Parse(value) || parser.is_empty()) return;

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
    // Take a user playcount only if there's no playcount already set
    if (song->playcount() == 0 && parser.result()[0].count() >= 2) {
      var = parser.result()[0][1];
      if (var.type() == QVariant::Double) {
        song->set_playcount(var.toDouble());
      }
    }
  } else if (name == "FMPS_Rating_Amarok_Score") {
    var = parser.result()[0][0];
    if (var.type() == QVariant::Double) {
      song->set_score(var.toFloat() * 100);
    }
  }
}

void TagReader::ParseOggTag(const TagLib::Ogg::FieldListMap& map,
                            const QTextCodec* codec, QString* disc,
                            QString* compilation,
                            pb::tagreader::SongMetadata* song) const {
  if (!map["COMPOSER"].isEmpty())
    Decode(map["COMPOSER"].front(), codec, song->mutable_composer());
  if (!map["PERFORMER"].isEmpty())
    Decode(map["PERFORMER"].front(), codec, song->mutable_performer());
  if (!map["CONTENT GROUP"].isEmpty())
    Decode(map["CONTENT GROUP"].front(), codec, song->mutable_grouping());

  if (!map["ALBUMARTIST"].isEmpty()) {
    Decode(map["ALBUMARTIST"].front(), codec, song->mutable_albumartist());
  } else if (!map["ALBUM ARTIST"].isEmpty()) {
    Decode(map["ALBUM ARTIST"].front(), codec, song->mutable_albumartist());
  }

  if (!map["ORIGINALDATE"].isEmpty())
    song->set_originalyear(
        TStringToQString(map["ORIGINALDATE"].front()).left(4).toInt());
  else if (!map["ORIGINALYEAR"].isEmpty())
    song->set_originalyear(
        TStringToQString(map["ORIGINALYEAR"].front()).toInt());

  if (!map["BPM"].isEmpty())
    song->set_bpm(TStringToQString(map["BPM"].front()).trimmed().toFloat());

  if (!map["DISCNUMBER"].isEmpty())
    *disc = TStringToQString(map["DISCNUMBER"].front()).trimmed();

  if (!map["COMPILATION"].isEmpty())
    *compilation = TStringToQString(map["COMPILATION"].front()).trimmed();

  if (!map["COVERART"].isEmpty()) song->set_art_automatic(kEmbeddedCover);

  if (!map["METADATA_BLOCK_PICTURE"].isEmpty())
    song->set_art_automatic(kEmbeddedCover);

  if (!map["FMPS_RATING"].isEmpty() && song->rating() <= 0)
    song->set_rating(
        TStringToQString(map["FMPS_RATING"].front()).trimmed().toFloat());

  if (!map["FMPS_PLAYCOUNT"].isEmpty() && song->playcount() <= 0)
    song->set_playcount(
        TStringToQString(map["FMPS_PLAYCOUNT"].front()).trimmed().toFloat());

  if (!map["FMPS_RATING_AMAROK_SCORE"].isEmpty() && song->score() <= 0)
    song->set_score(TStringToQString(map["FMPS_RATING_AMAROK_SCORE"].front())
                        .trimmed()
                        .toFloat() *
                    100);

  if (!map["LYRICS"].isEmpty())
    Decode(map["LYRICS"].front(), codec, song->mutable_lyrics());
  else if (!map["UNSYNCEDLYRICS"].isEmpty())
    Decode(map["UNSYNCEDLYRICS"].front(), codec, song->mutable_lyrics());
}

void TagReader::SetVorbisComments(
    TagLib::Ogg::XiphComment* vorbis_comments,
    const pb::tagreader::SongMetadata& song) const {
  vorbis_comments->addField("COMPOSER",
                            StdStringToTaglibString(song.composer()), true);
  vorbis_comments->addField("PERFORMER",
                            StdStringToTaglibString(song.performer()), true);
  vorbis_comments->addField("CONTENT GROUP",
                            StdStringToTaglibString(song.grouping()), true);
  vorbis_comments->addField(
      "BPM", QStringToTaglibString(
                 song.bpm() <= 0 - 1 ? QString() : QString::number(song.bpm())),
      true);
  vorbis_comments->addField(
      "DISCNUMBER",
      QStringToTaglibString(
          song.disc() <= 0 - 1 ? QString() : QString::number(song.disc())),
      true);
  vorbis_comments->addField(
      "COMPILATION", StdStringToTaglibString(song.compilation() ? "1" : "0"),
      true);

  // Try to be coherent, the two forms are used but the first one is preferred

  vorbis_comments->addField("ALBUMARTIST",
                            StdStringToTaglibString(song.albumartist()), true);
  vorbis_comments->removeField("ALBUM ARTIST");

  vorbis_comments->addField("LYRICS",
                            StdStringToTaglibString(song.lyrics()), true);
  vorbis_comments->removeField("UNSYNCEDLYRICS");

}

void TagReader::SetFMPSStatisticsVorbisComments(
    TagLib::Ogg::XiphComment* vorbis_comments,
    const pb::tagreader::SongMetadata& song) const {
  vorbis_comments->addField(
      "FMPS_PLAYCOUNT",
      QStringToTaglibString(QString::number(song.playcount())));
  vorbis_comments->addField(
      "FMPS_RATING_AMAROK_SCORE",
      QStringToTaglibString(QString::number(song.score() / 100.0)));
}

void TagReader::SetFMPSRatingVorbisComments(
    TagLib::Ogg::XiphComment* vorbis_comments,
    const pb::tagreader::SongMetadata& song) const {
  vorbis_comments->addField(
      "FMPS_RATING", QStringToTaglibString(QString::number(song.rating())));
}

pb::tagreader::SongMetadata_Type TagReader::GuessFileType(
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
#ifdef TAGLIB_HAS_OPUS
  if (dynamic_cast<TagLib::Ogg::Opus::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_OGGOPUS;
#endif
  if (dynamic_cast<TagLib::RIFF::AIFF::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_AIFF;
  if (dynamic_cast<TagLib::RIFF::WAV::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_WAV;
  if (dynamic_cast<TagLib::TrueAudio::File*>(fileref->file()))
    return pb::tagreader::SongMetadata_Type_TRUEAUDIO;

  return pb::tagreader::SongMetadata_Type_UNKNOWN;
}

bool TagReader::SaveFile(const QString& filename,
                         const pb::tagreader::SongMetadata& song) const {
  if (filename.isNull()) return false;

  qLog(Debug) << "Saving tags to" << filename;

  std::unique_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));

  if (!fileref || fileref->isNull())  // The file probably doesn't exist
    return false;

  fileref->tag()->setTitle(StdStringToTaglibString(song.title()));
  fileref->tag()->setArtist(StdStringToTaglibString(song.artist()));  // TPE1
  fileref->tag()->setAlbum(StdStringToTaglibString(song.album()));
  fileref->tag()->setGenre(StdStringToTaglibString(song.genre()));
  fileref->tag()->setComment(StdStringToTaglibString(song.comment()));
  fileref->tag()->setYear(song.year());
  fileref->tag()->setTrack(song.track());

  if (TagLib::MPEG::File* file =
          dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    TagLib::ID3v2::Tag* tag = file->ID3v2Tag(true);
    SetTextFrame(
        "TPOS", song.disc() <= 0 - 1 ? QString() : QString::number(song.disc()),
        tag);
    SetTextFrame("TBPM",
                 song.bpm() <= 0 - 1 ? QString() : QString::number(song.bpm()),
                 tag);
    SetTextFrame("TCOM", song.composer(), tag);
    SetTextFrame("TIT1", song.grouping(), tag);
    SetTextFrame("TOPE", song.performer(), tag);
    SetUnsyncLyricsFrame(song.lyrics(), tag);
    // Skip TPE1 (which is the artist) here because we already set it
    SetTextFrame("TPE2", song.albumartist(), tag);
    SetTextFrame("TCMP", std::string(song.compilation() ? "1" : "0"), tag);
  } else if (TagLib::FLAC::File* file =
                 dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    TagLib::Ogg::XiphComment* tag = file->xiphComment();
    SetVorbisComments(tag, song);
  } else if (TagLib::MP4::File* file =
                 dynamic_cast<TagLib::MP4::File*>(fileref->file())) {
    TagLib::MP4::Tag* tag = file->tag();
    tag->itemListMap()["disk"] =
        TagLib::MP4::Item(song.disc() <= 0 - 1 ? 0 : song.disc(), 0);
    tag->itemListMap()["tmpo"] = TagLib::StringList(
        song.bpm() <= 0 - 1 ? "0" : TagLib::String::number(song.bpm()));
    tag->itemListMap()["\251wrt"] = TagLib::StringList(song.composer().c_str());
    tag->itemListMap()["\251grp"] = TagLib::StringList(song.grouping().c_str());
    tag->itemListMap()["aART"] = TagLib::StringList(song.albumartist().c_str());
    tag->itemListMap()["cpil"] =
        TagLib::StringList(song.compilation() ? "1" : "0");
  }

  // Handle all the files which have VorbisComments (Ogg, OPUS, ...) in the same
  // way;
  // apart, so we keep specific behavior for some formats by adding another
  // "else if" block above.
  if (TagLib::Ogg::XiphComment* tag =
          dynamic_cast<TagLib::Ogg::XiphComment*>(fileref->file()->tag())) {
    SetVorbisComments(tag, song);
  }

  bool ret = fileref->save();
#ifdef Q_OS_LINUX
  if (ret) {
    // Linux: inotify doesn't seem to notice the change to the file unless we
    // change the timestamps as well. (this is what touch does)
    utimensat(0, QFile::encodeName(filename).constData(), nullptr, 0);
  }
#endif  // Q_OS_LINUX

  return ret;
}

bool TagReader::SaveSongStatisticsToFile(
    const QString& filename, const pb::tagreader::SongMetadata& song) const {
  if (filename.isNull()) return false;

  qLog(Debug) << "Saving song statistics tags to" << filename;

  std::unique_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));

  if (!fileref || fileref->isNull())  // The file probably doesn't exist
    return false;

  if (TagLib::MPEG::File* file =
          dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    TagLib::ID3v2::Tag* tag = file->ID3v2Tag(true);

    // Save as FMPS
    SetUserTextFrame("FMPS_PlayCount", QString::number(song.playcount()), tag);
    SetUserTextFrame("FMPS_Rating_Amarok_Score",
                     QString::number(song.score() / 100.0), tag);

    // Also save as POPM
    TagLib::ID3v2::PopularimeterFrame* frame = GetPOPMFrameFromTag(tag);
    frame->setCounter(song.playcount());

  } else if (TagLib::FLAC::File* file =
                 dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    TagLib::Ogg::XiphComment* vorbis_comments = file->xiphComment(true);
    SetFMPSStatisticsVorbisComments(vorbis_comments, song);
  } else if (TagLib::Ogg::XiphComment* tag =
                 dynamic_cast<TagLib::Ogg::XiphComment*>(
                     fileref->file()->tag())) {
    SetFMPSStatisticsVorbisComments(tag, song);
  }
#ifdef TAGLIB_WITH_ASF
  else if (TagLib::ASF::File* file =
               dynamic_cast<TagLib::ASF::File*>(fileref->file())) {
    TagLib::ASF::Tag* tag = file->tag();
    tag->addAttribute("FMPS/Playcount", NumberToASFAttribute(song.playcount()));
    tag->addAttribute("FMPS/Rating_Amarok_Score",
                      NumberToASFAttribute(song.score() / 100.0));
  }
#endif
  else if (TagLib::MP4::File* file =
               dynamic_cast<TagLib::MP4::File*>(fileref->file())) {
    TagLib::MP4::Tag* tag = file->tag();
    tag->itemListMap()[kMP4_FMPS_Score_ID] = TagLib::StringList(
        QStringToTaglibString(QString::number(song.score() / 100.0)));
    tag->itemListMap()[kMP4_FMPS_Playcount_ID] =
        TagLib::StringList(TagLib::String::number(song.playcount()));
  } else {
    // Nothing to save: stop now
    return true;
  }

  bool ret = fileref->save();
#ifdef Q_OS_LINUX
  if (ret) {
    // Linux: inotify doesn't seem to notice the change to the file unless we
    // change the timestamps as well. (this is what touch does)
    utimensat(0, QFile::encodeName(filename).constData(), nullptr, 0);
  }
#endif  // Q_OS_LINUX
  return ret;
}

bool TagReader::SaveSongRatingToFile(
    const QString& filename, const pb::tagreader::SongMetadata& song) const {
  if (filename.isNull()) return false;

  qLog(Debug) << "Saving song rating tags to" << filename;
  if (song.rating() < 0) {
    // The FMPS spec says unrated == "tag not present". For us, no rating
    // results in rating being -1, so don't write anything in that case.
    // Actually, we should also remove tag set in this case, but in
    // Clementine it is not possible to unset rating i.e. make a song "unrated".
    qLog(Debug) << "Unrated: do nothing";
    return true;
  }

  std::unique_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));

  if (!fileref || fileref->isNull())  // The file probably doesn't exist
    return false;

  if (TagLib::MPEG::File* file =
          dynamic_cast<TagLib::MPEG::File*>(fileref->file())) {
    TagLib::ID3v2::Tag* tag = file->ID3v2Tag(true);

    // Save as FMPS
    SetUserTextFrame("FMPS_Rating", QString::number(song.rating()), tag);

    // Also save as POPM
    TagLib::ID3v2::PopularimeterFrame* frame = GetPOPMFrameFromTag(tag);
    frame->setRating(ConvertToPOPMRating(song.rating()));

  } else if (TagLib::FLAC::File* file =
                 dynamic_cast<TagLib::FLAC::File*>(fileref->file())) {
    TagLib::Ogg::XiphComment* vorbis_comments = file->xiphComment(true);
    SetFMPSRatingVorbisComments(vorbis_comments, song);
  } else if (TagLib::Ogg::XiphComment* tag =
                 dynamic_cast<TagLib::Ogg::XiphComment*>(
                     fileref->file()->tag())) {
    SetFMPSRatingVorbisComments(tag, song);
  }
#ifdef TAGLIB_WITH_ASF
  else if (TagLib::ASF::File* file =
               dynamic_cast<TagLib::ASF::File*>(fileref->file())) {
    TagLib::ASF::Tag* tag = file->tag();
    tag->addAttribute("FMPS/Rating", NumberToASFAttribute(song.rating()));
  }
#endif
  else if (TagLib::MP4::File* file =
               dynamic_cast<TagLib::MP4::File*>(fileref->file())) {
    TagLib::MP4::Tag* tag = file->tag();
    tag->itemListMap()[kMP4_FMPS_Rating_ID] = TagLib::StringList(
        QStringToTaglibString(QString::number(song.rating())));
  } else {
    // Nothing to save: stop now
    return true;
  }

  bool ret = fileref->save();
#ifdef Q_OS_LINUX
  if (ret) {
    // Linux: inotify doesn't seem to notice the change to the file unless we
    // change the timestamps as well. (this is what touch does)
    utimensat(0, QFile::encodeName(filename).constData(), nullptr, 0);
  }
#endif  // Q_OS_LINUX
  return ret;
}

void TagReader::SetUserTextFrame(const QString& description,
                                 const QString& value,
                                 TagLib::ID3v2::Tag* tag) const {
  const QByteArray descr_utf8(description.toUtf8());
  const QByteArray value_utf8(value.toUtf8());
  qLog(Debug) << "Setting FMPSFrame:" << description << ", " << value;
  SetUserTextFrame(std::string(descr_utf8.constData(), descr_utf8.length()),
                   std::string(value_utf8.constData(), value_utf8.length()),
                   tag);
}

void TagReader::SetUserTextFrame(const std::string& description,
                                 const std::string& value,
                                 TagLib::ID3v2::Tag* tag) const {
  const TagLib::String t_description = StdStringToTaglibString(description);
  // Remove the frame if it already exists
  TagLib::ID3v2::UserTextIdentificationFrame* frame =
      TagLib::ID3v2::UserTextIdentificationFrame::find(tag, t_description);
  if (frame) {
    tag->removeFrame(frame);
  }

  // Create and add a new frame
  frame = new TagLib::ID3v2::UserTextIdentificationFrame(TagLib::String::UTF8);

  frame->setDescription(t_description);
  frame->setText(StdStringToTaglibString(value));
  tag->addFrame(frame);
}

void TagReader::SetTextFrame(const char* id, const QString& value,
                             TagLib::ID3v2::Tag* tag) const {
  const QByteArray utf8(value.toUtf8());
  SetTextFrame(id, std::string(utf8.constData(), utf8.length()), tag);
}

void TagReader::SetTextFrame(const char* id, const std::string& value,
                             TagLib::ID3v2::Tag* tag) const {
  TagLib::ByteVector id_vector(id);
  QVector<TagLib::ByteVector> frames_buffer;

  // Store and clear existing frames
  while (tag->frameListMap().contains(id_vector) &&
         tag->frameListMap()[id_vector].size() != 0) {
    frames_buffer.push_back(tag->frameListMap()[id_vector].front()->render());
    tag->removeFrame(tag->frameListMap()[id_vector].front());
  }

  // If no frames stored create empty frame
  if (frames_buffer.isEmpty()) {
    TagLib::ID3v2::TextIdentificationFrame frame(id_vector,
                                                 TagLib::String::UTF8);
    frames_buffer.push_back(frame.render());
  }

  // Update and add the frames
  for (int lyrics_index = 0; lyrics_index < frames_buffer.size();
       lyrics_index++) {
    TagLib::ID3v2::TextIdentificationFrame* frame =
        new TagLib::ID3v2::TextIdentificationFrame(
            frames_buffer.at(lyrics_index));
    if (lyrics_index == 0) {
      frame->setText(StdStringToTaglibString(value));
    }
    // add frame takes ownership and clears the memory
    tag->addFrame(frame);
  }
}

void TagReader::SetUnsyncLyricsFrame(const std::string& value,
                                     TagLib::ID3v2::Tag* tag) const {
  TagLib::ByteVector id_vector("USLT");
  QVector<TagLib::ByteVector> frames_buffer;

  // Store and clear existing frames
  while (tag->frameListMap().contains(id_vector) &&
         tag->frameListMap()[id_vector].size() != 0) {
    frames_buffer.push_back(tag->frameListMap()[id_vector].front()->render());
    tag->removeFrame(tag->frameListMap()[id_vector].front());
  }

  // If no frames stored create empty frame
  if (frames_buffer.isEmpty()) {
    TagLib::ID3v2::UnsynchronizedLyricsFrame frame(TagLib::String::UTF8);
    frame.setDescription("Clementine editor");
    frames_buffer.push_back(frame.render());
  }

  // Update and add the frames
  for (int lyrics_index = 0; lyrics_index < frames_buffer.size();
       lyrics_index++) {
    TagLib::ID3v2::UnsynchronizedLyricsFrame* frame =
        new TagLib::ID3v2::UnsynchronizedLyricsFrame(
            frames_buffer.at(lyrics_index));
    if (lyrics_index == 0) {
      frame->setText(StdStringToTaglibString(value));
    }
    // add frame takes ownership and clears the memory
    tag->addFrame(frame);
  }
}

bool TagReader::IsMediaFile(const QString& filename) const {
  qLog(Debug) << "Checking for valid file" << filename;

  std::unique_ptr<TagLib::FileRef> fileref(factory_->GetFileRef(filename));
  return !fileref->isNull() && fileref->tag();
}

QByteArray TagReader::LoadEmbeddedArt(const QString& filename) const {
  if (filename.isEmpty()) return QByteArray();

  qLog(Debug) << "Loading art from" << filename;

#ifdef Q_OS_WIN32
  TagLib::FileRef ref(filename.toStdWString().c_str());
#else
  TagLib::FileRef ref(QFile::encodeName(filename).constData());
#endif

  if (ref.isNull() || !ref.file()) return QByteArray();

  // MP3
  TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(ref.file());
  if (file && file->ID3v2Tag()) {
    TagLib::ID3v2::FrameList apic_frames =
        file->ID3v2Tag()->frameListMap()["APIC"];
    if (apic_frames.isEmpty()) return QByteArray();

    TagLib::ID3v2::AttachedPictureFrame* pic =
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(apic_frames.front());

    return QByteArray((const char*)pic->picture().data(),
                      pic->picture().size());
  }

  // Ogg vorbis/speex
  TagLib::Ogg::XiphComment* xiph_comment =
      dynamic_cast<TagLib::Ogg::XiphComment*>(ref.file()->tag());

  if (xiph_comment) {
    TagLib::Ogg::FieldListMap map = xiph_comment->fieldListMap();

#if TAGLIB_MAJOR_VERSION <= 1 && TAGLIB_MINOR_VERSION < 11
    // Other than the below mentioned non-standard COVERART,
    // METADATA_BLOCK_PICTURE
    // is the proposed tag for cover pictures.
    // (see http://wiki.xiph.org/VorbisComment#METADATA_BLOCK_PICTURE)
    if (map.contains("METADATA_BLOCK_PICTURE")) {
      TagLib::StringList pict_list = map["METADATA_BLOCK_PICTURE"];
      for (std::list<TagLib::String>::iterator it = pict_list.begin();
           it != pict_list.end(); ++it) {
        QByteArray data(QByteArray::fromBase64(it->toCString()));
        TagLib::ByteVector tdata(data.data(), data.size());
        TagLib::FLAC::Picture p(tdata);
        if (p.type() == TagLib::FLAC::Picture::FrontCover)
          return QByteArray(p.data().data(), p.data().size());
      }
      // If there was no specific front cover, just take the first picture
      QByteArray data(QByteArray::fromBase64(
          map["METADATA_BLOCK_PICTURE"].front().toCString()));
      TagLib::ByteVector tdata(data.data(), data.size());
      TagLib::FLAC::Picture p(tdata);
      return QByteArray(p.data().data(), p.data().size());
    }
#else
    TagLib::List<TagLib::FLAC::Picture*> pics = xiph_comment->pictureList();
    if (!pics.isEmpty()) {
      for (auto p : pics) {
        if (p->type() == TagLib::FLAC::Picture::FrontCover)
          return QByteArray(p->data().data(), p->data().size());
      }
      // If there was no specific front cover, just take the first picture
      std::list<TagLib::FLAC::Picture*>::iterator it = pics.begin();
      TagLib::FLAC::Picture* picture = *it;

      return QByteArray(picture->data().data(), picture->data().size());
    }
#endif

    // Ogg lacks a definitive standard for embedding cover art, but it seems
    // b64 encoding a field called COVERART is the general convention
    if (!map.contains("COVERART")) return QByteArray();

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

#ifdef HAVE_GOOGLE_DRIVE
bool TagReader::ReadCloudFile(const QUrl& download_url, const QString& title,
                              int size, const QString& mime_type,
                              const QString& authorisation_header,
                              pb::tagreader::SongMetadata* song) const {
  qLog(Debug) << "Loading tags from" << title;

  std::unique_ptr<CloudStream> stream(new CloudStream(
      download_url, title, size, authorisation_header, network_));
  stream->Precache();
  std::unique_ptr<TagLib::File> tag;
  if (mime_type == "audio/mpeg" &&
      title.endsWith(".mp3", Qt::CaseInsensitive)) {
    tag.reset(new TagLib::MPEG::File(stream.get(),
                                     TagLib::ID3v2::FrameFactory::instance(),
                                     TagLib::AudioProperties::Accurate));
  } else if (mime_type == "audio/mp4" ||
             (mime_type == "audio/mpeg" &&
              title.endsWith(".m4a", Qt::CaseInsensitive))) {
    tag.reset(new TagLib::MP4::File(stream.get(), true,
                                    TagLib::AudioProperties::Accurate));
  }
#ifdef TAGLIB_HAS_OPUS
  else if ((mime_type == "application/opus" || mime_type == "audio/opus" ||
            mime_type == "application/ogg" || mime_type == "audio/ogg") &&
           title.endsWith(".opus", Qt::CaseInsensitive)) {
    tag.reset(new TagLib::Ogg::Opus::File(stream.get(), true,
                                          TagLib::AudioProperties::Accurate));
  }
#endif
  else if (mime_type == "application/ogg" || mime_type == "audio/ogg") {
    tag.reset(new TagLib::Ogg::Vorbis::File(stream.get(), true,
                                            TagLib::AudioProperties::Accurate));
  } else if (mime_type == "application/x-flac" || mime_type == "audio/flac" ||
             mime_type == "audio/x-flac") {
    tag.reset(new TagLib::FLAC::File(stream.get(),
                                     TagLib::ID3v2::FrameFactory::instance(),
                                     true, TagLib::AudioProperties::Accurate));
  } else if (mime_type == "audio/x-ms-wma") {
    tag.reset(new TagLib::ASF::File(stream.get(), true,
                                    TagLib::AudioProperties::Accurate));
  } else {
    qLog(Debug) << "Unknown mime type for tagging:" << mime_type;
    return false;
  }

  if (stream->num_requests() > 2) {
    // Warn if pre-caching failed.
    qLog(Warning) << "Total requests for file:" << title
                  << stream->num_requests() << stream->cached_bytes();
  }

  if (tag->tag() && !tag->tag()->isEmpty()) {
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
#endif  // HAVE_GOOGLE_DRIVE

TagLib::ID3v2::PopularimeterFrame* TagReader::GetPOPMFrameFromTag(
    TagLib::ID3v2::Tag* tag) {
  TagLib::ID3v2::PopularimeterFrame* frame = nullptr;

  const TagLib::ID3v2::FrameListMap& map = tag->frameListMap();
  if (!map["POPM"].isEmpty()) {
    frame =
        dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(map["POPM"].front());
  }

  if (!frame) {
    frame = new TagLib::ID3v2::PopularimeterFrame();
    tag->addFrame(frame);
  }
  return frame;
}

float TagReader::ConvertPOPMRating(const int POPM_rating) {
  if (POPM_rating < 0x01) {
    return 0.0;
  } else if (POPM_rating < 0x40) {
    return 0.20;  // 1 star
  } else if (POPM_rating < 0x80) {
    return 0.40;  // 2 stars
  } else if (POPM_rating < 0xC0) {
    return 0.60;                    // 3 stars
  } else if (POPM_rating < 0xFC) {  // some players store 5 stars as 0xFC
    return 0.80;                    // 4 stars
  }
  return 1.0;  // 5 stars
}

int TagReader::ConvertToPOPMRating(const float rating) {
  if (rating < 0.20) {
    return 0x00;
  } else if (rating < 0.40) {
    return 0x01;
  } else if (rating < 0.60) {
    return 0x40;
  } else if (rating < 0.80) {
    return 0x80;
  } else if (rating < 1.0) {
    return 0xC0;
  }
  return 0xFF;
}
