#ifndef SONG_H
#define SONG_H

#include <QString>
#include <QList>
#include <QSqlQuery>
#include <QSharedData>
#include <QSharedDataPointer>

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

  int length_;
  int bitrate_;
  int samplerate_;

  int directory_id_;
  QString filename_;
  int mtime_;
  int ctime_;
  int filesize_;
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

  // Save
  void BindToQuery(QSqlQuery* query) const;
  void ToLastFM(lastfm::Track* track) const;

  // Simple accessors
  bool is_valid() const { return d->valid_; }
  int id() const { return d->id_; }

  QString title() const { return d->title_; }
  QString album() const { return d->album_; }
  QString artist() const { return d->artist_; }
  QString albumartist() const { return d->albumartist_; }
  QString composer() const { return d->composer_; }
  int track() const { return d->track_; }
  int disc() const { return d->disc_; }
  float bpm() const { return d->bpm_; }
  int year() const { return d->year_; }
  QString genre() const { return d->genre_; }
  QString comment() const { return d->comment_; }
  bool is_compilation() const { return d->compilation_; }

  int length() const { return d->length_; }
  int bitrate() const { return d->bitrate_; }
  int samplerate() const { return d->samplerate_; }

  int directory_id() const { return d->directory_id_; }
  QString filename() const { return d->filename_; }
  uint mtime() const { return d->mtime_; }
  uint ctime() const { return d->ctime_; }
  int filesize() const { return d->filesize_; }

  // Pretty accessors
  QString PrettyTitle() const;
  QString PrettyTitleWithArtist() const;
  QString PrettyLength() const;

  // Setters
  void set_id(int id) { d->id_ = id; }
  void set_title(const QString& title) { d->title_ = title; }

  // Comparison functions
  bool IsMetadataEqual(const Song& other) const;

 private:
  QSharedDataPointer<SongData> d;
};

typedef QList<Song> SongList;

#endif // SONG_H
