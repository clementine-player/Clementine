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

#include "engines/engine_fwd.h"
#include "playlistitem.h"
#include "song.h"

class Playlist;
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
  Player(Playlist* playlist, LastFMService* lastfm, Engine::Type engine, QObject* parent = 0);

  EngineBase* createEngine(Engine::Type engine);
  void Init();

  EngineBase* GetEngine() { return engine_; }
  Engine::State GetState() const;
  int GetVolume() const;

  PlaylistItem::Options GetCurrentItemOptions() const { return current_item_options_; }
  Song GetCurrentItem() const { return current_item_; }

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

  void PlayAt(int index, Engine::TrackChangeType change);
  void PlayPause();
  void NextItem(Engine::TrackChangeType change = Engine::Auto);
  void Previous();
  void SetVolume(int value);
  void Seek(int seconds);

  void TrackEnded();
  void StreamReady(const QUrl& original_url, const QUrl& media_url);
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
  void Next(Engine::TrackChangeType change = Engine::Manual);
  void Prev();
  int PositionGet() const;
  void PositionSet(int);
  void Repeat(bool);
  void ShowOSD();
  void VolumeDown(int);
  void VolumeUp(int);
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
  void NextAuto();

 private:
  QVariantMap GetMetadata(const PlaylistItem& item) const;

  Playlist* playlist_;
  LastFMService* lastfm_;
  QSettings settings_;

  PlaylistItem::Options current_item_options_;
  Song current_item_;

  EngineBase* engine_;
  Engine::TrackChangeType stream_change_type_;
};

#endif // PLAYER_H
