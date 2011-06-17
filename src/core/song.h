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

#ifndef SONG_H
#define SONG_H

#include <QFuture>
#include <QImage>
#include <QList>
#include <QMetaType>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QSqlQuery>
#include <QString>
#include <QUrl>

#include <xiphcomment.h>

#include "config.h"
#include "timeconstants.h"
#include "engines/engine_fwd.h"

#ifdef HAVE_LIBGPOD
#  include <gpod/itdb.h>
#endif

#ifdef HAVE_LIBMTP
#  include <libmtp.h>
#endif

#ifdef Q_OS_WIN32
   struct IWMDMMetaData;
#endif

class SqlRow;

#ifdef HAVE_LIBLASTFM
  namespace lastfm {
    class Track;
  }
#endif

namespace TagLib {
  class FileRef;
  class String;

  namespace ID3v2 {
    class Tag;
  }
}

class FileRefFactory {
 public:
  virtual ~FileRefFactory() {}
  virtual TagLib::FileRef* GetFileRef(const QString& filename) = 0;
};

class TagLibFileRefFactory : public FileRefFactory {
 public:
  virtual TagLib::FileRef* GetFileRef(const QString& filename);
};


class Song {
 public:
  Song();
  Song(const Song& other);
  Song(FileRefFactory* factory);

  static const QStringList kColumns;
  static const QString kColumnSpec;
  static const QString kBindSpec;
  static const QString kUpdateSpec;

  static const QStringList kFtsColumns;
  static const QString kFtsColumnSpec;
  static const QString kFtsBindSpec;
  static const QString kFtsUpdateSpec;

  static const QString kManuallyUnsetCover;
  static const QString kEmbeddedCover;

  static QString JoinSpec(const QString& table);

  // Don't change these values - they're stored in the database
  enum FileType {
    Type_Unknown = 0,
    Type_Asf = 1,
    Type_Flac = 2,
    Type_Mp4 = 3,
    Type_Mpc = 4,
    Type_Mpeg = 5,
    Type_OggFlac = 6,
    Type_OggSpeex = 7,
    Type_OggVorbis = 8,
    Type_Aiff = 9,
    Type_Wav = 10,
    Type_TrueAudio = 11,
    Type_Cdda = 12,

    Type_Stream = 99,
  };
  static QString TextForFiletype(FileType type);
  QString TextForFiletype() const { return TextForFiletype(filetype()); }

  // Helper function to load embedded cover art from a music file.  This is not
  // actually used by the Song class, but instead it is called by
  // AlbumCoverLoader and is here so it can lock on the taglib mutex.
  static QImage LoadEmbeddedArt(const QString& filename);
  // Checks if this Song can be properly initialized from it's media file.
  // This requires the 'filename' attribute to be set first.
  bool HasProperMediaFile() const;

  // Constructors
  void Init(const QString& title, const QString& artist, const QString& album, qint64 length_nanosec);
  void Init(const QString& title, const QString& artist, const QString& album, qint64 beginning, qint64 end);
  void InitFromFile(const QString& filename, int directory_id);
  void InitFromQuery(const SqlRow& query, bool reliable_metadata, int col = 0);
  void InitFromFilePartial(const QString& filename); // Just store the filename: incomplete but fast
#ifdef HAVE_LIBLASTFM
  void InitFromLastFM(const lastfm::Track& track);
#endif

  void MergeFromSimpleMetaBundle(const Engine::SimpleMetaBundle& bundle);

#ifdef HAVE_LIBGPOD
  void InitFromItdb(const Itdb_Track* track, const QString& prefix);
  void ToItdb(Itdb_Track* track) const;
#endif

#ifdef HAVE_LIBMTP
  void InitFromMTP(const LIBMTP_track_t* track, const QString& host);
  void ToMTP(LIBMTP_track_t* track) const;
#endif

#ifdef Q_OS_WIN32
  void InitFromWmdm(IWMDMMetaData* metadata);
  void ToWmdm(IWMDMMetaData* metadata) const;
#endif

