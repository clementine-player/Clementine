/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QSettings>

#include "core/song.h"
#include "engines/engine_fwd.h"
#include "playlist/playlistitem.h"

class PlaylistManager;
class Settings;
class LastFMService;

struct DBusStatus {    // From Amarok.
  int play;            // Playing = 0, Paused = 1, Stopped = 2
  int random;          // Linearly = 0, Randomly = 1
  int repeat;          // Go_To_Next = 0, Repeat_Current = 1
  int repeat_playlist; // Stop_When_Finished = 0, Never_Give_Up_Playing = 1, Never_Let_You_Down = 42

  enum MprisPlayState {
    Mpris_Playing = 0,
    Mpris_Paused = 1,
    Mpris_Stopped = 2,
  };
};
Q_DECLARE_METATYPE(DBusStatus);

#ifdef Q_WS_X11
#include <QDBusArgument>
QDBusArgument& operator<< (QDBusArgument& arg, const DBusStatus& status);
const QDBusArgument& operator>> (const QDBusArgument& arg, DBusStatus& status);
#endif

class Player : public QObject {
  Q_OBJECT

 public:
  Player(PlaylistManager* playlists, LastFMService* lastfm, Engine::Type engine,
         QObject* parent = 0);

  EngineBase* createEngine(Engine::Type engine);
  void Init();

  EngineBase* GetEngine() { return engine_; }
  Engine::State GetState() const;
  int GetVolume() const;

  boost::shared_ptr<PlaylistItem> GetCurrentItem() const { return current_item_; }

  // MPRIS
  enum DBusCaps {
    NONE                 = 0,
    CAN_GO_NEXT          = 1 << 0,
    CAN_GO_PREV          = 1 << 1,
    CAN_PAUSE            = 1 << 2,
    CAN_PLAY             = 1 << 3,
    CAN_SEEK             = 1 << 4,
    CAN_PROVIDE_METADATA = 1 << 5,
    CAN_HAS_TRACKLIST    = 1 << 6,
  };

 public slots:
  void ReloadSettings();

  // Manual track change to the specified track
  void PlayAt(int i, Engine::TrackChangeType change, bool reshuffle);

  // If there's currently a song playing, pause it, otherwise play the track
  // that was playing last, or the first one on the playlist
  void PlayPause();

  // Skips this track.  Might load more of the current radio station.
  void Next();

  void Previous();
  void SetVolume(int value);
  void Seek(int seconds);
  void SeekForward() { Seek(+5); }
  void SeekBackward() { Seek(-5); }

  void HandleSpecialLoad(const PlaylistItem::SpecialLoadResult& result);
  void CurrentMetadataChanged(const Song& metadata);

  void PlaylistChanged();

  // MPRIS /Player
  int GetCaps() const;
  DBusStatus GetStatus() const;
  QVariantMap GetMetadata() const;
  void Mute();
  void Pause();
  void Stop();
  void Play();
  void Prev();
  int PositionGet() const;
  void PositionSet(int);
  void Repeat(bool);
  void ShowOSD();
  void VolumeDown(int);
  void VolumeUp(int);
  void VolumeDown() { VolumeDown(4); }
  void VolumeUp() { VolumeUp(4); }
  int VolumeGet() const;
  void VolumeSet(int);

  // MPRIS /Tracklist
  int AddTrack(const QString&, bool);
  void DelTrack(int index);
  int GetCurrentTrack() const;
  int GetLength() const;
  QVariantMap GetMetadata(int) const;
  void SetLoop(bool enable);
  void SetRandom(bool enable);

  // Amarok extension.
  void PlayTrack(int index);

 signals:
  void Playing();
  void Paused();
  void Stopped();
  void PlaylistFinished();
  void VolumeChanged(int volume);
  void Error(const QString& message);

  void ForceShowOSD(Song);

  // MPRIS
  // Player
  void CapsChange(int);
  void TrackChange(QVariantMap);
  void StatusChange(DBusStatus);
  // TrackList
  void TrackListChange(int i);

 private slots:
  void EngineStateChanged(Engine::State);
  void EngineMetadataReceived(const Engine::SimpleMetaBundle& bundle);
  void TrackAboutToEnd();
  void TrackEnded();

  // Play the next item on the playlist - disregarding radio stations like
  // last.fm that might have more tracks.
  void NextItem(Engine::TrackChangeType change);

  void NextInternal(Engine::TrackChangeType);

 private:
  QVariantMap GetMetadata(const PlaylistItem& item) const;

  PlaylistManager* playlists_;
  LastFMService* lastfm_;
  QSettings settings_;

  boost::shared_ptr<PlaylistItem> current_item_;

  EngineBase* engine_;
  Engine::TrackChangeType stream_change_type_;

  QUrl loading_async_;
};

#endif // PLAYER_H
