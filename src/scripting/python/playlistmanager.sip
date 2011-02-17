class PlaylistManagerInterface : QObject {

%TypeHeaderCode
#include "playlist/playlistmanager.h"
%End

%Docstring
Container for all of Clementine's playlists.

Clementine supports multiple tabbed playlists - a user can create a new
playlist by clicking the C{New playlist} button at the top of the window, and
switch between them by using the tabs above each playlist.
This class manages the list of playlists and allows scripts to get at each
individual playlist, create new ones or change existing ones.

In the list of playlists, one playlist is the I{current} playlist and one is
the I{active} playlist:
  - The B{current} playlist is the one that is displayed in the PlaylistView
    right now - the user can make a different playlist current by clicking on
    another tab.  Actions such as clearing or shuffling the playlist, or
    adding new songs, should operate on the current playlist by default.
  - The B{active} playlist is the playlist that contains the currently
    playing song, or if the player is stopped, it is the playlist that will
    start playing when the user presses C{Play}.  The active and current
    playlists don't have to be the same (although they sometimes are) - songs
    can be playing from one playlist while another is displayed in the window.

There will always be at least one playlist in the PlaylistManager, and the
active and current playlists are guaranteed to always be valid.

Each playlist has an ID that is used to save it in the database.  You can get
the ID of a playlist by calling the L{Playlist.id()} method.

You can't create new PlaylistManagers, there is only one instance of
this class ever created.  You can access it through the global variable in the
L{clementine} module:

  >>> clementine.playlists
%End

public:
  int current_id() const;
%Docstring
current_id() -> int
Returns the ID of the current playlist.
%End

  int active_id() const;
%Docstring
active_id() -> int
Returns the ID of the active playlist.
%End

  Playlist* playlist(int id) const;
%Docstring
playlist(id) -> L{Playlist}
Returns the Playlist with the given identifier.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  Playlist* current() const;
%Docstring
current() -> L{Playlist}
Returns the current playlist.

Equivalent to C{playlist(current_id())}.
%End

  Playlist* active() const;
%Docstring
active() -> L{Playlist}
Returns the active playlist.

Equivalent to C{playlist(active_id())}.
%End

  const QList<Playlist*> GetAllPlaylists() const;
%Docstring
GetAllPlaylists() -> list of L{Playlist}s
Returns a list containing all the playlists.
%End

  const QItemSelection& selection(int id) const;
%Docstring
selection(id) -> PyQt4.QtGui.QItemSelection
Returns any selected items in the playlist with the given ID.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  const QItemSelection& current_selection() const;
%Docstring
current_selection() -> PyQt4.QtGui.QItemSelection
Returns any selected items in the current playlist.

Equivalent to C{selection(current_id())}.
%End

  const QItemSelection& active_selection() const;
%Docstring
active_selection() -> PyQt4.QtGui.QItemSelection
Returns any selected items in the active playlist.

Equivalent to C{selection(active_id())}.
%End

  QString name(int index) const;
%Docstring
name(id) -> str
Returns the name of the playlist with the given ID.

A playlist's name is shown in the tab bar above the playlist.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  LibraryBackend* library_backend() const;
%Docstring
library_backend() -> L{LibraryBackend}
Convenience function to return the local library backend.
%End

  // PlaylistBackend* playlist_backend() const;

  PlaylistSequence* sequence() const;
%Docstring
sequence() -> L{PlaylistSequence}
Convenience function to return the playlist sequence controller.

A playlist sequence controller manages the repeat/shuffle state.  All
playlists share the same playlist sequence controller.
%End

  PlaylistParser* parser() const;
%Docstring
parser() -> L{PlaylistParser}
Convenience function to return the playlist parser.

This is used when loading and saving playlists to files (eg. C{.pls} or
C{.xspf}) with the L{Load} and L{Save} methods.
%End

public slots:
  void New(const QString& name, const SongList& songs = SongList());
%Docstring
New(name, songs)
Creates a new playlist and optionally fills it with a list of songs.

The newly created playlist is made current and can be accessed with
L{current()}.

@param songs: A list of songs to add to the new playlist.  Defaults to the
  empty list.
@type songs: list of L{Song}s
%End

  void Load(const QString& filename);
%Docstring
Load(filename)
Creates a new playlist from a playlist file on disk.

The playlist file is loaded by using the L{PlaylistParser} returned by
L{parser()}.  The newly created playlist is made current and can be accessed
with L{current()}.

@note: If the file does not exist or can't be parsed for whatever reason, an
  error dialog is shown and no playlist is created.
%End

  void Save(int id, const QString& filename);
%Docstring
Save(id, filename)
Saves the playlist with the given ID to a file.

The playlist file is written by using the L{PlaylistParser} returned by
L{parser()}.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  void Rename(int id, const QString& new_name);
%Docstring
Rename(id, new_name)
Renames the playlist with the given ID.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  void Remove(int id);
%Docstring
Remove(id)
Removes the playlist with the given ID.

If you remove either the active or the current playlist then another playlist
will be made active/current to replace it.  It is not possible to remove the
last playlist - this function will silently return if you try.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  void ChangePlaylistOrder(const QList<int>& ids);
%Docstring
ChangePlaylistOrder(new_order)
Rearranges the order of the tabs in the tab bar.

@field new_order: A list of playlist IDs in the new desired order.
@type new_order: list of ints
%End

  void SetCurrentPlaylist(int id);
%Docstring
SetCurrentPlaylist(id)
Changes the current playlist (the one visible in the window) to the one with
the given ID.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  void SetActivePlaylist(int id);
%Docstring
SetActivePlaylist(id)
Changes the active playlist to the one with the given ID.

@warning: You MUST ensure that a playlist with this identifier exists before
  calling this method.  If you request an ID that doesn't exist the program
  will crash.
%End

  void SetActiveToCurrent();
%Docstring
SetActiveToCurrent()
Convenience function that sets the active playlist to be the same as the
current playlist.

This is equivalent to calling C{SetActivePlaylist(current_id())}.
%End

  void ClearCurrent();
%Docstring
ClearCurrent()
Convenience function to clear the current playlist.

Equivalent to calling C{current().Clear()}.
%End

  void ShuffleCurrent();
%Docstring
ShuffleCurrent()
Convenience function to shuffle the current playlist.

Equivalent to calling C{current().Shuffle()}.
%End

  void SetActivePlaying();
%Docstring
SetActivePlaying()
Convenience function to mark the current song in the active playlist as
Playing.

Equivalent to calling C{active().Playing()}.
%End

  void SetActivePaused();
%Docstring
SetActivePaused()
Convenience function to mark the current song in the active playlist as
Paused.

Equivalent to calling C{active().Paused()}.
%End

  void SetActiveStopped();
%Docstring
SetActiveStopped()
Convenience function to mark the current song in the active playlist as
Stopped.

Equivalent to calling C{active().Stopped()}.
%End

  void SetActiveStreamMetadata(const QUrl& url, const Song& song);
%Docstring
SetActiveStreamMetadata(url, song)
Convenience function to set the metadata of the currently playing song in
the active playlist.

Equivalent to calling C{active().SetStreamMetadata(url, song)}.

@type url: PyQt4.QtCore.QUrl
@type song: L{Song}
%End

  void RateCurrentSong(double rating);
%Docstring
RateCurrentSong(rating)
Convenience function to set the rating of the currently playing song in the
active playlist.

Equivalent to calling C{active().RateSong(active().current_index(), rating)}.

@param rating: from 0.0 to 1.0
@type rating: float
%End

  // void PlaySmartPlaylist(smart_playlists::GeneratorPtr generator, bool as_new, bool clear);

signals:
  void PlaylistManagerInitialized();

  void PlaylistAdded(int id, const QString& name);
  void PlaylistRemoved(int id);
  void PlaylistRenamed(int id, const QString& new_name);
  void CurrentChanged(Playlist* new_playlist);
  void ActiveChanged(Playlist* new_playlist);

  void Error(const QString& message);
  void SummaryTextChanged(const QString& summary);

  // Forwarded from individual playlists
  void CurrentSongChanged(const Song& song);

  // Signals that one of manager's playlists has changed (new items, new
  // ordering etc.) - the argument shows which.
  void PlaylistChanged(Playlist* playlist);
  void EditingFinished(const QModelIndex& index);
  void PlayRequested(const QModelIndex& index);

private:
  PlaylistManagerInterface();
};