  static QString Decode(const TagLib::String& tag, const QTextCodec* codec = NULL);
  static QString Decode(const QString& tag, const QTextCodec* codec = NULL);

  // Save
  void BindToQuery(QSqlQuery* query) const;
  void BindToFtsQuery(QSqlQuery* query) const;
#ifdef HAVE_LIBLASTFM
  void ToLastFM(lastfm::Track* track) const;
#endif

  // Simple accessors
  bool is_valid() const { return d->valid_; }
  bool is_unavailable() const { return d->unavailable_; }
  int id() const { return d->id_; }

  const QString& title() const { return d->title_; }
  const QString& album() const { return d->album_; }
  const QString& artist() const { return d->artist_; }
  const QString& albumartist() const { return d->albumartist_; }
  const QString& composer() const { return d->composer_; }
  int track() const { return d->track_; }
  int disc() const { return d->disc_; }
  float bpm() const { return d->bpm_; }
  int year() const { return d->year_; }
  const QString& genre() const { return d->genre_; }
  const QString& comment() const { return d->comment_; }
  bool is_compilation() const {
    return (d->compilation_ || d->sampler_ || d->forced_compilation_on_)
            && ! d->forced_compilation_off_;
  }
  float rating() const { return d->rating_; }
  int playcount() const { return d->playcount_; }
  int skipcount() const { return d->skipcount_; }
  int lastplayed() const { return d->lastplayed_; }
  int score() const { return d->score_; }

  const QString& cue_path() const { return d->cue_path_; }
  bool has_cue() const { return !d->cue_path_.isEmpty(); }

  qint64 beginning_nanosec() const { return d->beginning_; }
  qint64 end_nanosec() const { return d->end_; }

  qint64 length_nanosec() const { return d->end_ - d->beginning_; }

  int bitrate() const { return d->bitrate_; }
  int samplerate() const { return d->samplerate_; }

  int directory_id() const { return d->directory_id_; }
  const QUrl& url() const { return d->url_; }
  const QString& basefilename() const { return d->basefilename_; }
  uint mtime() const { return d->mtime_; }
  uint ctime() const { return d->ctime_; }
  int filesize() const { return d->filesize_; }
  FileType filetype() const { return d->filetype_; }
  bool is_stream() const { return d->filetype_ == Type_Stream; }
  bool is_cdda() const { return d->filetype_ == Type_Cdda; }

  const QString& art_automatic() const { return d->art_automatic_; }
  const QString& art_manual() const { return d->art_manual_; }

  // Returns true if this Song had it's cover manually unset by user.
  bool has_manually_unset_cover() const { return d->art_manual_ == kManuallyUnsetCover; }
  // This method represents an explicit request to unset this song's
  // cover.
  void manually_unset_cover() { d->art_manual_ = kManuallyUnsetCover; }

  // Returns true if this song (it's media file) has an embedded cover.
  bool has_embedded_cover() const { return d->art_automatic_ == kEmbeddedCover; }
  // Sets a flag saying that this song (it's media file) has an embedded
  // cover.
  void set_embedded_cover() { d->art_automatic_ = kEmbeddedCover; }

  const QImage& image() const { return d->image_; }

  // Pretty accessors
  QString PrettyTitle() const;
  QString PrettyTitleWithArtist() const;
  QString PrettyLength() const;
  QString PrettyYear() const;

  QString TitleWithCompilationArtist() const;

  // Setters
  bool IsEditable() const;
  bool Save() const;
  QFuture<bool> BackgroundSave() const;

  void set_id(int id) { d->id_ = id; }
  void set_valid(bool v) { d->valid_ = v; }
  void set_title(const QString& v) { d->title_ = v; }

