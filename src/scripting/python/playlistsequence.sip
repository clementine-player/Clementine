// The QWidget base class is deliberately hidden
struct PlaylistSequence {

%TypeHeaderCode
#include "playlist/playlistsequence.h"
%End

%Docstring
Contains the shuffle and repeat state of the player.

This state is shared between all playlists.  You can access the global
PlaylistSequence instance through the L{PlaylistManager}:

  >>> sequence = clementine.playlists.sequence()
  ... sequence.SetShuffleMode(clementine.PlaylistSequence.Shuffle_All)
  ... sequence.SetRepeatMode(clementine.PlaylistSequence.Repeat_Playlist)

The constants defined in this class are:

  - C{Repeat_Off}
  - C{Repeat_Track} - repeats the current track
  - C{Repeat_Album} - repeats the current album
  - C{Repeat_Playlist} - repeats the whole playlist
  - C{Shuffle_Off}
  - C{Shuffle_All} - plays tracks in a random order
  - C{Shuffle_Album} - plays tracks from the current album in a random order

@group Signals: RepeatModeChanged, ShuffleModeChanged
@group Slots: SetRepeatMode, SetShuffleMode, SetUsingDynamicPlaylist
%End

public:
  enum RepeatMode {
    Repeat_Off = 0,
    Repeat_Track = 1,
    Repeat_Album = 2,
    Repeat_Playlist = 3,
  };
  enum ShuffleMode {
    Shuffle_Off = 0,
    Shuffle_All = 1,
    Shuffle_Album = 2,
  };

  RepeatMode repeat_mode() const;
%Docstring
repeat_mode() -> int
Returns the current repeat mode.

@return: one of the C{Repeat_} constants.
%End

  ShuffleMode shuffle_mode() const;
%Docstring
shuffle_mode() -> int
Returns the current shuffle mode.

@return: one of the C{Shuffle_} constants.
%End

  QMenu* repeat_menu() const;
%Docstring
repeat_menu() -> L{PyQt4.QtGui.QMenu}
Returns a menu that can be added to a button or another menu to allow the user
to control the repeat behaviour.
%End

  QMenu* shuffle_menu() const;
%Docstring
shuffle_menu() -> L{PyQt4.QtGui.QMenu}
Returns a menu that can be added to a button or another menu to allow the user
to control the shuffle behaviour.
%End

public slots:
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
%Docstring
SetRepeatMode(mode)
Changes the repeat mode.

@param mode: one of the C{Repeat_} constants.
%End

  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);
%Docstring
SetShuffleMode(mode)
Changes the shuffle mode.

@param mode: one of the C{Shuffle_} constants.
%End

  void SetUsingDynamicPlaylist(bool dynamic);
%Docstring
SetUsingDynamicPlaylist(dynamic)
When using dynamic playlists changing the shuffle and repeat modes is not
allowed, their buttons in the interface are disabled, and the L{repeat_mode()}
and L{shuffle_mode()} functions always return C{Repeat_Off} and C{Shuffle_Off}.

@type dynamic: bool
%End

signals:
  void RepeatModeChanged(PlaylistSequence::RepeatMode mode);
%Docstring
RepeatModeChanged(new_mode)
Emitted when the repeat mode is changed, either by the user or by a call to
L{SetRepeatMode()}.
%End

  void ShuffleModeChanged(PlaylistSequence::ShuffleMode mode);
%Docstring
ShuffleModeChanged(new_mode)
Emitted when the shuffle mode is changed, either by the user or by a call to
L{SetShuffleMode()}.
%End

private:
  PlaylistSequence();
};
