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
#include "mpris1.h"
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
#include <QtConcurrentRun>

#ifdef HAVE_LIBINDICATE
# include <qindicateserver.h>
#endif

namespace mpris {

const char* Mpris2::kMprisObjectPath = "/org/mpris/MediaPlayer2";
const char* Mpris2::kServiceName = "org.mpris.MediaPlayer2.clementine";
const char* Mpris2::kFreedesktopPath = "org.freedesktop.DBus.Properties";

Mpris2::Mpris2(Player* player, ArtLoader* art_loader,
               Mpris1* mpris1, QObject* parent)
  : QObject(parent),
    player_(player),
    mpris1_(mpris1)
{
  new Mpris2Root(this);
  new Mpris2TrackList(this);
  new Mpris2Player(this);

  QDBusConnection::sessionBus().registerService(kServiceName);
  QDBusConnection::sessionBus().registerObject(kMprisObjectPath, this);

  connect(art_loader, SIGNAL(ArtLoaded(Song,QString)), SLOT(ArtLoaded(Song,QString)));

  connect(player->engine(), SIGNAL(StateChanged(Engine::State)), SLOT(EngineStateChanged(Engine::State)));
  connect(player, SIGNAL(VolumeChanged(int)), SLOT(VolumeChanged()));
  connect(player, SIGNAL(Seeked(qlonglong)), SIGNAL(Seeked(qlonglong)));

  connect(player->playlists(), SIGNAL(PlaylistManagerInitialized()), SLOT(PlaylistManagerInitialized()));
  connect(player->playlists(), SIGNAL(CurrentSongChanged(Song)), SLOT(CurrentSongChanged(Song)));
}

void Mpris2::InitLibIndicate() {
#ifdef HAVE_LIBINDICATE
  QIndicate::Server* indicate_server = QIndicate::Server::defaultInstance();
  indicate_server->setType("music.clementine");
  indicate_server->setDesktopFile(DesktopEntryAbsolutePath());
  indicate_server->show();
#endif
}

// when PlaylistManager gets it ready, we connect PlaylistSequence with this
void Mpris2::PlaylistManagerInitialized() {
  connect(player_->playlists()->sequence(), SIGNAL(ShuffleModeChanged(PlaylistSequence::ShuffleMode)),
          SLOT(ShuffleModeChanged()));
  connect(player_->playlists()->sequence(), SIGNAL(RepeatModeChanged(PlaylistSequence::RepeatMode)),
          SLOT(RepeatModeChanged()));
}

void Mpris2::EngineStateChanged(Engine::State newState) {
  if(newState != Engine::Playing && newState != Engine::Paused) {
    last_metadata_= QVariantMap();
    EmitNotification("Metadata");
  }

  EmitNotification("PlaybackStatus", PlaybackStatus(newState));
}

void Mpris2::VolumeChanged() {
  EmitNotification("Volume");
}

void Mpris2::ShuffleModeChanged() {
  EmitNotification("Shuffle");
}

void Mpris2::RepeatModeChanged() {
  EmitNotification("LoopStatus");
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

QString Mpris2::DesktopEntryAbsolutePath() const {
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

QString Mpris2::DesktopEntry() const {
  return QApplication::applicationName().toLower();
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
  emit RaiseMainWindow();
}

void Mpris2::Quit() {
  qApp->quit();
}

QString Mpris2::PlaybackStatus() const {
  return PlaybackStatus(player_->GetState());
}

QString Mpris2::PlaybackStatus(Engine::State state) const {
  switch (state) {
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
}

double Mpris2::Rate() const {
  return 1.0;
}

void Mpris2::SetRate(double rate) {
  if(rate == 0) {
    mpris1_->player()->Pause();
  }
}

bool Mpris2::Shuffle() const {
  return mpris1_->player()->GetStatus().random;
}

void Mpris2::SetShuffle(bool value) {
  mpris1_->tracklist()->SetRandom(value);
}

QVariantMap Mpris2::Metadata() const {
  return last_metadata_;
}

QString Mpris2::current_track_id() const {
  return QString("/org/mpris/MediaPlayer2/Track/%1").arg(
        QString::number(mpris1_->tracklist()->GetCurrentTrack()));
}

// We send Metadata change notification as soon as the process of
// changing song starts...
void Mpris2::CurrentSongChanged(const Song& song) {
  ArtLoaded(song, "");
}

// ... and we add the cover information later, when it's available.
void Mpris2::ArtLoaded(const Song& song, const QString& art_uri) {
  last_metadata_ = QVariantMap();

  using mpris::AddMetadata;
  AddMetadata("mpris:trackid", current_track_id(), &last_metadata_);
  AddMetadata("xesam:url", song.filename(), &last_metadata_);
  AddMetadata("xesam:title", song.PrettyTitle(), &last_metadata_);
  AddMetadataAsList("xesam:artist", song.artist(), &last_metadata_);
  AddMetadata("xesam:album", song.album(), &last_metadata_);
  AddMetadataAsList("xesam:albumArtist", song.albumartist(), &last_metadata_);
  AddMetadata("mpris:length", song.length()*1e6, &last_metadata_);
  AddMetadata("xesam:trackNumber", song.track(), &last_metadata_);
  AddMetadataAsList("xesam:genre", song.genre(), &last_metadata_);
  AddMetadata("xesam:discNumber", song.disc(), &last_metadata_);
  AddMetadataAsList("xesam:comment", song.comment(), &last_metadata_);
  AddMetadata("xesam:contentCreated", AsMPRISDateTimeType(song.ctime()), &last_metadata_);
  AddMetadata("xesam:lastUsed", AsMPRISDateTimeType(song.lastplayed()), &last_metadata_);
  AddMetadata("xesam:audioBPM", song.bpm(), &last_metadata_);
  AddMetadataAsList("xesam:composer", song.composer(), &last_metadata_);
  AddMetadata("xesam:useCount", song.playcount(), &last_metadata_);
  AddMetadata("xesam:autoRating", song.score(), &last_metadata_);
  if (song.rating() != -1.0) {
    AddMetadata("rating", song.rating() * 5, &last_metadata_);
  }
  if (!art_uri.isEmpty()) {
    AddMetadata("mpris:artUrl", art_uri, &last_metadata_);
  }

  EmitNotification("Metadata", last_metadata_);
}

double Mpris2::Volume() const {
  return double(mpris1_->player()->VolumeGet()) / 100;
}

void Mpris2::SetVolume(double value) {
  player_->SetVolume(value * 100);
}

qlonglong Mpris2::Position() const {
  return mpris1_->player()->PositionGet() / 1e3;
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

// This one's a bit different than MPRIS 1 - we want this to be true even when
// the song is already paused.
bool Mpris2::CanPause() const {
  return mpris1_->player()->GetCaps() & CAN_PAUSE || PlaybackStatus() == "Paused";
}

bool Mpris2::CanSeek() const {
  return mpris1_->player()->GetCaps() & CAN_SEEK;
}

bool Mpris2::CanControl() const {
  return true;
}

void Mpris2::Next() {
  if(CanGoNext()) {
    player_->Next();
  }
}

void Mpris2::Previous() {
  if(CanGoPrevious()) {
    player_->Previous();
  }
}

void Mpris2::Pause() {
  if(CanPause() && player_->GetState() != Engine::Paused) {
    player_->Pause();
  }
}

void Mpris2::PlayPause() {
  if(CanPause()) {
    player_->PlayPause();
  } else {
    // TODO: raise an error
  }
}

void Mpris2::Stop() {
  player_->Stop();
}

void Mpris2::Play() {
  if(CanPlay()) {
    player_->Play();
  }
}

void Mpris2::Seek(qlonglong offset) {
  if(CanSeek()) {
    player_->Seek(player_->engine()->position_nanosec() / 1e9 + offset / 1e6);
  }
}

void Mpris2::SetPosition(const QDBusObjectPath& trackId, qlonglong offset) {
  if (CanSeek() && trackId.path() == current_track_id() && offset >= 0) {
    offset *= 1e3;

    if(offset < player_->GetCurrentItem()->Metadata().length()) {
      player_->Seek(offset / 1e9);
    }
  }
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
