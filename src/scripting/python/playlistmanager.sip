class PlaylistManagerInterface : QObject /PyName=PlaylistManager/ {

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

@group Signals: PlaylistManagerInitialized, PlaylistAdded, PlaylistRemoved,
  PlaylistRenamed, CurrentChanged, ActiveChanged, Error, SummaryTextChanged,
  CurrentSongChanged, PlaylistChanged, EditingFinished, PlayRequested
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

  void InvalidateDeletedSongs();
%Docstring
InvalidateDeletedSongs()
Grays out and reloads all deleted songs in all playlists. Also, "ungreys"
those songs which were once deleted but now got restored somehow.
%End

void RemoveDeletedSongs();
%Docstring
RemoveDeletedSongs()
Removes all deleted songs from all playlists.
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

@param new_order: A list of playlist IDs in the new desired order.
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
%Docstring
PlaylistManagerInitialized()
Emitted during Clementine startup when all the playlists stored in the
database have been created and added to the UI.
%End

  void PlaylistAdded(int id, const QString& name);
%Docstring
PlaylistAdded(id, name)
Emitted when a new playlist is added.
%End

  void PlaylistRemoved(int id);
%Docstring
PlaylistRemoved(id)
Emitted when a playlist is removed.
%End

  void PlaylistRenamed(int id, const QString& new_name);
%Docstring
PlaylistRenamed(id, new_name)
Emitted when a playlist is renamed.
%End

  void CurrentChanged(Playlist* new_playlist);
%Docstring
CurrentChanged(new_current_playlist)
Emitted when the current playlist is changed (ie the user clicked on a
different tab).
%End

  void ActiveChanged(Playlist* new_playlist);
%Docstring
ActiveChanged(new_active_playlist)
Emitted when the active playlist is changed.
%End

  void Error(const QString& message);
%Docstring
Error(message)
Emitted whenever an error occurs in the PlaylistManager.

Errors are typically raised when loading a playlist file from disk.  Note: any
errors emitted through this signal will already have been displayed in an
error dialog.
%End

  void SummaryTextChanged(const QString& summary);
%Docstring
SummaryTextChanged(summary)
The summary text is a short string displayed in Clementine's status bar to say
how many (if any) songs are selected, and the total length of the songs in the
current playlist.

This signal is emitted whenever that text changes, for example when the
current playlist is changed, or when the user selects some different songs.
%End

  void CurrentSongChanged(const Song& song);
%Docstring
CurrentSongChanged(new_current_song)
Emitted when the currently playling song in the active playlist changes.

This is a convenience signal that is forwarded from the active playlist.  The
current song is the song that is currently playing - it has an animation and
a special appearance in the playlist view.

@type new_current_song: L{Song}
%End

  // Signals that one of manager's playlists has changed (new items, new
  // ordering etc.) - the argument shows which.
  void PlaylistChanged(Playlist* playlist);
%Docstring
PlaylistChanged(playlist)
Emitted when any playlist changes (items added, items removed, order changed,
etc.).

This is a convenience signal that is forwarded from individual playlists.

@type playlist: L{Playlist}
%End

  void EditingFinished(const QModelIndex& index);
%Docstring
EditingFinished(index)
Emitted after the user has made a change to one of the indexes in the current
playlist.

@type index: L{PyQt4.QtCore.QModelIndex}
%End

  void PlayRequested(const QModelIndex& index);
%Docstring
PlayRequested(index)
Emitted after songs are added to a playlist if the user wants them to start
playing immediately (eg by double clicking on a song in the library view).

@param index: The first of the newly added indexes in the playlist.
@type index: L{PyQt4.QtCore.QModelIndex}
%End

private:
  PlaylistManagerInterface();
};
