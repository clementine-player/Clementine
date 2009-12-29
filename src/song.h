#ifndef SONG_H
#define SONG_H

#include <QString>
#include <QList>
#include <QSqlQuery>

namespace lastfm {
  class Track;
}

// TODO: QSharedData
class Song {
 public:
  Song();

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
  bool is_valid() const { return valid_; }
  int id() const { return id_; }

  QString title() const { return title_; }
  QString album() const { return album_; }
  QString artist() const { return artist_; }
  QString albumartist() const { return albumartist_; }
  QString composer() const { return composer_; }
  int track() const { return track_; }
  int disc() const { return disc_; }
  float bpm() const { return bpm_; }
  int year() const { return year_; }
  QString genre() const { return genre_; }
  QString comment() const { return comment_; }
  bool is_compilation() const { return compilation_; }

  int length() const { return length_; }
  int bitrate() const { return bitrate_; }
  int samplerate() const { return samplerate_; }

  int directory_id() const { return directory_id_; }
  QString filename() const { return filename_; }
  uint mtime() const { return mtime_; }
  uint ctime() const { return ctime_; }
  int filesize() const { return filesize_; }

  // Pretty accessors
  QString PrettyTitle() const;
  QString PrettyTitleWithArtist() const;
  QString PrettyLength() const;

  // Setters
  void set_id(int id) { id_ = id; }
  void set_title(const QString& title) { title_ = title; }

  // Comparison functions
  bool IsMetadataEqual(const Song& other) const;

 private:
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

typedef QList<Song> SongList;

#endif // SONG_H
