/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include <boost/scoped_ptr.hpp>

#include "config.h"
#include "core/albumcoverloader.h"
#include "core/song.h"
#include "engines/engine_fwd.h"
#include "playlist/playlistitem.h"

class PlaylistManager;
class Settings;
class MainWindow;

#ifdef HAVE_LIBLASTFM
  class LastFMService;
#endif

class Player : public QObject {
  Q_OBJECT

 public:
  Player(PlaylistManager* playlists,
#ifdef HAVE_LIBLASTFM
         LastFMService* lastfm,
#endif
         QObject* parent = 0);
  ~Player();

  void Init();

  EngineBase* engine() const { return engine_.get(); }
  Engine::State GetState() const { return last_state_; }
  int GetVolume() const;

  PlaylistItemPtr GetCurrentItem() const { return current_item_; }
  PlaylistItemPtr GetItemAt(int pos) const;
  PlaylistManager* playlists() const { return playlists_; }

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
  void VolumeUp() { SetVolume(GetVolume() + 5); }
  void VolumeDown() { SetVolume(GetVolume() - 5); }
  void Seek(int seconds);
  void SeekForward() { Seek(+5); }
  void SeekBackward() { Seek(-5); }

  void HandleSpecialLoad(const PlaylistItem::SpecialLoadResult& result);
  void CurrentMetadataChanged(const Song& metadata);

  void Mute();
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
  void TrackSkipped(PlaylistItemPtr old_track);

  void ForceShowOSD(Song);

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
  PlaylistManager* playlists_;
#ifdef HAVE_LIBLASTFM
  LastFMService* lastfm_;
#endif
  QSettings settings_;

  PlaylistItemPtr current_item_;

  boost::scoped_ptr<EngineBase> engine_;
  Engine::TrackChangeType stream_change_type_;
  Engine::State last_state_;

  QUrl loading_async_;

  int volume_before_mute_;
};

#endif // PLAYER_H
