#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QSettings>

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

  EngineBase* GetEngine() { return engine_; }
  Engine::State GetState() const;
  int GetVolume() const;

  PlaylistItem::Options GetCurrentItemOptions() const { return current_item_options_; }
  Song GetCurrentItem() const { return current_item_; }

 public slots:
  void PlayAt(int index);
  void PlayPause();
  void Next();
  void Previous();
  void Stop();
  void SetVolume(int value);
  void Seek(int seconds);

  void TrackEnded();
  void StreamReady(const QUrl& original_url, const QUrl& media_url);

 signals:
  void Playing();
  void Paused();
  void Stopped();
  void VolumeChanged(int volume);
  void Error(const QString& message);

 private slots:
  void EngineStateChanged(Engine::State);

 private:
  Playlist* playlist_;
  LastFMService* lastfm_;
  QSettings settings_;

  PlaylistItem::Options current_item_options_;
  Song current_item_;

  EngineBase* engine_;
};

#endif // PLAYER_H
