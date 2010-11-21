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

#include "mpris.h"

#include <QCoreApplication>
#include <QDBusConnection>

#include "core/mpris_player.h"
#include "core/mpris_root.h"
#include "core/mpris_tracklist.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"

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

MPRIS::MPRIS(Player* player, QObject* parent)
    : QObject(parent),
      player_(player)
{
  MprisRoot* mpris_root = new MprisRoot(this);
  MprisPlayer* mpris_player = new MprisPlayer(this);
  MprisTrackList* mpris_tracklist = new MprisTrackList(this);

  QDBusConnection bus(QDBusConnection::sessionBus());
  bus.registerService("org.mpris.clementine");
  bus.registerObject("/", mpris_root, QDBusConnection::ExportAllContents);
  bus.registerObject("/Player", mpris_player, QDBusConnection::ExportAllContents);
  bus.registerObject("/TrackList", mpris_tracklist, QDBusConnection::ExportAllContents);
}

QString MPRIS::Identity() {
  return QString("%1 %2").arg(
      QCoreApplication::applicationName(),
      QCoreApplication::applicationVersion());
}

Version MPRIS::MprisVersion() {
  Version version;
  version.major = 1;
  version.minor = 0;
  return version;
}

void MPRIS::Quit() {
  qApp->quit();
}

void MPRIS::Pause() {
   player_->Pause();
}

void MPRIS::Stop() {
  player_->Stop();
}

void MPRIS::Prev() {
  player_->Prev();
}

void MPRIS::Play() {
  player_->Prev();
}

void MPRIS::Next() {
  player_->Next();
}

void MPRIS::Repeat(bool repeat) {
  player_->Repeat(repeat);
}

DBusStatus MPRIS::GetStatus() const {
  DBusStatus status;
  switch (player_->GetState()) {
    case Engine::Empty:
    case Engine::Idle:
      status.play = DBusStatus::Mpris_Stopped;
      break;
    case Engine::Playing:
      status.play = DBusStatus::Mpris_Playing;
      break;
    case Engine::Paused:
      status.play = DBusStatus::Mpris_Paused;
      break;
  }

  PlaylistManager* playlists_ = player_->GetPlaylists();
  PlaylistSequence::RepeatMode repeat_mode = playlists_->sequence()->repeat_mode();

  status.random = playlists_->sequence()->shuffle_mode() == PlaylistSequence::Shuffle_Off ? 0 : 1;
  status.repeat = repeat_mode == PlaylistSequence::Repeat_Track ? 1 : 0;
  status.repeat_playlist = (repeat_mode == PlaylistSequence::Repeat_Album ||
                            repeat_mode == PlaylistSequence::Repeat_Playlist) ? 1 : 0;
  return status;

}
void MPRIS::VolumeSet(int volume) {
  player_->SetVolume(volume);
}

int MPRIS::VolumeGet() const {
  return player_->VolumeGet();
}
void MPRIS::PositionSet(int pos) {
  player_->Seek(pos/1000);
}

int MPRIS::PositionGet() const {
  return player_->PositionGet()*1000;
}

QVariantMap MPRIS::GetMetadata() const {
  return GetMetadata(player_->GetCurrentItem());
}

int MPRIS::GetCaps() const {
  int caps = CAN_HAS_TRACKLIST;
  Engine::State state = player_->GetState();
  boost::shared_ptr<PlaylistItem> current_item_ = player_->GetCurrentItem();
  PlaylistManager* playlists_ = player_->GetPlaylists();

  if (state == Engine::Paused) {
    caps |= CAN_PLAY;
  }

  if (current_item_) {
    caps |= CAN_PROVIDE_METADATA;
    if (state == Engine::Playing && current_item_->options() & PlaylistItem::PauseDisabled) {
      caps |= CAN_PAUSE;
    }
    if (state != Engine::Empty && current_item_->Metadata().filetype() != Song::Type_Stream) {
      caps |= CAN_SEEK;
    }
  }

  if (playlists_->active()->next_index() != -1 ||
      playlists_->active()->current_item_options() & PlaylistItem::ContainsMultipleTracks) {
    caps |= CAN_GO_NEXT;
  }
  if (playlists_->active()->previous_index() != -1) {
    caps |= CAN_GO_PREV;
  }

  return caps;
}

void MPRIS::VolumeUp(int vol) {
  player_->VolumeUp(vol);
}

void MPRIS::VolumeDown(int vol) {
  player_->VolumeDown(vol);
}

void MPRIS::Mute() {
  player_->Mute();
}

void MPRIS::ShowOSD() {
  player_->ShowOSD();
}

int MPRIS::AddTrack(const QString& track, bool play) {
  return player_->AddTrack(track, play);
}

void MPRIS::DelTrack(int index) {
  player_->DelTrack(index);
}

int MPRIS::GetCurrentTrack() const {
  return player_->GetCurrentTrack();
}

int MPRIS::GetLength() const {
  return player_->GetLength();
}

QVariantMap MPRIS::GetMetadata(int pos) const {
  return GetMetadata(player_->GetItemAt(pos));
}

void MPRIS::SetLoop(bool enable) {
  player_->SetLoop(enable);
}

void MPRIS::SetRandom(bool enable) {
  player_->SetRandom(enable);
}

void MPRIS::PlayTrack(int index) {
  player_->PlayAt(index, Engine::Manual, true);
}

void MPRIS::EmitCapsChange(int param) {
  emit CapsChange(param);
}

void MPRIS::EmitTrackChange(QVariantMap param) {
  emit TrackChange(param);
}

void MPRIS::EmitStatusChange(DBusStatus param) {
  emit StatusChange(param);
}

void MPRIS::EmitTrackListChange(int i) {
  emit TrackListChange(i);
}


QVariantMap MPRIS::GetMetadata(PlaylistItemPtr item) const {
  using metadata::AddMetadata;
  QVariantMap ret;

  if (!item)
    return ret;

  Song song = item->Metadata();
  AddMetadata("location", item->Url().toString(), &ret);
  AddMetadata("title", song.PrettyTitle(), &ret);
  AddMetadata("artist", song.artist(), &ret);
  AddMetadata("album", song.album(), &ret);
  AddMetadata("time", song.length(), &ret);
  AddMetadata("tracknumber", song.track(), &ret);
  AddMetadata("year", song.year(), &ret);
  AddMetadata("genre", song.genre(), &ret);
  AddMetadata("disc", song.disc(), &ret);
  AddMetadata("comment", song.comment(), &ret);
  AddMetadata("bitrate", song.bitrate(), &ret);
  AddMetadata("samplerate", song.samplerate(), &ret);
  AddMetadata("bpm", song.bpm(), &ret);
  AddMetadata("composer", song.composer(), &ret);

  return ret;
}
