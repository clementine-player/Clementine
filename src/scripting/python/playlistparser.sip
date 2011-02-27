class PlaylistParser : QObject {
%TypeHeaderCode
  #include "playlistparsers/playlistparser.h"
%End

%Docstring
Utility class to read and write playlist files.

The PlaylistParser supports a number of different playlist formats - these are
implemeted by subclasses of L{ParserBase}.  Usually you don't need to worry
about finding the right parser for your file - PlaylistParser will do that
automatically based on the filename you give it (for example, it will use the
PlsParser if you ask it to load a file called C{playlist.pls}).

If you don't know the filename because you're reading data directly from the
network, PlaylistParser can also guess the type of parser to use by looking for
magic strings in the first few bytes of the file.  The constant C{kMagicSize}
tells you how many bytes the PlaylistParser will take when using magic.

PlaylistParser deals with loading and saving lists of L{Song} objects.

  >>> parser = clementine.PlaylistParser(clementine.library)
  ... songlist = parser.LoadFromFile("songs.xspf")
  ... print songlist[0].title()
  ... parser.Save(songlist, "songs.asx")

Notice that this class' constructor takes a L{LibraryBackend}.  Why does a
playlist parser need access to the library?  When loading a list of songs the
parser will check whether a song exists already in the library, and will use
the metadata that's already been loaded instead of loading it again.  You can
get Clementine's library by using C{clementine.library}.

@warning: The L{LoadFromFile()}, L{LoadFromDevice()} and L{Save()} methods in
  this class are B{blocking} and should not be called from the main thread.
  For a higher level interface to loading songs from playlists you should use
  the L{SongLoader} class which works in a background thread.
%End

public:
  PlaylistParser(LibraryBackend* library, QObject* parent /TransferThis/ = 0);

  static const int kMagicSize;

  QStringList file_extensions() const;
%Docstring
file_extensions() -> list of str
Returns the list of file extensions that the PlaylistParser knows how to parse.
%End

  QString filters() const;
%Docstring
filters() -> str
Returns a filter string suitable for use in L{PyQt4.QtGui.QFileDialog}.
%End

  QString default_extension() const;
%Docstring
default_extension() -> str
Returns the recommended extension to use when saving playlists.  Currently this
is C{xspf}.
%End

  QString default_filter() const;
%Docstring
default_filter() -> str
Returns the recommended default filter string suitable for use in
L{PyQt4.QtGui.QFileDialog}.
%End

  ParserBase* ParserForMagic(const QByteArray& data,
                             const QString& mime_type = QString()) const;
%Docstring
ParserForMagic(data, mime_type="") -> L{ParserBase}
Tries to guess the file type of a playlist based on the first few bytes of its
contents or (optionally) a mime type.

@param data: should be at least L{kMagicSize} bytes long.
@type data: str
@param mime_type: (optional) the mime type of the data obtained through (for
  example) and HTTP response header.
@type mime_type: str
@return: a parser for this file format, or None if it was not possible to guess.

@note: only use this method if you need direct access to the parser.  If you
  just want to load a list of songs from some data use L{LoadFromDevice()}.
%End

  ParserBase* ParserForExtension(const QString& suffix) const;
%Docstring
ParserForExtension(suffix) -> L{ParserBase}
Tried to guess the file type of a playlist based on its file extension.

@param suffix: the file extension, not including the C{.} character.
@type suffix: str
@return: a parser for this file format, or None if the extension wasn't
  recognised.

@note: only use this method if you need direct access to the parser.  If you
  just want to load a list of songs from a file use L{LoadFromFile()}.
%End

  SongList LoadFromFile(const QString& filename) const;
%Docstring
LoadFromFile(filename) -> list of L{Song}
Loads a playlist from a file and returns the list of songs that it contained.
Determines the type of the playlist based on the file extension by calling
L{ParserForExtension}.

@return: the list of songs in the playlist, or an empty list if the playlist
  couldn't be loaded.
%End

  SongList LoadFromDevice(QIODevice* device, const QString& path_hint = "",
                          const QDir& dir_hint = QDir()) const;
%Docstring
LoadFromDevice(device, path_hint="", dir_hint="") -> list of L{Song}
Loads a playlist from a L{PyQt4.QtCore.QIODevice} and returns the list of songs
that it contained.  Determines the type of the playlist by peeking at the first
L{kMagicSize} bytes of the device and using L{ParserForMagic}.

@param device: an open device on a playlist file.  You can use any device here,
  including L{PyQt4.QtNetwork.QTcpSocket}, L{PyQt4.QtCore.QFile} or
  L{PyQt4.QtCore.QBuffer}.
@type device: L{PyQt4.QtCore.QIODevice}
@param path_hint: (optional) the filename of the playlist.  Some parsers may
  use this filename to add additional information to the L{Song}s returned, for
  example the cue parser fills out the L{Song.cue_path()} field.
@type path_hint: str
@param dir_hint: (optional) the directory containing the playlist.  Parsers will
  use this directory to resolve any relative filenames in the playlist.
@type dir_hint: L{PyQt4.QtCore.QDir}
@return: the list of songs in the playlist, or an empty list if the playlist
  couldn't be loaded.
%End

  void Save(const SongList& songs, const QString& filename) const;
%Docstring
Save(songs, filename)
Saves a list of songs to a new playlist.  Determines the type of the playlist
to create based on the file extension by calling L{ParserForExtension}.

@type songs: list of L{Song}
@type filename: str
%End
};
