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

#include "artloader.h"
#include "mpris1.h"
#include "mpris_common.h"

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

Mpris1::Mpris1(PlayerInterface* player, ArtLoader* art_loader, QObject* parent,
               const QString& dbus_service_name)
  : QObject(parent),
    dbus_service_name_(dbus_service_name),
    root_(NULL),
    player_(NULL),
    tracklist_(NULL)
{
  qDBusRegisterMetaType<DBusStatus>();
  qDBusRegisterMetaType<Version>();

  if (dbus_service_name_.isEmpty()) {
    dbus_service_name_ = kDefaultDbusServiceName;
  }

  if (!QDBusConnection::sessionBus().registerService(dbus_service_name_)) {
    qWarning() << "Failed to register" << dbus_service_name_ << "on the session bus";
    return;
  }

  root_ = new Mpris1Root(player, this);
  player_ = new Mpris1Player(player, this);
  tracklist_ = new Mpris1TrackList(player, this);

  connect(art_loader, SIGNAL(ArtLoaded(const Song&, const QString&, const QImage&)),
          player_, SLOT(CurrentSongChanged(const Song&, const QString&, const QImage&)));
}

Mpris1::~Mpris1() {
  QDBusConnection::sessionBus().unregisterService(dbus_service_name_);
}

Mpris1Root::Mpris1Root(PlayerInterface* player, QObject* parent)
    : QObject(parent), player_(player) {
  new MprisRoot(this);
  QDBusConnection::sessionBus().registerObject("/", this);
}

Mpris1Player::Mpris1Player(PlayerInterface* player, QObject* parent)
    : QObject(parent), player_(player) {
  new MprisPlayer(this);
  QDBusConnection::sessionBus().registerObject("/Player", this);

  connect(player->engine(), SIGNAL(StateChanged(Engine::State)), SLOT(EngineStateChanged(Engine::State)));
  connect(player_->playlists(), SIGNAL(PlaylistManagerInitialized()), SLOT(PlaylistManagerInitialized()));
}

// when PlaylistManager gets it ready, we connect PlaylistSequence with this
void Mpris1Player::PlaylistManagerInitialized() {
  connect(player_->playlists()->sequence(), SIGNAL(ShuffleModeChanged(PlaylistSequence::ShuffleMode)),
          SLOT(ShuffleModeChanged()));
  connect(player_->playlists()->sequence(), SIGNAL(RepeatModeChanged(PlaylistSequence::RepeatMode)),
          SLOT(RepeatModeChanged()));
}

