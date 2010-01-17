#include "player.h"
#include "playlist.h"
#include "xine-engine.h"
#include "lastfmservice.h"

#include <QtDebug>

Player::Player(Playlist* playlist, LastFMService* lastfm, QObject* parent)
  : QObject(parent),
    playlist_(playlist),
    lastfm_(lastfm),
    engine_(new XineEngine)
{
  if (!engine_->init()) {
    qFatal("Couldn't load engine");
  }

  settings_.beginGroup("Player");
  SetVolume(settings_.value("volume", 50).toInt());

  connect(engine_, SIGNAL(stateChanged(Engine::State)), SLOT(EngineStateChanged(Engine::State)));
  connect(engine_, SIGNAL(trackEnded()), SLOT(TrackEnded()));
}

void Player::Next() {
  if (playlist_->current_item_options() & PlaylistItem::ContainsMultipleTracks) {
    playlist_->current_item()->LoadNext();
    return;
  }

  int i = playlist_->next_index();
  playlist_->set_current_index(i);
  if (i == -1) {
    Stop();
    return;
  }

  PlayAt(i);
}

void Player::TrackEnded() {
  int i = playlist_->current_index();
  if (i == -1 || playlist_->stop_after_current()) {
    Stop();
    return;
  }

  Next();
}

void Player::PlayPause() {
  switch (engine_->state()) {
  case Engine::Paused:
    qDebug() << "Unpausing";
    engine_->unpause();
    break;

  case Engine::Playing:
    // We really shouldn't pause last.fm streams
    if (playlist_->current_item()->options() & PlaylistItem::PauseDisabled)
      break;

    qDebug() << "Pausing";
    engine_->pause();
    break;

  case Engine::Empty:
  case Engine::Idle: {
    int i = playlist_->current_index();
    if (i == -1) {
      if (playlist_->rowCount() == 0)
        break;
      i = 0;
    }
    PlayAt(i);
    break;
  }
  }
}

void Player::Stop() {
  qDebug() << "Stopping";
  engine_->stop();
  playlist_->set_current_index(-1);
}

void Player::Previous() {
  int i = playlist_->previous_index();
  playlist_->set_current_index(i);
  if (i == -1) {
    Stop();
    return;
  }

  PlayAt(i);
}

void Player::EngineStateChanged(Engine::State state) {
  switch (state) {
    case Engine::Paused: emit Paused(); break;
    case Engine::Playing: emit Playing(); break;
    case Engine::Empty:
    case Engine::Idle: emit Stopped(); break;
  }
}

void Player::SetVolume(int value) {
  settings_.setValue("volume", value);
  engine_->setVolume(value);
  emit VolumeChanged(value);
}

int Player::GetVolume() const {
  return engine_->volume();
}

Engine::State Player::GetState() const {
  return engine_->state();
}

void Player::PlayAt(int index) {
  playlist_->set_current_index(index);

  PlaylistItem* item = playlist_->item_at(index);

  if (item->options() & PlaylistItem::SpecialPlayBehaviour)
    item->StartLoading();
  else {
    engine_->play(item->Url());

    if (lastfm_->IsScrobblingEnabled())
      lastfm_->NowPlaying(item->Metadata());
  }
}

void Player::StreamReady(const QUrl& original_url, const QUrl& media_url) {
  int current_index = playlist_->current_index();
  if (current_index == -1)
    return;

  PlaylistItem* item = playlist_->item_at(current_index);
  if (!item || item->Url() != original_url)
    return;

  engine_->play(media_url);

  lastfm_->NowPlaying(item->Metadata());
}

void Player::Seek(int seconds) {
  engine_->seek(seconds * 1000);
}
