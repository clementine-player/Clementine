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

#include "config.h"
#include "player.h"
#include "engines/enginebase.h"
#include "engines/gstengine.h"
#include "library/librarybackend.h"
#include "playlist/playlist.h"
#include "playlist/playlistitem.h"
#include "playlist/playlistmanager.h"
#include "mpris_common.h"

#ifdef HAVE_LIBLASTFM
#  include "radio/lastfmservice.h"
#endif

#ifdef HAVE_DBUS
#  include "mpris.h"
#  include "mpris2.h"
#  include <QDBusConnection>
#  include <QImage>
#endif

#include <QtDebug>
#include <QtConcurrentRun>

#include <boost/bind.hpp>

using boost::shared_ptr;


Player::Player(MainWindow* main_window, PlaylistManager* playlists,
#ifdef HAVE_LIBLASTFM
               LastFMService* lastfm,
#endif
               QObject* parent)
  : QObject(parent),
    art_loader_(new mpris::ArtLoader(this)),
    mpris1_(NULL),
    mpris2_(NULL),
    playlists_(playlists),
#ifdef HAVE_LIBLASTFM
    lastfm_(lastfm),
#endif
    engine_(new GstEngine),
    stream_change_type_(Engine::First),
    last_state_(Engine::Empty),
    volume_before_mute_(50)
{
  // Loads album art and saves it to a file in /tmp for MPRIS clients and those
  // objects which need cover art's filepath, not the image itself
  connect(playlists, SIGNAL(CurrentSongChanged(Song)),
          art_loader_, SLOT(LoadArt(Song)));

#ifdef HAVE_DBUS
  // MPRIS DBus interface.
  qDBusRegisterMetaType<QImage>();
  qDBusRegisterMetaType<TrackMetadata>();
  qDBusRegisterMetaType<TrackIds>();

  //MPRIS 1.0 implementation
  mpris1_ = new mpris::Mpris1(this, art_loader_, this);

  //MPRIS 2.0 implementation
  mpris2_ = new mpris::Mpris2(main_window, this, art_loader_, mpris1_, this);
#endif

  settings_.beginGroup("Player");

  SetVolume(settings_.value("volume", 50).toInt());

  connect(engine_.get(), SIGNAL(Error(QString)), SIGNAL(Error(QString)));
}

Player::~Player() {
}

void Player::Init() {
  if (!engine_->Init())
    qFatal("Error initialising audio engine");

  connect(engine_.get(), SIGNAL(StateChanged(Engine::State)), SLOT(EngineStateChanged(Engine::State)));
  connect(engine_.get(), SIGNAL(TrackAboutToEnd()), SLOT(TrackAboutToEnd()));
  connect(engine_.get(), SIGNAL(TrackEnded()), SLOT(TrackEnded()));
  connect(engine_.get(), SIGNAL(MetaData(Engine::SimpleMetaBundle)),
                   SLOT(EngineMetadataReceived(Engine::SimpleMetaBundle)));

  engine_->SetVolume(settings_.value("volume", 50).toInt());
}

void Player::ReloadSettings() {
  engine_->ReloadSettings();
}

void Player::HandleSpecialLoad(const PlaylistItem::SpecialLoadResult &result) {
  switch (result.type_) {
  case PlaylistItem::SpecialLoadResult::NoMoreTracks:
    loading_async_ = QUrl();
    NextItem(Engine::Auto);
    break;

  case PlaylistItem::SpecialLoadResult::TrackAvailable: {
    // Might've been an async load, so check we're still on the same item
    int current_index = playlists_->active()->current_row();
    if (current_index == -1)
      return;

    shared_ptr<PlaylistItem> item = playlists_->active()->item_at(current_index);
    if (!item || item->Url() != result.original_url_)
      return;

    engine_->Play(result.media_url_, stream_change_type_,
                  item->Metadata().beginning() * 1000,
                  item->Metadata().end() * 1000);

    current_item_ = item;
    loading_async_ = QUrl();
    break;
  }

  case PlaylistItem::SpecialLoadResult::WillLoadAsynchronously:
    // We'll get called again later with either NoMoreTracks or TrackAvailable
    loading_async_ = result.original_url_;
    break;
  }
}

void Player::Next() {
  NextInternal(Engine::Manual);
}

