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
#include "core/logging.h"
#include "core/urlhandler.h"
#include "engines/enginebase.h"
#include "engines/gstengine.h"
#include "library/librarybackend.h"
#include "playlist/playlist.h"
#include "playlist/playlistitem.h"
#include "playlist/playlistmanager.h"

#ifdef HAVE_LIBLASTFM
#  include "internet/lastfmservice.h"
#endif

#include <QtDebug>
#include <QtConcurrentRun>

#include <boost/bind.hpp>

using boost::shared_ptr;


Player::Player(PlaylistManagerInterface* playlists, QObject* parent)
  : PlayerInterface(parent),
    playlists_(playlists),
    lastfm_(NULL),
    engine_(new GstEngine),
    stream_change_type_(Engine::First),
    last_state_(Engine::Empty),
    volume_before_mute_(50)
{
  settings_.beginGroup("Player");

  SetVolume(settings_.value("volume", 50).toInt());

  connect(engine_.get(), SIGNAL(Error(QString)), SIGNAL(Error(QString)));

  connect(engine_.get(), SIGNAL(ValidSongRequested(QUrl)), SLOT(ValidSongRequested(QUrl)));
  connect(engine_.get(), SIGNAL(InvalidSongRequested(QUrl)), SLOT(InvalidSongRequested(QUrl)));
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

#ifdef HAVE_LIBLASTFM
  lastfm_ = InternetModel::Service<LastFMService>();
#endif
}

void Player::ReloadSettings() {
  engine_->ReloadSettings();
}

void Player::HandleLoadResult(const UrlHandler::LoadResult& result) {
  switch (result.type_) {
  case UrlHandler::LoadResult::NoMoreTracks:
    qLog(Debug) << "URL handler for" << result.original_url_
                << "said no more tracks";

    loading_async_ = QUrl();
    NextItem(Engine::Auto);
    break;

  case UrlHandler::LoadResult::TrackAvailable: {
    // Might've been an async load, so check we're still on the same item
    int current_index = playlists_->active()->current_row();
    if (current_index == -1)
      return;

    shared_ptr<PlaylistItem> item = playlists_->active()->item_at(current_index);
    if (!item || item->Url() != result.original_url_)
      return;

    qLog(Debug) << "URL handler for" << result.original_url_
                << "returned" << result.media_url_;

    engine_->Play(result.media_url_, stream_change_type_,
                  item->Metadata().has_cue(),
                  item->Metadata().beginning_nanosec(),
                  item->Metadata().end_nanosec());

    current_item_ = item;
    loading_async_ = QUrl();
    break;
  }

  case UrlHandler::LoadResult::WillLoadAsynchronously:
    qLog(Debug) << "URL handler for" << result.original_url_
                << "is loading asynchronously";

    // We'll get called again later with either NoMoreTracks or TrackAvailable
    loading_async_ = result.original_url_;
    break;
  }
}

void Player::Next() {
  NextInternal(Engine::Manual);
}

void Player::NextInternal(Engine::TrackChangeFlags change) {
  if (playlists_->active()->stop_after_current()) {
    playlists_->active()->StopAfter(-1);
    Stop();
    return;
  }

  if (playlists_->active()->current_item()) {
    const QUrl url = playlists_->active()->current_item()->Url();

    if (url_handlers_.contains(url.scheme())) {
      // The next track is already being loaded
      if (url == loading_async_)
        return;

      stream_change_type_ = change;
      HandleLoadResult(url_handlers_[url.scheme()]->LoadNext(url));
      return;
    }
  }

  NextItem(change);
}

