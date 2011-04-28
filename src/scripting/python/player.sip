class PlayerInterface : QObject /PyName=Player/ {

%TypeHeaderCode
#include "core/player.h"
%End

public:
  Engine::State GetState() const;
  int GetVolume() const;

  PlaylistItemPtr GetCurrentItem() const;
  PlaylistItemPtr GetItemAt(int pos) const;
  PlaylistManagerInterface* playlists() const;

  void RegisterUrlHandler(UrlHandler* handler);
  void UnregisterUrlHandler(UrlHandler* handler);

public slots:
  // Manual track change to the specified track
  void PlayAt(int i, Engine::TrackChangeType change, bool reshuffle);

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

  void SeekTo(int seconds);
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

  // Disabled for now because it causes:
  // TypeError: C++ type 'PlaylistItemPtr' is not supported as a native Qt signal type
  // void TrackSkipped(PlaylistItemPtr old_track);

  void Seeked(qlonglong microseconds);

  // Emitted when Player has processed a request to play another song. This contains
  // the URL of the song and a flag saying whether it was able to play the song.
  void SongChangeRequestProcessed(const QUrl& url, bool valid);

  void ForceShowOSD(Song);

private:
  PlayerInterface();
};
