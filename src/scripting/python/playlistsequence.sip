// The QWidget base class is deliberately hidden
struct PlaylistSequence {

%TypeHeaderCode
#include "playlist/playlistsequence.h"
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

  static const char* kSettingsGroup;

  RepeatMode repeat_mode() const;
  ShuffleMode shuffle_mode() const;

  QMenu* repeat_menu() const;
  QMenu* shuffle_menu() const;

public slots:
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);
  void SetUsingDynamicPlaylist(bool dynamic);

signals:
  void RepeatModeChanged(PlaylistSequence::RepeatMode mode);
  void ShuffleModeChanged(PlaylistSequence::ShuffleMode mode);

private:
  PlaylistSequence();
};