void Player::NextInternal(Engine::TrackChangeType change) {
  if (playlists_->active()->stop_after_current()) {
    playlists_->active()->StopAfter(-1);
    Stop();
    return;
  }

  if (playlists_->active()->current_item() &&
      playlists_->active()->current_item()->options() & PlaylistItem::ContainsMultipleTracks) {
    // The next track is already being loaded
    if (playlists_->active()->current_item()->Url() == loading_async_)
      return;

    stream_change_type_ = change;
    HandleSpecialLoad(playlists_->active()->current_item()->LoadNext());
    return;
  }

  NextItem(change);
}

void Player::NextItem(Engine::TrackChangeType change) {
  int i = playlists_->active()->next_row();
  if (i == -1) {
    playlists_->active()->set_current_row(i);
    emit PlaylistFinished();
    Stop();
    return;
  }

  PlayAt(i, change, false);
}

void Player::TrackEnded() {
  if (playlists_->active()->stop_after_current()) {
    playlists_->active()->StopAfter(-1);
    Stop();
    return;
  }

  if (current_item_ && current_item_->IsLocalLibraryItem() &&
      !playlists_->active()->has_scrobbled()) {
    // The track finished before its scrobble point (30 seconds), so increment
    // the play count now.
    playlists_->library_backend()->IncrementPlayCountAsync(
        current_item_->Metadata().id());
  }

  NextInternal(Engine::Auto);
}

void Player::PlayPause() {
  switch (engine_->state()) {
  case Engine::Paused:
    engine_->Unpause();
    break;

  case Engine::Playing: {
    // We really shouldn't pause last.fm streams
    // Stopping seems like a reasonable thing to do (especially on mac where there
    // is no media key for stop).
    if (current_item_->options() & PlaylistItem::PauseDisabled) {
      engine_->Stop();
    } else {
      engine_->Pause();
    }
    break;
  }

  case Engine::Empty:
  case Engine::Idle: {
    playlists_->SetActivePlaylist(playlists_->current_id());
    if (playlists_->active()->rowCount() == 0)
      break;

             int i = playlists_->active()->current_row();
    if (i == -1) i = playlists_->active()->last_played_row();
    if (i == -1) i = 0;

    PlayAt(i, Engine::First, true);
    break;
  }
  }
}

void Player::Stop() {
  engine_->Stop();
  playlists_->active()->set_current_row(-1);
  current_item_.reset();
}

void Player::Previous() {
  int i = playlists_->active()->previous_row();
  playlists_->active()->set_current_row(i);
  if (i == -1) {
    Stop();
    return;
  }

  PlayAt(i, Engine::Manual, false);
}

void Player::EngineStateChanged(Engine::State state) {
  switch (state) {
    case Engine::Paused: emit Paused(); break;
    case Engine::Playing: emit Playing(); break;
    case Engine::Empty:
    case Engine::Idle: emit Stopped(); break;
  }
  last_state_ = state;
}

void Player::SetVolume(int value) {
  int old_volume = engine_->volume();

  int volume = qBound(0, value, 100);
  settings_.setValue("volume", volume);
  engine_->SetVolume(volume);

  if (volume != old_volume){
    emit VolumeChanged(volume);
  }

}

int Player::GetVolume() const {
  return engine_->volume();
}

void Player::PlayAt(int index, Engine::TrackChangeType change, bool reshuffle) {
  if (change == Engine::Manual && engine_->position() != engine_->length()) {
    emit TrackSkipped(current_item_);
  }

  if (reshuffle)
    playlists_->active()->set_current_row(-1);
  playlists_->active()->set_current_row(index);

  current_item_ = playlists_->active()->current_item();

  if (current_item_->options() & PlaylistItem::SpecialPlayBehaviour) {
    // It's already loading
    if (current_item_->Url() == loading_async_)
      return;

    stream_change_type_ = change;
    HandleSpecialLoad(current_item_->StartLoading());
  }
  else {
    loading_async_ = QUrl();
    engine_->Play(current_item_->Url(), change,
                  current_item_->Metadata().beginning() * 1000,
                  current_item_->Metadata().end() * 1000);

#ifdef HAVE_LIBLASTFM
    if (lastfm_->IsScrobblingEnabled())
      lastfm_->NowPlaying(current_item_->Metadata());
#endif
  }
}

