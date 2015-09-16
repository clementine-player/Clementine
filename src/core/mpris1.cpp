/* This file is part of Clementine.
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2011-2012, David Sansome <me@davidsansome.com>
   Copyright 2013, Uwe Klotz <uwe.klotz@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "mpris1.h"
#include "mpris_common.h"
#include "core/application.h"
#include "core/logging.h"
#include "covers/currentartloader.h"

#include <QCoreApplication>
#include <QDBusConnection>

#include "core/mpris_player.h"
#include "core/mpris_root.h"
#include "core/mpris_tracklist.h"
#include "core/timeconstants.h"
#include "engines/enginebase.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"

namespace mpris {

const char* Mpris1::kDefaultDbusServiceName = "org.mpris.clementine";

Mpris1::Mpris1(Application* app, QObject* parent,
               const QString& dbus_service_name)
    : QObject(parent),
      dbus_service_name_(dbus_service_name),
      root_(nullptr),
      player_(nullptr),
      tracklist_(nullptr) {
  qDBusRegisterMetaType<DBusStatus>();
  qDBusRegisterMetaType<Version>();

  if (dbus_service_name_.isEmpty()) {
    dbus_service_name_ = kDefaultDbusServiceName;
  }

  if (!QDBusConnection::sessionBus().registerService(dbus_service_name_)) {
    qLog(Warning) << "Failed to register" << dbus_service_name_
                  << "on the session bus";
    return;
  }

  root_ = new Mpris1Root(app, this);
  player_ = new Mpris1Player(app, this);
  tracklist_ = new Mpris1TrackList(app, this);

  connect(app->current_art_loader(),
          SIGNAL(ArtLoaded(const Song&, const QString&, const QImage&)),
          player_,
          SLOT(CurrentSongChanged(const Song&, const QString&, const QImage&)));
}

Mpris1::~Mpris1() {
  QDBusConnection::sessionBus().unregisterService(dbus_service_name_);
}

Mpris1Root::Mpris1Root(Application* app, QObject* parent)
    : QObject(parent), app_(app) {
  new MprisRoot(this);
  QDBusConnection::sessionBus().registerObject("/", this);
}

Mpris1Player::Mpris1Player(Application* app, QObject* parent)
    : QObject(parent), app_(app) {
  new MprisPlayer(this);
  QDBusConnection::sessionBus().registerObject("/Player", this);

  connect(app_->player()->engine(), SIGNAL(StateChanged(Engine::State)),
          SLOT(EngineStateChanged(Engine::State)));
  connect(app_->playlist_manager(), SIGNAL(PlaylistManagerInitialized()),
          SLOT(PlaylistManagerInitialized()));
}

// when PlaylistManager gets it ready, we connect PlaylistSequence with this
void Mpris1Player::PlaylistManagerInitialized() {
  connect(app_->playlist_manager()->sequence(),
          SIGNAL(ShuffleModeChanged(PlaylistSequence::ShuffleMode)),
          SLOT(ShuffleModeChanged()));
  connect(app_->playlist_manager()->sequence(),
          SIGNAL(RepeatModeChanged(PlaylistSequence::RepeatMode)),
          SLOT(RepeatModeChanged()));
}

Mpris1TrackList::Mpris1TrackList(Application* app, QObject* parent)
    : QObject(parent), app_(app) {
  new MprisTrackList(this);
  QDBusConnection::sessionBus().registerObject("/TrackList", this);

  connect(app_->playlist_manager(), SIGNAL(PlaylistChanged(Playlist*)),
          SLOT(PlaylistChanged(Playlist*)));
}

void Mpris1TrackList::PlaylistChanged(Playlist* playlist) {
  emit TrackListChange(playlist->rowCount());
}

// we use the state from event and don't try to obtain it from Player
// later because only the event's version is really the current one
void Mpris1Player::EngineStateChanged(Engine::State state) {
  emit StatusChange(GetStatus(state));
  emit CapsChange(GetCaps(state));
}

void Mpris1Player::CurrentSongChanged(const Song& song, const QString& art_uri,
                                      const QImage&) {
  last_metadata_ = Mpris1::GetMetadata(song);

  if (!art_uri.isEmpty()) {
    AddMetadata("arturl", art_uri, &last_metadata_);
  }

  emit TrackChange(last_metadata_);
  emit StatusChange(GetStatus());
  emit CapsChange(GetCaps());
}

QString Mpris1Root::Identity() {
  return QString("%1 %2").arg(QCoreApplication::applicationName(),
                              QCoreApplication::applicationVersion());
}

Version Mpris1Root::MprisVersion() {
  Version version;
  version.major = 1;
  version.minor = 0;
  return version;
}

void Mpris1Root::Quit() { qApp->quit(); }

void Mpris1Player::Pause() { app_->player()->PlayPause(); }

void Mpris1Player::Stop() { app_->player()->Stop(); }

void Mpris1Player::Prev() { app_->player()->Previous(); }

void Mpris1Player::Play() { app_->player()->Play(); }

void Mpris1Player::Next() { app_->player()->Next(); }

void Mpris1Player::Repeat(bool repeat) {
  app_->playlist_manager()->sequence()->SetRepeatMode(
      repeat ? PlaylistSequence::Repeat_Track : PlaylistSequence::Repeat_Off);
}

void Mpris1Player::ShuffleModeChanged() { emit StatusChange(GetStatus()); }

void Mpris1Player::RepeatModeChanged() { emit StatusChange(GetStatus()); }

DBusStatus Mpris1Player::GetStatus() const {
  return GetStatus(app_->player()->GetState());
}

DBusStatus Mpris1Player::GetStatus(Engine::State state) const {
  DBusStatus status;
  switch (state) {
    case Engine::Playing:
      status.play = DBusStatus::Mpris_Playing;
      break;
    case Engine::Paused:
      status.play = DBusStatus::Mpris_Paused;
      break;
    case Engine::Empty:
    case Engine::Idle:
    default:
      status.play = DBusStatus::Mpris_Stopped;
      break;
  }

  if (app_->playlist_manager()->sequence()) {
    PlaylistManagerInterface* playlists_ = app_->playlist_manager();
    PlaylistSequence::RepeatMode repeat_mode =
        playlists_->sequence()->repeat_mode();

    status.random =
        playlists_->sequence()->shuffle_mode() == PlaylistSequence::Shuffle_Off
            ? 0
            : 1;
    status.repeat = repeat_mode == PlaylistSequence::Repeat_Track ? 1 : 0;
    status.repeat_playlist =
        (repeat_mode == PlaylistSequence::Repeat_Album ||
         repeat_mode == PlaylistSequence::Repeat_Playlist ||
         repeat_mode == PlaylistSequence::Repeat_Track)
            ? 1
            : 0;
  }
  return status;
}

void Mpris1Player::VolumeSet(int volume) { app_->player()->SetVolume(volume); }

int Mpris1Player::VolumeGet() const { return app_->player()->GetVolume(); }

void Mpris1Player::PositionSet(int pos_msec) {
  app_->player()->SeekTo(pos_msec / kMsecPerSec);
}

int Mpris1Player::PositionGet() const {
  return app_->player()->engine()->position_nanosec() / kNsecPerMsec;
}

QVariantMap Mpris1Player::GetMetadata() const { return last_metadata_; }

int Mpris1Player::GetCaps() const {
  return GetCaps(app_->player()->GetState());
}

int Mpris1Player::GetCaps(Engine::State state) const {
  int caps = CAN_HAS_TRACKLIST;
  PlaylistItemPtr current_item = app_->player()->GetCurrentItem();
  PlaylistManagerInterface* playlists = app_->playlist_manager();

  if (playlists->active()) {
    // play is disabled when playlist is empty or when last.fm stream is already
    // playing
    if (playlists->active() && playlists->active()->rowCount() != 0 &&
        !(state == Engine::Playing &&
          (app_->player()->GetCurrentItem()->options() &
           PlaylistItem::LastFMControls))) {
      caps |= CAN_PLAY;
    }

    if (playlists->active()->next_row() != -1) {
      caps |= CAN_GO_NEXT;
    }
    if (playlists->active()->previous_row() != -1 ||
        app_->player()->PreviousWouldRestartTrack()) {
      caps |= CAN_GO_PREV;
    }
  }

  if (current_item) {
    caps |= CAN_PROVIDE_METADATA;
    if (state == Engine::Playing &&
        !(current_item->options() & PlaylistItem::PauseDisabled)) {
      caps |= CAN_PAUSE;
    }
    if (state != Engine::Empty && !current_item->Metadata().is_stream()) {
      caps |= CAN_SEEK;
    }
  }

  return caps;
}

void Mpris1Player::VolumeUp(int change) { VolumeSet(VolumeGet() + change); }

void Mpris1Player::VolumeDown(int change) { VolumeSet(VolumeGet() - change); }

void Mpris1Player::Mute() { app_->player()->Mute(); }

void Mpris1Player::ShowOSD() { app_->player()->ShowOSD(); }

int Mpris1TrackList::AddTrack(const QString& track, bool play) {
  app_->playlist_manager()->active()->InsertUrls(QList<QUrl>() << QUrl(track),
                                                 -1, play);
  return 0;
}

void Mpris1TrackList::DelTrack(int index) {
  app_->playlist_manager()->active()->removeRows(index, 1);
}

int Mpris1TrackList::GetCurrentTrack() const {
  return app_->playlist_manager()->active()->current_row();
}

int Mpris1TrackList::GetLength() const {
  return app_->playlist_manager()->active()->rowCount();
}

QVariantMap Mpris1TrackList::GetMetadata(int pos) const {
  PlaylistItemPtr item = app_->player()->GetItemAt(pos);
  if (!item) return QVariantMap();

  return Mpris1::GetMetadata(item->Metadata());
}

void Mpris1TrackList::SetLoop(bool enable) {
  app_->playlist_manager()->active()->sequence()->SetRepeatMode(
      enable ? PlaylistSequence::Repeat_Playlist
             : PlaylistSequence::Repeat_Off);
}

void Mpris1TrackList::SetRandom(bool enable) {
  app_->playlist_manager()->active()->sequence()->SetShuffleMode(
      enable ? PlaylistSequence::Shuffle_All : PlaylistSequence::Shuffle_Off);
}

void Mpris1TrackList::PlayTrack(int index) {
  app_->player()->PlayAt(index, Engine::Manual, true);
}

QVariantMap Mpris1::GetMetadata(const Song& song) {
  QVariantMap ret;

  AddMetadata("location", song.url().toString(), &ret);
  AddMetadata("title", song.PrettyTitle(), &ret);
  AddMetadata("artist", song.artist(), &ret);
  AddMetadata("album", song.album(), &ret);
  AddMetadata("time", song.length_nanosec() / kNsecPerSec, &ret);
  AddMetadata("mtime", song.length_nanosec() / kNsecPerMsec, &ret);
  AddMetadata("tracknumber", song.track(), &ret);
  AddMetadata("year", song.year(), &ret);
  AddMetadata("genre", song.genre(), &ret);
  AddMetadata("disc", song.disc(), &ret);
  AddMetadata("comment", song.comment(), &ret);
  AddMetadata("audio-bitrate", song.bitrate(), &ret);
  AddMetadata("audio-samplerate", song.samplerate(), &ret);
  AddMetadata("bpm", song.bpm(), &ret);
  AddMetadata("composer", song.composer(), &ret);
  AddMetadata("performer", song.performer(), &ret);
  AddMetadata("grouping", song.grouping(), &ret);
  AddMetadata("lyrics", song.lyrics(), &ret);
  if (song.rating() != -1.0) {
    AddMetadata("rating", song.rating() * 5, &ret);
  }

  return ret;
}

}  // namespace mpris

QDBusArgument& operator<<(QDBusArgument& arg, const Version& version) {
  arg.beginStructure();
  arg << version.major << version.minor;
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, Version& version) {
  arg.beginStructure();
  arg >> version.major >> version.minor;
  arg.endStructure();
  return arg;
}

QDBusArgument& operator<<(QDBusArgument& arg, const DBusStatus& status) {
  arg.beginStructure();
  arg << status.play;
  arg << status.random;
  arg << status.repeat;
  arg << status.repeat_playlist;
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, DBusStatus& status) {
  arg.beginStructure();
  arg >> status.play;
  arg >> status.random;
  arg >> status.repeat;
  arg >> status.repeat_playlist;
  arg.endStructure();
  return arg;
}
