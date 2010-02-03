#include "player.h"
#include "playlist.h"
#include "xine-engine.h"
#include "lastfmservice.h"

#include <QtDebug>
#include <QtConcurrentRun>

#include <boost/bind.hpp>

Player::Player(Playlist* playlist, LastFMService* lastfm, QObject* parent)
  : QObject(parent),
    playlist_(playlist),
    lastfm_(lastfm),
    current_item_options_(PlaylistItem::Default),
    engine_(new XineEngine),
    init_engine_watcher_(new QFutureWatcher<bool>(this))
{
  settings_.beginGroup("Player");

  SetVolume(settings_.value("volume", 50).toInt());

  connect(init_engine_watcher_, SIGNAL(finished()), SLOT(EngineInitFinished()));
  connect(engine_, SIGNAL(error(QString)), SIGNAL(Error(QString)));
}

void Player::Init() {
  init_engine_ = QtConcurrent::run(boost::bind(&EngineBase::init, engine_));
  init_engine_watcher_->setFuture(init_engine_);
}

void Player::EngineInitFinished() {
  if (init_engine_.result() == false) {
    qFatal("Error initialising audio engine");
  }

  connect(engine_, SIGNAL(stateChanged(Engine::State)), SLOT(EngineStateChanged(Engine::State)));
  connect(engine_, SIGNAL(trackEnded()), SLOT(TrackEnded()));

  emit InitFinished();
}

void Player::ReloadSettings() {
  if (!init_engine_.isFinished())
    return;

  engine_->reloadSettings();
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
  if (!init_engine_.isFinished())
    return;

  switch (engine_->state()) {
  case Engine::Paused:
    qDebug() << "Unpausing";
    engine_->unpause();
    break;

  case Engine::Playing:
    // We really shouldn't pause last.fm streams
    if (current_item_options_ & PlaylistItem::PauseDisabled)
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
  if (!init_engine_.isFinished())
    return;

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
  if (!init_engine_.isFinished())
    return;

  playlist_->set_current_index(index);

  PlaylistItem* item = playlist_->item_at(index);
  current_item_options_ = item->options();
  current_item_ = item->Metadata();

  if (item->options() & PlaylistItem::SpecialPlayBehaviour)
    item->StartLoading();
  else {
    engine_->play(item->Url());

    if (lastfm_->IsScrobblingEnabled())
      lastfm_->NowPlaying(item->Metadata());
  }
}

void Player::StreamReady(const QUrl& original_url, const QUrl& media_url) {
  if (!init_engine_.isFinished())
    return;

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
  if (!init_engine_.isFinished())
    return;

  engine_->seek(seconds * 1000);
}