void Player::CurrentMetadataChanged(const Song& metadata) {
#ifdef HAVE_LIBLASTFM
  lastfm_->NowPlaying(metadata);
#endif
}

void Player::Seek(int seconds) {
  int msec = qBound(0, seconds * 1000, int(engine_->length()));
  engine_->Seek(msec);

  // If we seek the track we don't want to submit it to last.fm
  playlists_->active()->set_scrobbled(true);
}

void Player::EngineMetadataReceived(const Engine::SimpleMetaBundle& bundle) {
  PlaylistItemPtr item = playlists_->active()->current_item();
  if (!item)
    return;

  Engine::SimpleMetaBundle bundle_copy = bundle;

  // Maybe the metadata is from icycast and has "Artist - Title" shoved
  // together in the title field.
  int dash_pos = bundle_copy.title.indexOf('-');
  if (dash_pos != -1 && bundle_copy.artist.isEmpty()) {
    bundle_copy.artist = bundle_copy.title.mid(dash_pos + 1).trimmed();
    bundle_copy.title = bundle_copy.title.left(dash_pos).trimmed();
  }

  // Hack as SomaFM's and icecast's artist/title descriptions are backwards.
  if (item->Url().host().contains("somafm.com") ||
      item->Url().fragment() == "icecast") {
    qSwap(bundle_copy.artist, bundle_copy.title);
  }

  Song song = item->Metadata();
  song.MergeFromSimpleMetaBundle(bundle_copy);

  // Ignore useless metadata
  if (song.title().isEmpty() && song.artist().isEmpty())
    return;

  playlists_->active()->SetStreamMetadata(item->Url(), song);
}

PlaylistItemPtr Player::GetItemAt(int pos) const {
  if (pos < 0 || pos >= playlists_->active()->rowCount())
    return PlaylistItemPtr();
  return playlists_->active()->item_at(pos);
}

void Player::Mute() {
  const int current_volume = engine_->volume();

  if (current_volume == 0) {
    SetVolume(volume_before_mute_);
  } else {
    volume_before_mute_ = current_volume;
    SetVolume(0);
  }
}

void Player::Pause() {
  switch (GetState()) {
    case Engine::Playing:
      engine_->Pause();
      break;
    case Engine::Paused:
      engine_->Unpause();
      break;
    default:
      return;
  }
}

void Player::Play() {
  switch (GetState()) {
    case Engine::Playing:
      Seek(0);
      break;
    case Engine::Paused:
      engine_->Unpause();
      break;
    default:
      PlayPause();
      break;
  }
}

void Player::ShowOSD() {
  if (current_item_)
    emit ForceShowOSD(current_item_->Metadata());
}

void Player::TrackAboutToEnd() {
  if (engine_->is_autocrossfade_enabled()) {
    // Crossfade is on, so just start playing the next track.  The current one
    // will fade out, and the new one will fade in

    // But, if there's no next track and we don't want to fade out, then do
    // nothing and just let the track finish to completion.
    if (!engine_->is_fadeout_enabled() &&
        playlists_->active()->next_row() == -1)
      return;

    TrackEnded();
  } else {
    // Crossfade is off, so start preloading the next track so we don't get a
    // gap between songs.
    if (current_item_->options() & PlaylistItem::ContainsMultipleTracks)
      return;
    if (playlists_->active()->next_row() == -1)
      return;

    shared_ptr<PlaylistItem> item = playlists_->active()->item_at(
        playlists_->active()->next_row());
    if (!item)
      return;

    QUrl url = item->Url();

    // Get the actual track URL rather than the stream URL.
    if (item->options() & PlaylistItem::ContainsMultipleTracks) {
      PlaylistItem::SpecialLoadResult result = item->LoadNext();
      switch (result.type_) {
      case PlaylistItem::SpecialLoadResult::NoMoreTracks:
        return;

      case PlaylistItem::SpecialLoadResult::WillLoadAsynchronously:
        loading_async_ = item->Url();
        return;

      case PlaylistItem::SpecialLoadResult::TrackAvailable:
        url = result.media_url_;
        break;
      }
    }
    engine_->StartPreloading(url);
  }
}