  void set_album(const QString& v) { d->album_ = v; }
  void set_artist(const QString& v) { d->artist_ = v; }
  void set_albumartist(const QString& v) { d->albumartist_ = v; }
  void set_composer(const QString& v) { d->composer_ = v; }
  void set_track(int v) { d->track_ = v; }
  void set_disc(int v) { d->disc_ = v; }
  void set_bpm(float v) { d->bpm_ = v; }
  void set_year(int v) { d->year_ = v; }
  void set_genre(const QString& v) { d->genre_ = v; }
  void set_genre_id3(int id);
  void set_comment(const QString& v) { d->comment_ = v; }
  void set_compilation(bool v) { d->compilation_ = v; }
  void set_sampler(bool v) { d->sampler_ = v; }
  void set_beginning_nanosec(qint64 v) { d->beginning_ = qMax(0ll, v); }
  void set_end_nanosec(qint64 v) { d->end_ = v; }
  void set_length_nanosec(qint64 v) { d->end_ = d->beginning_ + v; }
  void set_bitrate(int v) { d->bitrate_ = v; }
  void set_samplerate(int v) { d->samplerate_ = v; }
  void set_mtime(int v) { d->mtime_ = v; }
  void set_ctime(int v) { d->ctime_ = v; }
  void set_filesize(int v) { d->filesize_ = v; }
  void set_filetype(FileType v) { d->filetype_ = v; }
  void set_art_automatic(const QString& v) { d->art_automatic_ = v; }
  void set_art_manual(const QString& v) { d->art_manual_ = v; }
  void set_image(const QImage& i) { d->image_ = i; }
  void set_forced_compilation_on(bool v) { d->forced_compilation_on_ = v; }
  void set_forced_compilation_off(bool v) { d->forced_compilation_off_ = v; }
  void set_rating(float v) { d->rating_ = v; }
  void set_playcount(int v) { d->playcount_ = v; }
  void set_skipcount(int v) { d->skipcount_ = v; }
  void set_lastplayed(int v) { d->lastplayed_ = v; }
  void set_score(int v) { d->score_ = qBound(0, v, 100); }
  void set_cue_path(const QString& v) { d->cue_path_ = v; }
  void set_unavailable(bool v) { d->unavailable_ = v; }

  // Setters that should only be used by tests
  void set_url(const QUrl& v) { d->url_ = v; }
  void set_basefilename(const QString& v) { d->basefilename_ = v; } 
  void set_directory_id(int v) { d->directory_id_ = v; }

  // Comparison functions
  bool IsMetadataEqual(const Song& other) const;
  bool IsOnSameAlbum(const Song& other) const;

  bool operator==(const Song& other) const;

 private:
  void GuessFileType(TagLib::FileRef* fileref);
  static bool Save(const Song& song);

  // Helper methods for taglib
  static void SetTextFrame(const QString& id, const QString& value,
                           TagLib::ID3v2::Tag* tag);
  void ParseFMPSFrame(const QString& name, const QString& value);

 private:
  struct Private : public QSharedData {
    Private();

    bool valid_;
    int id_;

    QString title_;
    QString album_;
    QString artist_;
    QString albumartist_;
    QString composer_;
    int track_;
    int disc_;
    float bpm_;
    int year_;
    QString genre_;
    QString comment_;
    bool compilation_;            // From the file tag
    bool sampler_;                // From the library scanner
    bool forced_compilation_on_;  // Set by the user
    bool forced_compilation_off_; // Set by the user

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
    QString art_automatic_; // Guessed by LibraryWatcher
    QString art_manual_;    // Set by the user - should take priority

    QImage image_;

    // Whether this song was loaded from a file using taglib.
    bool init_from_file_;
    // Whether our encoding guesser thinks these tags might be incorrectly encoded.
    bool suspicious_tags_;
    
    // Whether the song does not exist on the file system anymore, but is still
    // stored in the database so as to remember the user's metadata.
    bool unavailable_;
  };

  void ParseOggTag(const TagLib::Ogg::FieldListMap& map, const QTextCodec* codec, QString* disc, QString* compilation);

 private:
  QSharedDataPointer<Private> d;
  FileRefFactory* factory_;

  static TagLibFileRefFactory kDefaultFactory;

  static QMutex taglib_mutex_;
};
Q_DECLARE_METATYPE(Song);

typedef QList<Song> SongList;
Q_DECLARE_METATYPE(QList<Song>);

#endif // SONG_H