Mpris1TrackList::Mpris1TrackList(PlayerInterface* player, QObject* parent)
    : QObject(parent), player_(player) {
  new MprisTrackList(this);
  QDBusConnection::sessionBus().registerObject("/TrackList", this);

  connect(player->playlists(), SIGNAL(PlaylistChanged(Playlist*)), SLOT(PlaylistChanged(Playlist*)));
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

void Mpris1Player::CurrentSongChanged(
    const Song& song, const QString& art_uri, const QImage&) {
  last_metadata_ = Mpris1::GetMetadata(song);

  if (!art_uri.isEmpty()) {
    AddMetadata("arturl", art_uri, &last_metadata_);
  }

  emit TrackChange(last_metadata_);
  emit StatusChange(GetStatus());
  emit CapsChange(GetCaps());
}


QString Mpris1Root::Identity() {
  return QString("%1 %2").arg(
      QCoreApplication::applicationName(),
      QCoreApplication::applicationVersion());
}

Version Mpris1Root::MprisVersion() {
  Version version;
  version.major = 1;
  version.minor = 0;
  return version;
}

void Mpris1Root::Quit() {
  qApp->quit();
}

void Mpris1Player::Pause() {
  player_->Pause();
}

void Mpris1Player::Stop() {
  player_->Stop();
}

void Mpris1Player::Prev() {
  player_->Previous();
}

void Mpris1Player::Play() {
  player_->Play();
}

void Mpris1Player::Next() {
  player_->Next();
}

void Mpris1Player::Repeat(bool repeat) {
  player_->playlists()->sequence()->SetRepeatMode(
      repeat ? PlaylistSequence::Repeat_Track : PlaylistSequence::Repeat_Off);
}

void Mpris1Player::ShuffleModeChanged() {
  emit StatusChange(GetStatus());
}

void Mpris1Player::RepeatModeChanged() {
  emit StatusChange(GetStatus());
}

DBusStatus Mpris1Player::GetStatus() const {
  return GetStatus(player_->GetState());
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

  PlaylistManagerInterface* playlists_ = player_->playlists();
  PlaylistSequence::RepeatMode repeat_mode = playlists_->sequence()->repeat_mode();

  status.random = playlists_->sequence()->shuffle_mode() == PlaylistSequence::Shuffle_Off ? 0 : 1;
  status.repeat = repeat_mode == PlaylistSequence::Repeat_Track ? 1 : 0;
  status.repeat_playlist = (repeat_mode == PlaylistSequence::Repeat_Album ||
                            repeat_mode == PlaylistSequence::Repeat_Playlist ||
                            repeat_mode == PlaylistSequence::Repeat_Track) ? 1 : 0;
  return status;

}

void Mpris1Player::VolumeSet(int volume) {
  player_->SetVolume(volume);
}

int Mpris1Player::VolumeGet() const {
  return player_->GetVolume();
}

void Mpris1Player::PositionSet(int pos_msec) {
  player_->SeekTo(pos_msec / kMsecPerSec);
}

int Mpris1Player::PositionGet() const {
  return player_->engine()->position_nanosec() / kNsecPerMsec;
}

QVariantMap Mpris1Player::GetMetadata() const {
  return last_metadata_;
}

int Mpris1Player::GetCaps() const {
  return GetCaps(player_->GetState());
}

int Mpris1Player::GetCaps(Engine::State state) const {
  int caps = CAN_HAS_TRACKLIST;
  PlaylistItemPtr current_item = player_->GetCurrentItem();
  PlaylistManagerInterface* playlists = player_->playlists();

  // play is disabled when playlist is empty or when last.fm stream is already playing
  if (playlists->active()->rowCount() != 0
      && !(state == Engine::Playing && (player_->GetCurrentItem()->options() & PlaylistItem::LastFMControls))) {
    caps |= CAN_PLAY;
  }

  if (current_item) {
    caps |= CAN_PROVIDE_METADATA;
    if (state == Engine::Playing && !(current_item->options() & PlaylistItem::PauseDisabled)) {
      caps |= CAN_PAUSE;
    }
    if (state != Engine::Empty && !current_item->Metadata().is_stream()) {
      caps |= CAN_SEEK;
    }
  }

  if (playlists->active()->next_row() != -1 ||
      playlists->active()->current_item_options() & PlaylistItem::ContainsMultipleTracks) {
    caps |= CAN_GO_NEXT;
  }
  if (playlists->active()->previous_row() != -1) {
    caps |= CAN_GO_PREV;
  }

  return caps;
}

void Mpris1Player::VolumeUp(int change) {
  VolumeSet(VolumeGet() + change);
}

void Mpris1Player::VolumeDown(int change) {
  VolumeSet(VolumeGet() - change);
}

void Mpris1Player::Mute() {
  player_->Mute();
}

void Mpris1Player::ShowOSD() {
  player_->ShowOSD();
}

int Mpris1TrackList::AddTrack(const QString& track, bool play) {
  player_->playlists()->active()->InsertUrls(
        QList<QUrl>() << QUrl(track), -1, play);
  return 0;
}

void Mpris1TrackList::DelTrack(int index) {
  player_->playlists()->active()->removeRows(index, 1);
}

int Mpris1TrackList::GetCurrentTrack() const {
  return player_->playlists()->active()->current_row();
}

int Mpris1TrackList::GetLength() const {
  return player_->playlists()->active()->rowCount();
}

QVariantMap Mpris1TrackList::GetMetadata(int pos) const {
  PlaylistItemPtr item = player_->GetItemAt(pos);
  if (!item)
    return QVariantMap();

  return Mpris1::GetMetadata(item->Metadata());
}

void Mpris1TrackList::SetLoop(bool enable) {
  player_->playlists()->active()->sequence()->SetRepeatMode(
      enable ? PlaylistSequence::Repeat_Playlist : PlaylistSequence::Repeat_Off);
}

void Mpris1TrackList::SetRandom(bool enable) {
  player_->playlists()->active()->sequence()->SetShuffleMode(
      enable ? PlaylistSequence::Shuffle_All : PlaylistSequence::Shuffle_Off);
}

void Mpris1TrackList::PlayTrack(int index) {
  player_->PlayAt(index, Engine::Manual, true);
}

QVariantMap Mpris1::GetMetadata(const Song& song) {
  QVariantMap ret;

  AddMetadata("location", song.filename(), &ret);
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
  if (song.rating() != -1.0) {
    AddMetadata("rating", song.rating() * 5, &ret);
  }

  return ret;
}

} // namespace mpris


QDBusArgument& operator<< (QDBusArgument& arg, const Version& version) {
  arg.beginStructure();
  arg << version.major << version.minor;
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>> (const QDBusArgument& arg, Version& version) {
  arg.beginStructure();
  arg >> version.major >> version.minor;
  arg.endStructure();
  return arg;
}

QDBusArgument& operator<< (QDBusArgument& arg, const DBusStatus& status) {
  arg.beginStructure();
  arg << status.play;
  arg << status.random;
  arg << status.repeat;
  arg << status.repeat_playlist;
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>> (const QDBusArgument& arg, DBusStatus& status) {
  arg.beginStructure();
  arg >> status.play;
  arg >> status.random;
  arg >> status.repeat;
  arg >> status.repeat_playlist;
  arg.endStructure();
  return arg;
}