void Player::NextItem(Engine::TrackChangeFlags change) {
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
      current_item_->Metadata().id() != -1 &&
      !playlists_->active()->have_incremented_playcount() &&
      playlists_->active()->get_lastfm_status() != Playlist::LastFM_Seeked) {
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
      Stop();
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

void Player::PlayAt(int index, Engine::TrackChangeFlags change, bool reshuffle) {
  if (change == Engine::Manual && engine_->position_nanosec() != engine_->length_nanosec()) {
    emit TrackSkipped(current_item_);
  }

  if (current_item_ && current_item_->Metadata().IsOnSameAlbum(
        playlists_->active()->item_at(index)->Metadata())) {
    change |= Engine::SameAlbum;
  }

  if (reshuffle)
    playlists_->active()->set_current_row(-1);
  playlists_->active()->set_current_row(index);

  current_item_ = playlists_->active()->current_item();
  const QUrl url = current_item_->Url();

  if (url_handlers_.contains(url.scheme())) {
    // It's already loading
    if (url == loading_async_)
      return;

    stream_change_type_ = change;
    HandleLoadResult(url_handlers_[url.scheme()]->StartLoading(url));
  } else {
    loading_async_ = QUrl();
    engine_->Play(current_item_->Url(), change,
                  current_item_->Metadata().has_cue(),
                  current_item_->Metadata().beginning_nanosec(),
                  current_item_->Metadata().end_nanosec());

#ifdef HAVE_LIBLASTFM
    if (lastfm_->IsScrobblingEnabled())
      lastfm_->NowPlaying(current_item_->Metadata());
#endif
  }
}

void Player::CurrentMetadataChanged(const Song& metadata) {
  // those things might have changed (especially when a previously invalid
  // song was reloaded) so we push the latest version into Engine
  engine_->RefreshMarkers(metadata.beginning_nanosec(), metadata.end_nanosec());

#ifdef HAVE_LIBLASTFM
  lastfm_->NowPlaying(metadata);
#endif
}

void Player::SeekTo(int seconds) {
  qint64 nanosec = qBound(0ll, qint64(seconds) * kNsecPerSec,
                          engine_->length_nanosec());
  engine_->Seek(nanosec);

  // If we seek the track we don't want to submit it to last.fm
  qLog(Info) << "Track seeked to" << nanosec << "ns - not scrobbling";
  if (playlists_->active()->get_lastfm_status() == Playlist::LastFM_New) {
    playlists_->active()->set_lastfm_status(Playlist::LastFM_Seeked);
  }

  emit Seeked(nanosec / 1000);
}

void Player::SeekForward() {
  SeekTo(engine()->position_nanosec() / kNsecPerSec + 10);
}

void Player::SeekBackward() {
  SeekTo(engine()->position_nanosec() / kNsecPerSec - 10);
}

void Player::EngineMetadataReceived(const Engine::SimpleMetaBundle& bundle) {
  PlaylistItemPtr item = playlists_->active()->current_item();
  if (!item)
    return;

  Engine::SimpleMetaBundle bundle_copy = bundle;

  // Maybe the metadata is from icycast and has "Artist - Title" shoved
  // together in the title field.
  const int dash_pos = bundle_copy.title.indexOf('-');
  if (dash_pos != -1 && bundle_copy.artist.isEmpty()) {
    // Split on " - " if it exists, otherwise split on "-".
    const int space_dash_pos = bundle_copy.title.indexOf(" - ");
    if (space_dash_pos != -1) {
      bundle_copy.artist = bundle_copy.title.left(space_dash_pos).trimmed();
      bundle_copy.title  = bundle_copy.title.mid(space_dash_pos + 3).trimmed();
    } else {
      bundle_copy.artist = bundle_copy.title.left(dash_pos).trimmed();
      bundle_copy.title  = bundle_copy.title.mid(dash_pos + 1).trimmed();
    }
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
  engine_->Pause();
}

void Player::Play() {
  switch (GetState()) {
    case Engine::Playing:
      SeekTo(0);
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
    emit ForceShowOSD(current_item_->Metadata(), false);
}

void Player::TogglePrettyOSD() {
  if (current_item_)
    emit ForceShowOSD(current_item_->Metadata(), true);
}

void Player::TrackAboutToEnd() {
  // If the current track was from a URL handler then it might have special
  // behaviour to queue up a subsequent track.  We don't want to preload (and
  // scrobble) the next item in the playlist if it's just going to be stopped
  // again immediately after.
  if (playlists_->active()->current_item()) {
    const QUrl url = playlists_->active()->current_item()->Url();
    if (url_handlers_.contains(url.scheme()))
      return;
  }

  const bool has_next_row = playlists_->active()->next_row() != -1;
  PlaylistItemPtr next_item;

  if (has_next_row) {
    next_item = playlists_->active()->item_at(playlists_->active()->next_row());
  }

  if (engine_->is_autocrossfade_enabled()) {
    // Crossfade is on, so just start playing the next track.  The current one
    // will fade out, and the new one will fade in

    // But, if there's no next track and we don't want to fade out, then do
    // nothing and just let the track finish to completion.
    if (!engine_->is_fadeout_enabled() && !has_next_row)
      return;

    // If the next track is on the same album (or same cue file), and the
    // user doesn't want to crossfade between tracks on the same album, then
    // don't do this automatic crossfading.
    if (engine_->crossfade_same_album() ||
        !has_next_row ||
        !next_item ||
        !current_item_->Metadata().IsOnSameAlbum(next_item->Metadata())) {
      TrackEnded();
      return;
    }
  }

  // Crossfade is off, so start preloading the next track so we don't get a
  // gap between songs.
  if (!has_next_row || !next_item)
    return;

  QUrl url = next_item->Url();

  // Get the actual track URL rather than the stream URL.
  if (url_handlers_.contains(url.scheme())) {
    UrlHandler::LoadResult result = url_handlers_[url.scheme()]->LoadNext(url);
    switch (result.type_) {
    case UrlHandler::LoadResult::NoMoreTracks:
      return;

    case UrlHandler::LoadResult::WillLoadAsynchronously:
      loading_async_ = url;
      return;

    case UrlHandler::LoadResult::TrackAvailable:
      url = result.media_url_;
      break;
    }
  }
  engine_->StartPreloading(url, next_item->Metadata().has_cue(),
                           next_item->Metadata().beginning_nanosec(),
                           next_item->Metadata().end_nanosec());
}

void Player::ValidSongRequested(const QUrl& url) {
  emit SongChangeRequestProcessed(url, true);
}

void Player::InvalidSongRequested(const QUrl& url) {
  // first send the notification to others...
  emit SongChangeRequestProcessed(url, false);
  // ... and now when our listeners have completed their processing of the
  // current item we can change the current item by skipping to the next song
  NextItem(Engine::Auto);
}

void Player::RegisterUrlHandler(UrlHandler* handler) {
  const QString scheme = handler->scheme();

  if (url_handlers_.contains(scheme)) {
    qLog(Warning) << "Tried to register a URL handler for" << scheme
                  << "but one was already registered";
    return;
  }

  qLog(Info) << "Registered URL handler for" << scheme;
  url_handlers_.insert(scheme, handler);
  connect(handler, SIGNAL(destroyed(QObject*)), SLOT(UrlHandlerDestroyed(QObject*)));
  connect(handler, SIGNAL(AsyncLoadComplete(UrlHandler::LoadResult)),
          SLOT(HandleLoadResult(UrlHandler::LoadResult)));
}

void Player::UnregisterUrlHandler(UrlHandler* handler) {
  const QString scheme = url_handlers_.key(handler);
  if (scheme.isEmpty()) {
    qLog(Warning) << "Tried to unregister a URL handler for" << handler->scheme()
                  << "that wasn't registered";
    return;
  }

  qLog(Info) << "Unregistered URL handler for" << scheme;
  url_handlers_.remove(scheme);
  disconnect(handler, SIGNAL(destroyed(QObject*)), this, SLOT(UrlHandlerDestroyed(QObject*)));
  disconnect(handler, SIGNAL(AsyncLoadComplete(UrlHandler::LoadResult)),
             this, SLOT(HandleLoadResult(UrlHandler::LoadResult)));
}

void Player::UrlHandlerDestroyed(QObject* object) {
  UrlHandler* handler = static_cast<UrlHandler*>(object);
  const QString scheme = url_handlers_.key(handler);
  if (!scheme.isEmpty()) {
    url_handlers_.remove(scheme);
  }
}
