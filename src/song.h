#ifndef SONG_H
#define SONG_H

#include <QImage>
#include <QList>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QSqlQuery>
#include <QString>

#include "engine_fwd.h"

namespace lastfm {
  class Track;
}

struct SongData : public QSharedData {
  SongData();

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
  bool compilation_;
  bool sampler_;

  int length_;
  int bitrate_;
  int samplerate_;

  int directory_id_;
  QString filename_;
  int mtime_;
  int ctime_;
  int filesize_;

  // Filenames to album art for this song.
  QString art_automatic_; // Guessed by LibraryWatcher
  QString art_manual_;    // Set by the user - should take priority

  QImage image_;
};

class Song {
 public:
  Song();
  Song(const Song& other);

  static const char* kColumnSpec;
  static const char* kBindSpec;
  static const char* kUpdateSpec;

  // Constructors
  void InitFromFile(const QString& filename, int directory_id);
  void InitFromQuery(const QSqlQuery& query);
  void InitFromLastFM(const lastfm::Track& track);
  void InitFromSimpleMetaBundle(const Engine::SimpleMetaBundle& bundle);

  // Save
  void BindToQuery(QSqlQuery* query) const;
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
  bool is_compilation() const { return d->compilation_ || d->sampler_; }

  int length() const { return d->length_; }
  int bitrate() const { return d->bitrate_; }
  int samplerate() const { return d->samplerate_; }

  int directory_id() const { return d->directory_id_; }
  const QString& filename() const { return d->filename_; }
  uint mtime() const { return d->mtime_; }
  uint ctime() const { return d->ctime_; }
  int filesize() const { return d->filesize_; }

  const QString& art_automatic() const { return d->art_automatic_; }
  const QString& art_manual() const { return d->art_manual_; }

  const QImage& image() const { return d->image_; }

  // Pretty accessors
  QString PrettyTitle() const;
  QString PrettyTitleWithArtist() const;
  QString PrettyLength() const;

  // Loads and returns some album art for the song.  Tries, in this order:
  //  1) An image set explicitly with set_image (eg. last.fm radio)
  //  2) An image set by the user with set_art_manual
  //  3) An image found by the library scanner
  QImage GetBestImage() const;

  // Setters
  bool IsEditable() const { return d->valid_ && !d->filename_.isNull(); }
  bool Save() const;

  void set_id(int id) { d->id_ = id; }
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
  void set_art_automatic(const QString& v) { d->art_automatic_ = v; }
  void set_art_manual(const QString& v) { d->art_manual_ = v; }
  void set_image(const QImage& i) { d->image_ = i; }

  // Setters that should only be used by tests
  void set_filename(const QString& v) { d->filename_ = v; }
  void set_directory_id(int v) { d->directory_id_ = v; }

  // Comparison functions
  bool IsMetadataEqual(const Song& other) const;

 private:
  QSharedDataPointer<SongData> d;
};

typedef QList<Song> SongList;

#endif // SONG_H
