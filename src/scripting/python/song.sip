class Song {

%TypeHeaderCode
#include "core/song.h"
%End

%Docstring
Contains metadata and information about a music file.

Song objects are used throughout Clementine to represent music files and all the
metadata that goes along with them.  A Song contains several groups of
functions:

  - The B{constructor} functions L{Init()} and L{InitFromFile()} that are used
    to quickly initialise the contents of a Song, either using the values you
    provide or by reading the tags in a file on disk.
  - The B{getters} such as L{title()} and L{artist()} that return the data
    stored in a Song.
  - The B{setters} such as L{set_title()} and L{set_artist()} that let you
    change metadata in a Song.  New metadata isn't written to the actual file
    straight away, you have to call L{Save()} or L{BackgroundSave()} first.
  - The B{pretty getters} such as L{PrettyTitleWithArtist()} that format the
    Song's information in various useful ways.

Integer fields in the song like L{track} and L{year} will be C{-1} if they
don't have a value set.

You can get a Song object for the currently playing track by using
L{Player.GetCurrentItem()} and L{PlaylistItem.Metadata()}:

  >>> song = clementine.player.GetCurrentItem().Metadata()
  ... print song.title()

Note that Song objects are dumb containers of information - changing the fields
in one won't automatically change the information in Clementine's database or
save the tags to the file.  If you want to update information in the database
such as album art, play count, skip count, rating or whether the song is in a
compilation, use the methods in L{LibraryBackend}.  If you want to save tags
to a music file, use the L{Save()} or L{BackgroundSave()} methods in this class
after modifying the relevant fields.

Album art for songs in Clementine is split into three different fields -
L{art_automatic()}, L{art_manual()} and L{image()}.  L{art_automatic()} contains
the art that was found automatically by the library scanner, for example a jpeg
file in the same directory as the music file.  L{art_manual()} contains the art
that was set manually by the user - if it is present it always takes priority
over L{art_automatic()}.  These fields contain either a string with the filename
or URL of an image file, or one of two special values - L{kManuallyUnsetCover}
or L{kEmbeddedCover}.  The convenience functions L{has_manually_unset_cover()}
and L{has_embedded_cover()} check for these special values.  L{image()} is a
special field that is only currently used by radio services that want to
display the radio station's logo and override the other two C{art_} fields.

Some songs are marked as being in a compilation.  These songs are shown in the
special "Various Artists" section in the library.  There are three different
ways a song can be marked as being in a compilation.  L{set_compilation()} is
set by the library scanner when it finds two or more music files in the same
directory with different artists.  L{set_sampler()} is set by L{InitFromFile()}
if the C{sampler} field in the ID3v2 metadata is present.
L{set_forced_compilation_on()} and L{set_forced_compilation_off()} are set if
the user chooses C{Show in various artists} from the right-click menu in the
Library.  The L{is_compilation()} function returns True if the song has been
marked in any of these ways.

@group Class methods: TextForFiletype
@group Constructors: Init, InitFromFile
@group Save methods: Save, BackgroundSave
@group Simple getters: is_valid, id, title, album, artist, albumartist,
  composer, track, disc, bpm, year, genre, comment, is_compilation, rating,
  playcount, skipcount, lastplayed, score, cue_path, has_cue, beginning_nanosec,
  end_nanosec, length_nanosec, bitrate, samplerate, directory_id, filename,
  basefilename, mtime, ctime, filesize, filetype, art_automatic, art_manual,
  has_manually_unset_cover, has_embedded_cover, image, url, IsEditable,
  IsMetadataEqual
@group Pretty getters: PrettyTitle, PrettyTitleWithArtist, PrettyLength,
  PrettyYear, TitleWithCompilationArtist
@group Simple setters: manually_unset_cover, set_embedded_cover, set_id,
  set_valid, set_title, set_album, set_artist, set_albumartist, set_composer,
  set_track, set_disc, set_bpm, set_year, set_genre, set_genre_id3,
  set_comment, set_compilation, set_sampler, set_beginning_nanosec,
  set_end_nanosec, set_length_nanosec, set_bitrate, set_samplerate, set_mtime,
  set_ctime, set_filesize, set_filetype, set_art_automatic, set_art_manual,
  set_image, set_forced_compilation_on, set_forced_compilation_off, set_rating,
  set_playcount, set_skipcount, set_lastplayed, set_score, set_cue_path,
  set_filename, set_basefilename, set_directory_id
%End

public:
  Song();
  Song(const Song& other);

  static const QString kManuallyUnsetCover;
  static const QString kEmbeddedCover;

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
%Docstring
TextForFiletype(type) -> str
Returns a textual description of one of the C{Type_} constants.

Example:

  >>> print clementine.Song.TextForFiletype(clementine.Song.Type_OggVorbis)
  "Ogg Vorbis"
%End

  QString TextForFiletype() const;

  void Init(const QString& title, const QString& artist, const QString& album, qint64 length_nanosec);
%Docstring
Init(title, artist, album, length_nanosec)
Sets the title, artist, album and length in the Song, and also marks it as
valid.
%End

  void Init(const QString& title, const QString& artist, const QString& album, qint64 beginning, qint64 end);
  void InitFromFile(const QString& filename, int directory_id);
%Docstring
InitFromFile(filename, directory_id)
Loads tags from a music file on disk and marks the Song as valid if it was
successful.

@param directory_id: The ID of the directory containing this Song in the
  L{LibraryBackend}.  You can pass -1 to this function to indicate the file is
  not in the library.
%End

  bool is_valid() const;
%Docstring
is_valid() -> bool
Flag used to indicate whether an L{InitFromFile()} call returned successfully.
%End

  int id() const;
%Docstring
id() -> int
Identifier for this song in the L{LibraryBackend} - C{-1} if this song is not
from the library.
%End

  QString title() const;
%Docstring
title() -> str
%End

  QString album() const;
%Docstring
album() -> str
%End

  QString artist() const;
%Docstring
artist() -> str
%End

  QString albumartist() const;
%Docstring
albumartist() -> str
%End

  QString composer() const;
%Docstring
composer() -> str
%End

  int track() const;
%Docstring
track() -> int
%End

  int disc() const;
%Docstring
disc() -> int
%End

  float bpm() const;
%Docstring
bpm() -> float
%End

  int year() const;
%Docstring
year() -> int
%End

  const QString& genre() const;
%Docstring
genre() -> str
%End

  const QString& comment() const;
%Docstring
comment() -> str
%End

  bool is_compilation() const;
%Docstring
is_compilation() -> bool
%End

  float rating() const;
%Docstring
rating() -> float
The song's user-assigned rating between 0.0 and 1.0.
%End

  int playcount() const;
%Docstring
playcount() -> int
Number of times this song has been played by Clementine.
%End

  int skipcount() const;
%Docstring
skipcount() -> int
Number of times this song has been skipped by the user pressing Next.
%End

  int lastplayed() const;
%Docstring
lastplayed() -> int
The time (in seconds since the epoch) this song was last played by Clementine.

You can convert this time to a python C{datetime} object by using
C{datetime.datetime.fromtimestamp()}.
%End

  int score() const;
%Docstring
score() -> int
The automatically generated score assigned to this song based on the
L{rating()}, L{playcount()} and L{skipcount()}.
%End

  const QString& cue_path() const;
%Docstring
cue_path() -> str
If this song was loaded from a cuesheet then this contains the path to the
cuesheet.

@see: L{has_cue()}, L{beginning_nanosec()}, L{end_nanosec()}
%End

  bool has_cue() const;
%Docstring
has_cue() -> bool
Whether this song was loaded as part of a cuesheet.

@see: L{cue_path()}, L{beginning_nanosec()}, L{end_nanosec()}
%End

  qint64 beginning_nanosec() const;
%Docstring
beginning_nanosec() -> long
The index in nanoseconds into the file where this song begins.

This is normally 0 except when the song is part of a cuesheet.

@see: L{has_cue()}, L{end_nanosec()}
%End

  qint64 end_nanosec() const;
%Docstring
end_nanosec() -> long
The index in nanoseconds into the file where this song ends.

This is normally 0 except when the song is part of a cuesheet.

@see: L{has_cue()}, L{beginning_nanosec()}
%End

  qint64 length_nanosec() const;
%Docstring
length_nanosec() -> long
The length of the song in nanoseconds.
%End

  int bitrate() const;
%Docstring
bitrate() -> int
%End

  int samplerate() const;
%Docstring
samplerate() -> int
%End

  int directory_id() const;
%Docstring
directory_id() -> int
The ID in the L{LibraryBackend} of the directory containing this song, or -1 if
the song is not from the library.
%End

  const QString& filename() const;
%Docstring
filename() -> str
The filename I{or URL} of this song.
%End

  const QString& basefilename() const;
%Docstring
basefilename() -> str
The filename of this song without any directory component.
%End

  QUrl url() const;
%Docstring
url() -> QUrl
URL for this song which may point either to a file or to another type of stream.
%End

  uint mtime() const;
%Docstring
mtime() -> int
The time in seconds since the epoch that this song was modified.

You can convert this time to a python C{datetime} object by using
C{datetime.datetime.fromtimestamp()}.

@see: L{ctime()}
%End

  uint ctime() const;
%Docstring
ctime() -> int
The time in seconds since the epoch that this song was created.

You can convert this time to a python C{datetime} object by using
C{datetime.datetime.fromtimestamp()}.

@see: L{mtime()}
%End

  int filesize() const;
%Docstring
filesize() -> int
The size of this file in bytes.
%End

  FileType filetype() const;
%Docstring
filetype() -> FileType
The type of media file.

@see: L{TextForFiletype}
%End

  QString art_automatic() const;
%Docstring
art_automatic() -> str
The filename or URL of automatically discovered album art, or one of the special
values  L{kManuallyUnsetCover} or L{kEmbeddedCover}.

@see: L{art_manual()}, L{has_manually_unset_cover()}, L{has_embedded_cover()}
%End

  QString art_manual() const;
%Docstring
art_manual() -> str
The filename or URL of album art set by the user, or one of the special
values  L{kManuallyUnsetCover} or L{kEmbeddedCover}.

@see: L{art_automatic()}, L{has_manually_unset_cover()}, L{has_embedded_cover()}
%End

  bool has_manually_unset_cover() const;
%Docstring
has_manually_unset_cover() -> bool
Returns True if the album art has been manually unset by the user.

@see: L{art_manual()}, L{manually_unset_cover()}
%End

  void manually_unset_cover();
%Docstring
manually_unset_cover()
Marks this song as having a manually unset album cover.

@see: L{art_manual()}, L{has_manually_unset_cover()}
%End

  bool has_embedded_cover() const;
%Docstring
has_embedded_cover() -> bool
Returns True if the album art is embedded within the file.

@see: L{art_automatic()}, L{set_embedded_cover()}
%End

  void set_embedded_cover();
%Docstring
set_embedded_cover()
Marks this song as having album art contained within it.

@see: L{art_automatic()}, L{has_embedded_cover()}
%End

  QImage image() const;
%Docstring
image() -> L{PyQt4.QtGui.QImage}
An album art image that overrides L{art_automatic()} and L{art_manual()}.
%End

  // Pretty accessors
  QString PrettyTitle() const;
%Docstring
PrettyTitle() -> str
Returns the title of this song if it is set, or else returns the filename.
%End

  QString PrettyTitleWithArtist() const;
%Docstring
PrettyTitleWithArtist() -> str
Returns "artist - title" if both are set, otherwise returns L{PrettyTitle()}.
%End

  QString PrettyLength() const;
%Docstring
PrettyLength() -> str
Returns the length of the song formatted as "(hh:)mm:ss".
%End

  QString PrettyYear() const;
%Docstring
PrettyYear() -> str
Returns the year if it is set, otherwise returns "".
%End

  QString TitleWithCompilationArtist() const;
%Docstring
TitleWithCompilationArtist() -> str
If this song is part of a compilation B{and} the artist is B{not} "Various
Artists", returns L{PrettyTitleWithArtist()}, otherwise returns L{PrettyTitle()}.
%End

  // Setters
  bool IsEditable() const;
%Docstring
IsEditable() -> bool
Returns True if this song is backed by a local file that is writable and is not
part of a cue sheet.
%End

  bool Save() const;
%Docstring
Save() -> bool
Saves the metadata to the file and returns True if it was successful.

@warning: This method is B{blocking} and should not be called from the main
  thread.  Use the L{BackgroundSave()} method instead which works in a
  background thread.
%End

  void BackgroundSave() const;
%Docstring
BackgroundSave()
Saves the metadata to the file in a background thread and returns immediately.
%End

  void set_id(int id);
%Docstring
set_id(id)
%End

  void set_valid(bool v);
%Docstring
set_valid(valid)
%End

  void set_title(const QString& v);
%Docstring
set_title(title)
%End


  void set_album(const QString& v);
%Docstring
set_album(album)
%End

  void set_artist(const QString& v);
%Docstring
set_artist(artist)
%End

  void set_albumartist(const QString& v);
%Docstring
set_albumartist(albumartist)
%End

  void set_composer(const QString& v);
%Docstring
set_composer(composer)
%End

  void set_track(int v);
%Docstring
set_track(track)
%End

  void set_disc(int v);
%Docstring
set_disc(disc)
%End

  void set_bpm(float v);
%Docstring
set_bpm(bpm)
%End

  void set_year(int v);
%Docstring
set_year(year)
%End

  void set_genre(const QString& v);
%Docstring
set_genre(genre)
%End

  void set_genre_id3(int id);
%Docstring
set_genre_id3(id)
Sets the genre from an ID3v1 genre ID.  See
U{Appendix A of the ID3v1 specification<http://www.id3.org/id3v2.3.0#head-129376727ebe5309c1de1888987d070288d7c7e7>}.
%End

  void set_comment(const QString& v);
%Docstring
set_comment(comment)
%End

  void set_compilation(bool v);
%Docstring
set_compilation(compilation)
%End

  void set_sampler(bool v);
%Docstring
set_sampler(sampler)
%End

  void set_beginning_nanosec(qint64 v);
%Docstring
set_beginning_nanosec(nanosec)
%End

  void set_end_nanosec(qint64 v);
%Docstring
set_end_nanosec(nanosec)
%End

  void set_length_nanosec(qint64 v);
%Docstring
set_length_nanosec(nanosec)
%End

  void set_bitrate(int v);
%Docstring
set_bitrate(bitrate)
%End

  void set_samplerate(int v);
%Docstring
set_samplerate(samplerate)
%End

  void set_mtime(int v);
%Docstring
set_mtime(mtime)
%End

  void set_ctime(int v);
%Docstring
set_ctime(ctime)
%End

  void set_filesize(int v);
%Docstring
set_filesize(size)
%End

  void set_filetype(FileType v);
%Docstring
set_filetype(type)
%End

  void set_art_automatic(const QString& v);
%Docstring
set_art_automatic(art)
%End

  void set_art_manual(const QString& v);
%Docstring
set_art_manual(art)
%End

  void set_image(const QImage& i);
%Docstring
set_image(image)
%End

  void set_forced_compilation_on(bool v);
%Docstring
set_forced_compilation_on(compilation)
%End

  void set_forced_compilation_off(bool v);
%Docstring
set_forced_compilation_off(compilation)
%End

  void set_rating(float v);
%Docstring
set_rating(rating)
%End

  void set_playcount(int v);
%Docstring
set_playcount(playcount)
%End

  void set_skipcount(int v);
%Docstring
set_skipcount(skipcount)
%End

  void set_lastplayed(int v);
%Docstring
set_lastplayed(lastplayed)
%End

  void set_score(int v);
%Docstring
set_score(score)
%End

  void set_filename(const QString& v);
%Docstring
set_filename(filename)
%End

  void set_basefilename(const QString& v);
%Docstring
set_basefilename(basefilename)
%End

  void set_directory_id(int v);
%Docstring
set_directory_id(directory_id)
%End

  void set_cue_path(const QString& v);
%Docstring
set_cue_path(cue_path)
%End

  bool IsMetadataEqual(const Song& other) const;
%Docstring
IsMetadataEqual(other) -> bool
Compares the metadata (not including ratings, playcounts, etc) with another
Song and returns true if they are equal.
%End
};
typedef QList<Song> SongList;
