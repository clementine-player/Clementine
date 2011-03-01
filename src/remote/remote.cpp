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

#include "remote.h"
#include "remoteconfig.h"
#include "core/player.h"
#include "engines/enginebase.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"

#include <QSettings>
#include <QTimer>

Remote::Remote(Player* player, QObject* parent)
  : QObject(parent),
    player_(player),
    connection_(new xrme::Connection(this)),
    retry_count_(0)
{
  connection_->SetMediaPlayer(this);
  connection_->set_verbose(true);
  connect(connection_, SIGNAL(Connected()), SLOT(Connected()));
  connect(connection_, SIGNAL(Disconnected(QString)), SLOT(Disconnected(QString)));

  connect(player_, SIGNAL(Playing()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(Paused()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(Stopped()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(PlaylistFinished()), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(VolumeChanged(int)), SLOT(SetStateChanged()));
  connect(player_, SIGNAL(Seeked(qlonglong)), SLOT(SetStateChanged()));
  connect(player_->playlists(), SIGNAL(CurrentSongChanged(Song)), SLOT(SetStateChanged()));

  ReloadSettings();
}

void Remote::ReloadSettings() {
  QSettings s;
  s.beginGroup(RemoteConfig::kSettingsGroup);

  QString username = s.value("username").toString();
  QString password = s.value("token").toString();
  QString agent_name = s.value("agent_name", RemoteConfig::DefaultAgentName()).toString();

  // Have the settings changed?
  if (username != connection_->username() ||
      password != connection_->password() ||
      agent_name != connection_->agent_name()) {
    connection_->set_username(username);
    connection_->set_agent_name(agent_name);
    connection_->set_password(password);

    if (connection_->is_connected()) {
      // We'll reconnect later
      connection_->Disconnect();
    } else if (is_configured()) {
      connection_->Connect();
    }
  }
}

bool Remote::is_configured() const {
  return !connection_->username().isEmpty() &&
         !connection_->password().isEmpty() &&
         !connection_->agent_name().isEmpty();
}

void Remote::Connected() {
  retry_count_ = 0;
}

void Remote::Disconnected(const QString& error) {
  if (retry_count_++ >= kMaxRetries) {
    // Show an error and give up if we're above the retry count
    if (!error.isEmpty()) {
      emit Error("XMPP remote control disconnected: " + error);
    }
  } else if (is_configured()) {
    // Try again
    QTimer::singleShot(0, connection_, SLOT(Connect()));
  }
}

void Remote::PlayPause() {
  player_->PlayPause();
}

void Remote::Stop() {
  player_->Stop();
}

void Remote::Next() {
  player_->Next();
}

void Remote::Previous() {
  player_->Previous();
}

xrme::State Remote::state() const {
  const Playlist* active = player_->playlists()->active();
  const Engine::State state = player_->GetState();
  const PlaylistItemPtr current_item = player_->GetCurrentItem();

  xrme::State ret;
  ret.can_go_next = active->next_row() != -1 ||
                    active->current_item_options() & PlaylistItem::ContainsMultipleTracks;
  ret.can_go_previous = active->previous_row() != -1;
  ret.can_seek = current_item &&
                 current_item->Metadata().filetype() != Song::Type_Stream;

  switch (state) {
    case Engine::Playing: ret.playback_state = xrme::State::PlaybackState_Playing; break;
    case Engine::Paused:  ret.playback_state = xrme::State::PlaybackState_Paused;  break;
    case Engine::Idle:
    case Engine::Empty:   ret.playback_state = xrme::State::PlaybackState_Stopped; break;
  }

  ret.position_millisec = player_->engine()->position_nanosec() / kNsecPerMsec;
  ret.volume = double(player_->GetVolume()) / 100;

  if (current_item) {
    const Song m = current_item->Metadata();

    ret.metadata.title = m.title();
    ret.metadata.artist = m.artist();
    ret.metadata.album = m.album();
    ret.metadata.albumartist = m.albumartist();
    ret.metadata.composer = m.composer();
    ret.metadata.genre = m.genre();
    ret.metadata.track = m.track();
    ret.metadata.disc = m.disc();
    ret.metadata.year = m.year();
    ret.metadata.length_millisec = m.length_nanosec() / kNsecPerMsec;
    ret.metadata.rating = m.rating();
  }

  return ret;
}

QImage Remote::album_art() const {
  return last_image_;
}

void Remote::SetStateChanged() {
  StateChanged();
}

void Remote::ArtLoaded(const Song&, const QString&, const QImage& image) {
  last_image_ = image;
  AlbumArtChanged();
}
