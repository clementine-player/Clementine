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
#include <QFuture>
#include <QFutureWatcher>

#include "engine_fwd.h"
#include "playlistitem.h"
#include "song.h"

class Playlist;
class Settings;
class LastFMService;

class Player : public QObject {
  Q_OBJECT

 public:
  Player(Playlist* playlist, LastFMService* lastfm, QObject* parent = 0);

  void Init();

  EngineBase* GetEngine() { return engine_; }
  Engine::State GetState() const;
  int GetVolume() const;

  PlaylistItem::Options GetCurrentItemOptions() const { return current_item_options_; }
  Song GetCurrentItem() const { return current_item_; }

 public slots:
  void ReloadSettings();

  void PlayAt(int index);
  void PlayPause();
  void Next();
  void NextItem();
  void Previous();
  void Stop();
  void SetVolume(int value);
  void Seek(int seconds);

  void TrackEnded();
  void StreamReady(const QUrl& original_url, const QUrl& media_url);
  void CurrentMetadataChanged(const Song& metadata);

 signals:
  void InitFinished();

  void Playing();
  void Paused();
  void Stopped();
  void VolumeChanged(int volume);
  void Error(const QString& message);

 private slots:
  void EngineInitFinished();
  void EngineStateChanged(Engine::State);
  void EngineMetadataReceived(const Engine::SimpleMetaBundle& bundle);

 private:
  Playlist* playlist_;
  LastFMService* lastfm_;
  QSettings settings_;

  PlaylistItem::Options current_item_options_;
  Song current_item_;

  EngineBase* engine_;
  QFuture<bool> init_engine_;
  QFutureWatcher<bool>* init_engine_watcher_;
};

#endif // PLAYER_H
