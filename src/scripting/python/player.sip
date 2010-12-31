class Player : QObject {

%TypeHeaderCode
#include "core/player.h"
%End

public:
  // Engine::State GetState() const;
  int GetVolume() const;

  //PlaylistItemPtr GetCurrentItem() const { return current_item_; }
  //PlaylistItemPtr GetItemAt(int pos) const;
  //PlaylistManager* playlists() const { return playlists_; }

public slots:
  // Manual track change to the specified track
  //void PlayAt(int i, Engine::TrackChangeType change, bool reshuffle);

  // If there's currently a song playing, pause it, otherwise play the track
  // that was playing last, or the first one on the playlist
  void PlayPause();

  // Skips this track.  Might load more of the current radio station.
  void Next();
  void Previous();

  void SetVolume(int value);
  void VolumeUp();
  void VolumeDown();
  void Mute();

  void Seek(int seconds);
  void SeekForward();
  void SeekBackward();

  void Pause();
  void Stop();
  void Play();
  void ShowOSD();

signals:
  void Playing();
  void Paused();
  void Stopped();
  void PlaylistFinished();
  void VolumeChanged(int volume);
  void Error(const QString& message);
  //void TrackSkipped(PlaylistItemPtr old_track);

  //void ForceShowOSD(Song);

private:
  Player();
};
