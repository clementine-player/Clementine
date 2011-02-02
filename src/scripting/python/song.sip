class Song {

%TypeHeaderCode
#include "core/song.h"
%End

public:
  Song();
  Song(const Song& other);

  enum FileType {
    Type_Unknown,
    Type_Asf,
    Type_Flac,
    Type_Mp4,
    Type_Mpc,
    Type_Mpeg,
    Type_OggFlac,
    Type_OggSpeex,
    Type_OggVorbis,
    Type_Aiff,
    Type_Wav,
    Type_TrueAudio,

    Type_Stream,
  };

  static QString TextForFiletype(FileType type);
  QString TextForFiletype() const;

  // Constructors
  void Init(const QString& title, const QString& artist, const QString& album, int length);
  void Init(const QString& title, const QString& artist, const QString& album, int beginning, int end);
  void InitFromFile(const QString& filename, int directory_id);

  // Simple accessors
  bool is_valid() const;
  int id() const;

  QString title() const;
  QString album() const;
  QString artist() const;
  QString albumartist() const;
  QString composer() const;
  int track() const;
  int disc() const;
  float bpm() const;
  int year() const;
  const QString& genre() const;
  const QString& comment() const;
  bool is_compilation() const;
  float rating() const;
  int playcount() const;
  int skipcount() const;
  int lastplayed() const;
  int score() const;

  const QString& cue_path() const;
  bool has_cue() const;

  int beginning() const;
  int end() const;

  int length() const;

  int bitrate() const;
  int samplerate() const;

  int directory_id() const;
  const QString& filename() const;
  const QString& basefilename() const;
  uint mtime() const;
  uint ctime() const;
  int filesize() const;
  FileType filetype() const;

  QString art_automatic() const;
  QString art_manual() const;

  bool has_manually_unset_cover() const;
  void manually_unset_cover();

  bool has_embedded_cover() const;
  void set_embedded_cover();

  QImage image() const;

  // Pretty accessors
  QString PrettyTitle() const;
  QString PrettyTitleWithArtist() const;
  QString PrettyLength() const;
  QString PrettyYear() const;

  QString TitleWithCompilationArtist() const;

  // Setters
  bool IsEditable() const;
  bool Save() const;
  void BackgroundSave() const;

  void set_id(int id);
  void set_valid(bool v);
  void set_title(const QString& v);

  void set_album(const QString& v);
  void set_artist(const QString& v);
  void set_albumartist(const QString& v);
  void set_composer(const QString& v);
  void set_track(int v);
  void set_disc(int v);
  void set_bpm(float v);
  void set_year(int v);
  void set_genre(const QString& v);
  void set_genre(int id);
  void set_comment(const QString& v);
  void set_compilation(bool v);
  void set_sampler(bool v);
  void set_beginning(int v);
  void set_end(int v);
  void set_length(int v);
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
  void set_filename(const QString& v);
  void set_basefilename(const QString& v);
  void set_directory_id(int v);
  void set_cue_path(const QString& v);

  // Comparison functions
  bool IsMetadataEqual(const Song& other) const;
};
typedef QList<Song> SongList;
