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

#include "config.h"
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

namespace lastfm {
  class Track;
}

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

    Type_Stream = 99,
  };
  static QString TextForFiletype(FileType type);
  QString TextForFiletype() const { return TextForFiletype(filetype()); }

  // Constructors
  void Init(const QString& title, const QString& artist, const QString& album, int length);
  void InitFromFile(const QString& filename, int directory_id);
  void InitFromQuery(const SqlRow& query, int col = 0);
  void InitFromLastFM(const lastfm::Track& track);

  void MergeFromSimpleMetaBundle(const Engine::SimpleMetaBundle& bundle);

#ifdef HAVE_LIBGPOD
  void InitFromItdb(const Itdb_Track* track);
  void ToItdb(Itdb_Track* track) const;
#endif

#ifdef HAVE_LIBMTP
  void InitFromMTP(const LIBMTP_track_t* track);
  void ToMTP(LIBMTP_track_t* track) const;
#endif

#ifdef Q_OS_WIN32
  void InitFromWmdm(IWMDMMetaData* metadata);
  void ToWmdm(IWMDMMetaData* metadata) const;
#endif

  static QString Decode(const TagLib::String& tag, const QTextCodec* codec);
  static QString Decode(const QString& tag, const QTextCodec* codec);

  // Save
  void BindToQuery(QSqlQuery* query) const;
  void BindToFtsQuery(QSqlQuery* query) const;
  void ToLastFM(lastfm::Track* track) const;

  // Simple accessors
  bool is_valid() const { return d->valid_; }
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

  int length() const { return d->length_; }
  int bitrate() const { return d->bitrate_; }
  int samplerate() const { return d->samplerate_; }

  int directory_id() const { return d->directory_id_; }
  const QString& filename() const { return d->filename_; }
  const QString& basefilename() const { return d->basefilename_; }
  uint mtime() const { return d->mtime_; }
  uint ctime() const { return d->ctime_; }
  int filesize() const { return d->filesize_; }
  FileType filetype() const { return d->filetype_; }

  const QString& art_automatic() const { return d->art_automatic_; }
  const QString& art_manual() const { return d->art_manual_; }

  const QImage& image() const { return d->image_; }

  // Pretty accessors
  QString PrettyTitle() const;
  QString PrettyTitleWithArtist() const;
  QString PrettyLength() const;

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
  void set_comment(const QString& v) { d->comment_ = v; }
  void set_compilation(bool v) { d->compilation_ = v; }
  void set_sampler(bool v) { d->sampler_ = v; }
  void set_length(int v) { d->length_ = v; }
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

  // Setters that should only be used by tests
  void set_filename(const QString& v) { d->filename_ = v; }
  void set_basefilename(const QString& v) { d->basefilename_ = v; } 
  void set_directory_id(int v) { d->directory_id_ = v; }

  // Comparison functions
  bool IsMetadataEqual(const Song& other) const;

 private:
  void GuessFileType(TagLib::FileRef* fileref);
  static bool Save(const Song& song);

  // Helper methods for taglib
  static void SetTextFrame(const QString& id, const QString& value,
                           TagLib::ID3v2::Tag* tag);

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

    int length_;
    int bitrate_;
    int samplerate_;

    int directory_id_;
    QString filename_;
    QString basefilename_;
    int mtime_;
    int ctime_;
    int filesize_;
    FileType filetype_;

    // Filenames to album art for this song.
    QString art_automatic_; // Guessed by LibraryWatcher
    QString art_manual_;    // Set by the user - should take priority

    QImage image_;
  };

 private:
  QSharedDataPointer<Private> d;
  FileRefFactory* factory_;

  static TagLibFileRefFactory kDefaultFactory;

  static QMutex taglib_mutex_;
};
Q_DECLARE_METATYPE(Song);

typedef QList<Song> SongList;
Q_DECLARE_METATYPE(SongList);

#endif // SONG_H
