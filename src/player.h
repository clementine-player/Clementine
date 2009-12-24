#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QSettings>

#include "engine_fwd.h"

class Playlist;
class Settings;

class Player : public QObject {
  Q_OBJECT

 public:
  Player(Playlist* playlist, QObject* parent = 0);

  EngineBase* GetEngine() { return engine_; }
  Engine::State GetState() const;
  int GetVolume() const;

 public slots:
  void PlayAt(int index);
  void PlayPause();
  void Next();
  void Previous();
  void Stop();
  void SetVolume(int value);

 signals:
  void Playing();
  void Paused();
  void Stopped();
  void Error(const QString& message);

 private slots:
  void EngineStateChanged(Engine::State);

 private:
  Playlist* playlist_;
  QSettings settings_;

  EngineBase* engine_;
};

#endif // PLAYER_H
