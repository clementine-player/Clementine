/* This file is part of Clementine.
   Copyright 2009-2012, David Sansome <me@davidsansome.com>
   Copyright 2010-2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2010, 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011-2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2011, Angus Gratton <gus@projectgus.com>
   Copyright 2012, Kacper "mattrick" Banasik <mattrick@jabster.pl>
   Copyright 2013, Martin Brodbeck <martin@brodbeck-online.de>
   Copyright 2013, Joel Bradshaw <cincodenada@gmail.com>
   Copyright 2013, Uwe Klotz <uwe.klotz@gmail.com>
   Copyright 2013, Mateusz Kowalczyk <fuuzetsu@fuuzetsu.co.uk>
   Copyright 2014, Andreas <asfa194@gmail.com>
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

#ifndef CORE_SONG_H_
#define CORE_SONG_H_

#include <QImage>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QVariantMap>

#include "config.h"
#include "engines/engine_fwd.h"

namespace pb {
namespace tagreader {
class SongMetadata;
}  // namespace tagreader
}  // namespace pb

class QSqlQuery;
class QUrl;

#ifdef HAVE_LIBGPOD
struct _Itdb_Track;
#endif

#ifdef HAVE_LIBMTP
struct LIBMTP_track_struct;
#endif

#ifdef HAVE_LIBLASTFM
namespace lastfm {
class Track;
}
#endif

class SqlRow;

class Song {
 public:
  Song();
  Song(const Song& other);
  ~Song();

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

  // Don't change these values - they're stored in the database, and defined
  // in the tag reader protobuf.
  // If a new lossless file is added, also add it to IsFileLossless().
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
    Type_OggOpus = 13,
    Type_WavPack = 14,
    Type_Stream = 99,
  };
  static QString TextForFiletype(FileType type);
  QString TextForFiletype() const { return TextForFiletype(filetype()); }
  bool IsFileLossless() const;

  // Sort songs alphabetically using their pretty title
  static void SortSongsListAlphabetically(QList<Song>* songs);

  // Constructors
  void Init(const QString& title, const QString& artist, const QString& album,
            qint64 length_nanosec);
  void Init(const QString& title, const QString& artist, const QString& album,
            qint64 beginning, qint64 end);
  void InitFromProtobuf(const pb::tagreader::SongMetadata& pb);
  void InitFromQuery(const SqlRow& query, bool reliable_metadata, int col = 0);
  void InitFromFilePartial(
      const QString& filename);  // Just store the filename: incomplete but fast
  void InitArtManual();  // Check if there is already a art in the cache and
                         // store the filename in art_manual
#ifdef HAVE_LIBLASTFM
  void InitFromLastFM(const lastfm::Track& track);
#endif

  void MergeFromSimpleMetaBundle(const Engine::SimpleMetaBundle& bundle);

#ifdef HAVE_LIBGPOD
  void InitFromItdb(const _Itdb_Track* track, const QString& prefix);
  void ToItdb(_Itdb_Track* track) const;
#endif

#ifdef HAVE_LIBMTP
  void InitFromMTP(const LIBMTP_track_struct* track, const QString& host);
  void ToMTP(LIBMTP_track_struct* track) const;
#endif

  // Copies important statistics from the other song to this one, overwriting
  // any data that already exists.  Useful when you want updated tags from disk
  // but you want to keep user stats.
  void MergeUserSetData(const Song& other);

  static QString Decode(const QString& tag, const QTextCodec* codec = nullptr);

  // Save
  void BindToQuery(QSqlQuery* query) const;
  void BindToFtsQuery(QSqlQuery* query) const;
#ifdef HAVE_LIBLASTFM
  void ToLastFM(lastfm::Track* track, bool prefer_album_artist) const;
#endif
  void ToXesam(QVariantMap* map) const;
  void ToProtobuf(pb::tagreader::SongMetadata* pb) const;

  // Simple accessors
  bool is_valid() const;
  bool is_unavailable() const;
  int id() const;

  const QString& title() const;
  const QString& album() const;
  const QString& effective_album() const;
  const QString& artist() const;
  const QString& albumartist() const;
  const QString& effective_albumartist() const;
  // Playlist views are special because you don't want to fill in album artists
  // automatically for
  // compilations, but you do for normal albums:
  const QString& playlist_albumartist() const;
  const QString& composer() const;
  const QString& performer() const;
  const QString& grouping() const;
  const QString& lyrics() const;
  int track() const;
  int disc() const;
  float bpm() const;
  int year() const;
  int originalyear() const;
  int effective_originalyear() const;
  const QString& genre() const;
  const QString& comment() const;
  bool is_compilation() const;
  float rating() const;
  int playcount() const;
  int skipcount() const;
  int lastplayed() const;
  int score() const;
  int album_id() const;

  const QString& cue_path() const;
  bool has_cue() const;

  qint64 beginning_nanosec() const;
  qint64 end_nanosec() const;

  qint64 length_nanosec() const;

  int bitrate() const;
  int samplerate() const;

  int directory_id() const;
  const QUrl& url() const;
  const QString& basefilename() const;
  uint mtime() const;
  uint ctime() const;
  int filesize() const;
  FileType filetype() const;
  bool is_stream() const;
  bool is_cdda() const;
  // Guess if it is a library song using fields set. Might not be 100% reliable.
  bool is_library_song() const;

  const QString& art_automatic() const;
  const QString& art_manual() const;

  const QString& etag() const;

  // Returns true if this Song had it's cover manually unset by user.
  bool has_manually_unset_cover() const;
  // This method represents an explicit request to unset this song's
  // cover.
  void manually_unset_cover();

  // Returns true if this song (it's media file) has an embedded cover.
  bool has_embedded_cover() const;
  // Sets a flag saying that this song (it's media file) has an embedded
  // cover.
  void set_embedded_cover();

  const QImage& image() const;

  // Pretty accessors
  QString PrettyRating() const;
  QString PrettyTitle() const;
  QString PrettyTitleWithArtist() const;
  QString PrettyLength() const;
  QString PrettyYear() const;

  QString TitleWithCompilationArtist() const;

  // Setters
  bool IsEditable() const;

  void set_id(int id);
  void set_valid(bool v);
  void set_title(const QString& v);

  void set_album(const QString& v);
  void set_artist(const QString& v);
  void set_albumartist(const QString& v);
  void set_composer(const QString& v);
  void set_performer(const QString& v);
  void set_grouping(const QString& v);
  void set_lyrics(const QString& v);
  void set_track(int v);
  void set_disc(int v);
  void set_bpm(float v);
  void set_year(int v);
  void set_originalyear(int v);
  void set_genre(const QString& v);
  void set_genre_id3(int id);
  void set_comment(const QString& v);
  void set_compilation(bool v);
  void set_sampler(bool v);
  void set_album_id(int v);
  void set_beginning_nanosec(qint64 v);
  void set_end_nanosec(qint64 v);
  void set_length_nanosec(qint64 v);
  void set_bitrate(int v);
  void set_samplerate(int v);
  void set_mtime(int v);
  void set_ctime(int v);
  void set_filesize(int v);
  void set_filetype(FileType v);
  void set_art_automatic(const QString& v);
  void set_art_manual(const QString& v);
  void set_image(const QImage& i);
  void set_forced_compilation_on(bool v);
  void set_forced_compilation_off(bool v);
  void set_rating(float v);
  void set_playcount(int v);
  void set_skipcount(int v);
  void set_lastplayed(int v);
  void set_score(int v);
  void set_cue_path(const QString& v);
  void set_unavailable(bool v);
  void set_etag(const QString& etag);

  // Setters that should only be used by tests
  void set_url(const QUrl& v);
  void set_basefilename(const QString& v);
  void set_directory_id(int v);

  // Comparison functions
  bool IsMetadataEqual(const Song& other) const;
  bool IsOnSameAlbum(const Song& other) const;
  bool IsSimilar(const Song& other) const;

  bool operator==(const Song& other) const;

  // Two songs that are on the same album will have the same AlbumKey.  It is
  // more efficient to use IsOnSameAlbum, but this function can be used when
  // you need to hash the key to do fast lookups.
  QString AlbumKey() const;

  Song& operator=(const Song& other);

 private:
  struct Private;
  QSharedDataPointer<Private> d;
};
Q_DECLARE_METATYPE(Song);

typedef QList<Song> SongList;
Q_DECLARE_METATYPE(QList<Song>);

uint qHash(const Song& song);
// Hash function using field checked in IsSimilar function
uint HashSimilar(const Song& song);

#endif  // CORE_SONG_H_
