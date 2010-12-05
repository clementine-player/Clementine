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
#include "mpris_common.h"
#include "mpris.h"
#include "mpris2.h"
#include "core/mpris2_player.h"
#include "core/mpris2_root.h"
#include "core/mpris2_tracklist.h"
#include "core/player.h"
#include "engines/enginebase.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QDBusConnection>

#ifdef HAVE_LIBINDICATE
# include <qindicateserver.h>
#endif

namespace mpris {

const char* Mpris2::kMprisObjectPath = "/org/mpris/MediaPlayer2";
const char* Mpris2::kServiceName = "org.mpris.MediaPlayer2.clementine";
const char* Mpris2::kFreedesktopPath = "org.freedesktop.DBus.Properties";

Mpris2::Mpris2(MainWindow* main_window, Player* player, ArtLoader* art_loader,
               Mpris1* mpris1, QObject* parent)
  : QObject(parent),
    ui_(main_window),
    player_(player),
    mpris1_(mpris1)
{
  new Mpris2Root(this);
  new Mpris2TrackList(this);
  new Mpris2Player(this);

  QDBusConnection::sessionBus().registerService(kServiceName);
  QDBusConnection::sessionBus().registerObject(kMprisObjectPath, this);

#ifdef HAVE_LIBINDICATE
  QIndicate::Server* indicate_server = QIndicate::Server::defaultInstance();
  indicate_server->setType("music.clementine");
  indicate_server->setDesktopFile(DesktopEntry());
  indicate_server->show();
#endif

  connect(art_loader, SIGNAL(ArtLoaded(Song,QString)), SLOT(ArtLoaded(Song,QString)));

  connect(player->engine(), SIGNAL(StateChanged(Engine::State)), SLOT(EngineStateChanged()));
  connect(player, SIGNAL(VolumeChanged(int)), SLOT(VolumeChanged()));
}

void Mpris2::EngineStateChanged() {
  EmitNotification("PlaybackStatus");
}

void Mpris2::VolumeChanged() {
  EmitNotification("Volume");
}

void Mpris2::EmitNotification(const QString& name, const QVariant& val) {
  QDBusMessage msg = QDBusMessage::createSignal(
        kMprisObjectPath, kFreedesktopPath, "PropertiesChanged");
  QVariantMap map;
  map.insert(name, val);
  QVariantList args = QVariantList()
                      << "org.mpris.MediaPlayer2.Player"
                      << map
                      << QStringList();
  msg.setArguments(args);
  QDBusConnection::sessionBus().send(msg);
}

void Mpris2::EmitNotification(const QString& name) {
  QVariant value;
  if      (name == "PlaybackStatus")  value = PlaybackStatus();
  else if (name == "LoopStatus")      value = LoopStatus();
  else if (name == "Shuffle")         value = Shuffle();
  else if (name == "Metadata")        value = Metadata();
  else if (name == "Volume")          value = Volume();
  else if (name == "Position")        value = Position();

  if (value.isValid())
    EmitNotification(name, value);
}

//------------------Root Interface--------------------------//

bool Mpris2::CanQuit() const {
  return true;
}

bool Mpris2::CanRaise() const {
  return true;
}

bool Mpris2::HasTrackList() const {
  return true;
}

QString Mpris2::Identity() const {
  return QString("%1 %2").arg(
      QCoreApplication::applicationName(),
      QCoreApplication::applicationVersion());
}

QString Mpris2::DesktopEntry() const {
  QStringList xdg_data_dirs = QString(getenv("XDG_DATA_DIRS")).split(":");
  xdg_data_dirs.append("/usr/local/share/");
  xdg_data_dirs.append("/usr/share/");

  foreach (const QString& directory, xdg_data_dirs) {
    QString path = QString("%1/applications/%2.desktop").
                   arg(directory, QApplication::applicationName().toLower());
    if (QFile::exists(path))
      return path;
  }
  return QString();
}

QStringList Mpris2::SupportedUriSchemes() const {
  static QStringList res = QStringList()
                           << "file"
                           << "http"
                           << "cdda"
                           << "smb"
                           << "sftp";
  return res;
}

QStringList Mpris2::SupportedMimeTypes() const {
  static QStringList res = QStringList()
                           << "application/ogg"
                           << "application/x-ogg"
                           << "application/x-ogm-audio"
                           << "audio/aac"
                           << "audio/mp4"
                           << "audio/mpeg"
                           << "audio/mpegurl"
                           << "audio/ogg"
                           << "audio/vnd.rn-realaudio"
                           << "audio/vorbis"
                           << "audio/x-flac"
                           << "audio/x-mp3"
                           << "audio/x-mpeg"
                           << "audio/x-mpegurl"
                           << "audio/x-ms-wma"
                           << "audio/x-musepack"
                           << "audio/x-oggflac"
                           << "audio/x-pn-realaudio"
                           << "audio/x-scpls"
                           << "audio/x-speex"
                           << "audio/x-vorbis"
                           << "audio/x-vorbis+ogg"
                           << "audio/x-wav"
                           << "video/x-ms-asf"
                           << "x-content/audio-player";
  return res;
}

void Mpris2::Raise() {
  ui_->show();
  ui_->activateWindow();
}

void Mpris2::Quit() {
  qApp->quit();
}

QString Mpris2::PlaybackStatus() const {
  switch (player_->GetState()) {
    case Engine::Playing: return "Playing";
    case Engine::Paused:  return "Paused";
    default:              return "Stopped";
  }
}

QString Mpris2::LoopStatus() const {
  switch (player_->playlists()->sequence()->repeat_mode()) {
    case PlaylistSequence::Repeat_Album:
    case PlaylistSequence::Repeat_Playlist: return "Playlist";
    case PlaylistSequence::Repeat_Track:    return "Track";
    default:                                return "None";
  }
}

void Mpris2::SetLoopStatus(const QString& value) {
  PlaylistSequence::RepeatMode mode = PlaylistSequence::Repeat_Off;

  if (value == "None") {
    mode = PlaylistSequence::Repeat_Off;
  } else if (value == "Track") {
    mode = PlaylistSequence::Repeat_Track;
  } else if (value == "Playlist") {
    mode = PlaylistSequence::Repeat_Playlist;
  }

  player_->playlists()->active()->sequence()->SetRepeatMode(mode);
  EmitNotification("LoopStatus", value);
}

double Mpris2::Rate() const {
  return 1.0;
}

void Mpris2::SetRate(double) {
  // Do nothing
}

bool Mpris2::Shuffle() const {
  return player_->playlists()->sequence()->shuffle_mode() !=
      PlaylistSequence::Shuffle_Off;
}

void Mpris2::SetShuffle(bool value) {
  mpris1_->tracklist()->SetRandom(value);
  EmitNotification("Shuffle", value);
}

QVariantMap Mpris2::Metadata() const {
  return last_metadata_;
}

QString Mpris2::current_track_id() const {
  return QString("/org/mpris/MediaPlayer2/Track/%1").arg(
        QString::number(mpris1_->tracklist()->GetCurrentTrack()));
}

void Mpris2::ArtLoaded(const Song& song, const QString& art_uri) {
  last_metadata_ = QVariantMap();

  using mpris::AddMetadata;
  AddMetadata("mpris:trackid", current_track_id(), &last_metadata_);
  AddMetadata("xesam:url", song.filename(), &last_metadata_);
  AddMetadata("xesam:title", song.PrettyTitle(), &last_metadata_);
  AddMetadata("xesam:artist", QStringList() << song.artist(), &last_metadata_);
  AddMetadata("xesam:album", song.album(), &last_metadata_);
  AddMetadata("xesam:albumArtist", song.albumartist(), &last_metadata_);
  AddMetadata("mpris:length", song.length()*1e6, &last_metadata_);
  AddMetadata("xesam:trackNumber", song.track(), &last_metadata_);
  AddMetadata("xesam:genre", song.genre(), &last_metadata_);
  AddMetadata("xesam:discNumber", song.disc(), &last_metadata_);
  AddMetadata("xesam:comment", song.comment(), &last_metadata_);
  AddMetadata("xesam:contentCreated", QDateTime(QDate(song.year(),1,1)), &last_metadata_);
  AddMetadata("xesam:audioBPM", song.bpm(), &last_metadata_);
  AddMetadata("xesam:composer", song.composer(), &last_metadata_);

  if (!art_uri.isEmpty()) {
    AddMetadata("mpris:artUrl", art_uri, &last_metadata_);
  }

  EmitNotification("Metadata", last_metadata_);
}

double Mpris2::Volume() const {
  return double(player_->GetVolume()) / 100;
}

void Mpris2::SetVolume(double value) {
  player_->SetVolume(value * 100);
  EmitNotification("Volume",value);
}

qlonglong Mpris2::Position() const {
  return mpris1_->player()->PositionGet() * 1e3;
}

double Mpris2::MaximumRate() const {
  return 1.0;
}

double Mpris2::MinimumRate() const {
  return 1.0;
}

bool Mpris2::CanGoNext() const {
  return mpris1_->player()->GetCaps() & CAN_GO_NEXT;
}

bool Mpris2::CanGoPrevious() const {
  return mpris1_->player()->GetCaps() & CAN_GO_PREV;
}

bool Mpris2::CanPlay() const {
  return mpris1_->player()->GetCaps() & CAN_PLAY;
}

bool Mpris2::CanPause() const {
  return mpris1_->player()->GetCaps() & CAN_PAUSE;
}

bool Mpris2::CanSeek() const {
  return mpris1_->player()->GetCaps() & CAN_SEEK;
}

bool Mpris2::CanControl() const {
  return true;
}

void Mpris2::Next() {
  player_->Next();
}

void Mpris2::Previous() {
  player_->Previous();
}

void Mpris2::Pause() {
  player_->Pause();
}

void Mpris2::PlayPause() {
  player_->PlayPause();
}

void Mpris2::Stop() {
  player_->Stop();
}

void Mpris2::Play() {
  player_->Play();
}

void Mpris2::Seek(qlonglong offset) {
  player_->Seek((Position() + offset) / 1e6);
}

void Mpris2::SetPosition(const QDBusObjectPath& trackId, qlonglong offset) {
  if (trackId.path() != current_track_id())
    return;

  player_->Seek(offset / 1e6);
}

void Mpris2::OpenUri(const QString& uri) {
  mpris1_->tracklist()->AddTrack(uri, true);
}

TrackIds Mpris2::Tracks() const {
  //TODO
  return TrackIds();
}

bool Mpris2::CanEditTracks() const {
  return false;
}

TrackMetadata Mpris2::GetTracksMetadata(const TrackIds &tracks) const {
  //TODO
  return TrackMetadata();
}

void Mpris2::AddTrack(const QString &uri, const QDBusObjectPath &afterTrack, bool setAsCurrent) {
  //TODO
}

void Mpris2::RemoveTrack(const QDBusObjectPath &trackId) {
  //TODO
}

void Mpris2::GoTo(const QDBusObjectPath &trackId) {
  //TODO
}

} // namespace mpris
