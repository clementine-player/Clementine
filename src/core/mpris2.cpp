/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2010-2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2013, TTSDA <ttsda@ttsda.cc>
   Copyright 2013, Aggelos Biboudis <biboudis@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "mpris2.h"

#include <algorithm>

#include <QApplication>
#include <QDBusConnection>
#include <QtConcurrentRun>

#include "config.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/mpris_common.h"
#include "core/mpris2_player.h"
#include "core/mpris2_playlists.h"
#include "core/mpris2_root.h"
#include "core/mpris2_tracklist.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "covers/currentartloader.h"
#include "engines/enginebase.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "ui/mainwindow.h"

QDBusArgument& operator<<(QDBusArgument& arg, const MprisPlaylist& playlist) {
  arg.beginStructure();
  arg << playlist.id << playlist.name << playlist.icon;
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg,
                                MprisPlaylist& playlist) {
  arg.beginStructure();
  arg >> playlist.id >> playlist.name >> playlist.icon;
  arg.endStructure();
  return arg;
}

QDBusArgument& operator<<(QDBusArgument& arg, const MaybePlaylist& playlist) {
  arg.beginStructure();
  arg << playlist.valid;
  arg << playlist.playlist;
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg,
                                MaybePlaylist& playlist) {
  arg.beginStructure();
  arg >> playlist.valid >> playlist.playlist;
  arg.endStructure();
  return arg;
}

namespace mpris {

const char* Mpris2::kMprisObjectPath = "/org/mpris/MediaPlayer2";
const char* Mpris2::kServiceName = "org.mpris.MediaPlayer2.clementine";
const char* Mpris2::kFreedesktopPath = "org.freedesktop.DBus.Properties";

Mpris2::Mpris2(Application* app, QObject* parent) : QObject(parent), app_(app) {
  new Mpris2Root(this);
  new Mpris2TrackList(this);
  new Mpris2Player(this);
  new Mpris2Playlists(this);

  if (!QDBusConnection::sessionBus().registerService(kServiceName)) {
    qLog(Warning) << "Failed to register" << QString(kServiceName)
                  << "on the session bus";
    return;
  }

  QDBusConnection::sessionBus().registerObject(kMprisObjectPath, this);

  connect(app_->current_art_loader(), SIGNAL(ArtLoaded(Song, QString, QImage)),
          SLOT(ArtLoaded(Song, QString)));

  connect(app_->player()->engine(), SIGNAL(StateChanged(Engine::State)),
          SLOT(EngineStateChanged(Engine::State)));
  connect(app_->player(), SIGNAL(VolumeChanged(int)), SLOT(VolumeChanged()));
  connect(app_->player(), SIGNAL(Seeked(qlonglong)), SIGNAL(Seeked(qlonglong)));

  connect(app_->playlist_manager(), SIGNAL(PlaylistManagerInitialized()),
          SLOT(PlaylistManagerInitialized()));
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(CurrentSongChanged(Song)));
  connect(app_->playlist_manager(), SIGNAL(PlaylistChanged(Playlist*)),
          SLOT(PlaylistChanged(Playlist*)));
  connect(app_->playlist_manager(), SIGNAL(CurrentChanged(Playlist*)),
          SLOT(PlaylistCollectionChanged(Playlist*)));
}

// when PlaylistManager gets it ready, we connect PlaylistSequence with this
void Mpris2::PlaylistManagerInitialized() {
  connect(app_->playlist_manager()->sequence(),
          SIGNAL(ShuffleModeChanged(PlaylistSequence::ShuffleMode)),
          SLOT(ShuffleModeChanged()));
  connect(app_->playlist_manager()->sequence(),
          SIGNAL(RepeatModeChanged(PlaylistSequence::RepeatMode)),
          SLOT(RepeatModeChanged()));
}

void Mpris2::EngineStateChanged(Engine::State newState) {
  if (newState != Engine::Playing && newState != Engine::Paused) {
    last_metadata_ = QVariantMap();
    EmitNotification("Metadata");
  }

  EmitNotification("CanPlay");
  EmitNotification("CanPause");
  EmitNotification("PlaybackStatus", PlaybackStatus(newState));
  if (newState == Engine::Playing)
    EmitNotification("CanSeek", CanSeek(newState));
}

void Mpris2::VolumeChanged() { EmitNotification("Volume"); }

void Mpris2::ShuffleModeChanged() { EmitNotification("Shuffle"); }

void Mpris2::RepeatModeChanged() {
  EmitNotification("LoopStatus");
  EmitNotification("CanGoNext", CanGoNext());
  EmitNotification("CanGoPrevious", CanGoPrevious());
}

void Mpris2::EmitNotification(const QString& name, const QVariant& val) {
  EmitNotification(name, val, "org.mpris.MediaPlayer2.Player");
}

void Mpris2::EmitNotification(const QString& name, const QVariant& val,
                              const QString& mprisEntity) {
  QDBusMessage msg = QDBusMessage::createSignal(
      kMprisObjectPath, kFreedesktopPath, "PropertiesChanged");
  QVariantMap map;
  map.insert(name, val);
  QVariantList args = QVariantList() << mprisEntity << map << QStringList();
  msg.setArguments(args);
  QDBusConnection::sessionBus().send(msg);
}

void Mpris2::EmitNotification(const QString& name) {
  QVariant value;
  if (name == "PlaybackStatus")
    value = PlaybackStatus();
  else if (name == "LoopStatus")
    value = LoopStatus();
  else if (name == "Shuffle")
    value = Shuffle();
  else if (name == "Metadata")
    value = Metadata();
  else if (name == "Volume")
    value = Volume();
  else if (name == "Position")
    value = Position();
  else if (name == "CanGoNext")
    value = CanGoNext();
  else if (name == "CanGoPrevious")
    value = CanGoPrevious();
  else if (name == "CanSeek")
    value = CanSeek();
  else if (name == "CanPlay")
    value = CanPlay();
  else if (name == "CanPause")
    value = CanPause();

  if (value.isValid()) EmitNotification(name, value);
}

// ------------------Root Interface--------------- //

bool Mpris2::CanQuit() const { return true; }

bool Mpris2::CanRaise() const { return true; }

bool Mpris2::HasTrackList() const { return true; }

QString Mpris2::Identity() const { return QCoreApplication::applicationName(); }

QString Mpris2::DesktopEntryAbsolutePath() const {
  QStringList xdg_data_dirs = QString(getenv("XDG_DATA_DIRS")).split(":");
  xdg_data_dirs.append("/usr/local/share/");
  xdg_data_dirs.append("/usr/share/");

  for (const QString& directory : xdg_data_dirs) {
    QString path = QString("%1/applications/%2.desktop").arg(
        directory, QApplication::applicationName().toLower());
    if (QFile::exists(path)) return path;
  }
  return QString();
}

QString Mpris2::DesktopEntry() const {
  return QApplication::applicationName().toLower();
}

QStringList Mpris2::SupportedUriSchemes() const {
  static QStringList res = QStringList() << "file"
                                         << "http"
                                         << "cdda"
                                         << "smb"
                                         << "sftp";
  return res;
}

QStringList Mpris2::SupportedMimeTypes() const {
  static QStringList res = QStringList() << "application/ogg"
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

void Mpris2::Raise() { emit RaiseMainWindow(); }

void Mpris2::Quit() { qApp->quit(); }

QString Mpris2::PlaybackStatus() const {
  return PlaybackStatus(app_->player()->GetState());
}

QString Mpris2::PlaybackStatus(Engine::State state) const {
  switch (state) {
    case Engine::Playing:
      return "Playing";
    case Engine::Paused:
      return "Paused";
    default:
      return "Stopped";
  }
}

QString Mpris2::LoopStatus() const {
  if (!app_->playlist_manager()->sequence()) {
    return "None";
  }

  switch (app_->playlist_manager()->sequence()->repeat_mode()) {
    case PlaylistSequence::Repeat_Album:
    case PlaylistSequence::Repeat_Playlist:
      return "Playlist";
    case PlaylistSequence::Repeat_Track:
      return "Track";
    default:
      return "None";
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

  app_->playlist_manager()->active()->sequence()->SetRepeatMode(mode);
}

double Mpris2::Rate() const { return 1.0; }

void Mpris2::SetRate(double rate) {
  if (rate == 0) {
    app_->player()->Pause();
  }
}

bool Mpris2::Shuffle() const {
  return app_->playlist_manager()->sequence()->shuffle_mode() !=
         PlaylistSequence::Shuffle_Off;
}

void Mpris2::SetShuffle(bool enable) {
  app_->playlist_manager()->active()->sequence()->SetShuffleMode(
      enable ? PlaylistSequence::Shuffle_All : PlaylistSequence::Shuffle_Off);
}

QVariantMap Mpris2::Metadata() const { return last_metadata_; }

QString Mpris2::current_track_id() const {
  return QString("/org/clementineplayer/Clementine/Track/%1")
      .arg(QString::number(app_->playlist_manager()->active()->current_row()));
}

// We send Metadata change notification as soon as the process of
// changing song starts...
void Mpris2::CurrentSongChanged(const Song& song) {
  ArtLoaded(song, "");
  EmitNotification("CanPlay");
  EmitNotification("CanPause");
  EmitNotification("CanGoNext", CanGoNext());
  EmitNotification("CanGoPrevious", CanGoPrevious());
  EmitNotification("CanSeek", CanSeek());
}

// ... and we add the cover information later, when it's available.
void Mpris2::ArtLoaded(const Song& song, const QString& art_uri) {
  last_metadata_ = QVariantMap();
  song.ToXesam(&last_metadata_);

  using mpris::AddMetadata;
  AddMetadata("mpris:trackid", current_track_id(), &last_metadata_);

  if (song.rating() != -1.0) {
    AddMetadata("rating", song.rating() * 5, &last_metadata_);
  }
  if (!art_uri.isEmpty()) {
    AddMetadata("mpris:artUrl", art_uri, &last_metadata_);
  }

  AddMetadata("year", song.year(), &last_metadata_);
  AddMetadata("bitrate", song.bitrate(), &last_metadata_);

  EmitNotification("Metadata", last_metadata_);
}

double Mpris2::Volume() const { return app_->player()->GetVolume() / 100.0; }

void Mpris2::SetVolume(double value) { app_->player()->SetVolume(value * 100); }

qlonglong Mpris2::Position() const {
  return app_->player()->engine()->position_nanosec() / kNsecPerUsec;
}

double Mpris2::MaximumRate() const { return 1.0; }

double Mpris2::MinimumRate() const { return 1.0; }

bool Mpris2::CanGoNext() const {
  return app_->playlist_manager()->active() &&
         app_->playlist_manager()->active()->next_row() != -1;
}

bool Mpris2::CanGoPrevious() const {
  return app_->playlist_manager()->active() &&
         (app_->playlist_manager()->active()->previous_row() != -1 ||
          app_->player()->PreviousWouldRestartTrack());
}

bool Mpris2::CanPlay() const {
  return app_->playlist_manager()->active() &&
         app_->playlist_manager()->active()->rowCount() != 0 &&
         !(app_->player()->GetState() == Engine::Playing &&
           (app_->player()->GetCurrentItem()->options() &
            PlaylistItem::LastFMControls));
}

// This one's a bit different than MPRIS 1 - we want this to be true even when
// the song is already paused or stopped.
bool Mpris2::CanPause() const {
  return (app_->player()->GetCurrentItem() &&
          app_->player()->GetState() == Engine::Playing &&
          !(app_->player()->GetCurrentItem()->options() &
            PlaylistItem::PauseDisabled)) ||
         PlaybackStatus() == "Paused" || PlaybackStatus() == "Stopped";
}

bool Mpris2::CanSeek() const { return CanSeek(app_->player()->GetState()); }

bool Mpris2::CanSeek(Engine::State state) const {
  return app_->player()->GetCurrentItem() && state != Engine::Empty &&
         !app_->player()->GetCurrentItem()->Metadata().is_stream();
}

bool Mpris2::CanControl() const { return true; }

void Mpris2::Next() {
  if (CanGoNext()) {
    app_->player()->Next();
  }
}

void Mpris2::Previous() {
  if (CanGoPrevious()) {
    app_->player()->Previous();
  }
}

void Mpris2::Pause() {
  if (CanPause() && app_->player()->GetState() != Engine::Paused) {
    app_->player()->Pause();
  }
}

void Mpris2::PlayPause() {
  if (CanPause()) {
    app_->player()->PlayPause();
  }
}

void Mpris2::Stop() { app_->player()->Stop(); }

void Mpris2::Play() {
  if (CanPlay()) {
    app_->player()->Play();
  }
}

void Mpris2::Seek(qlonglong offset) {
  if (CanSeek()) {
    app_->player()->SeekTo(app_->player()->engine()->position_nanosec() /
                               kNsecPerSec +
                           offset / kUsecPerSec);
  }
}

void Mpris2::SetPosition(const QDBusObjectPath& trackId, qlonglong offset) {
  if (CanSeek() && trackId.path() == current_track_id() && offset >= 0) {
    offset *= kNsecPerUsec;

    if (offset <
        app_->player()->GetCurrentItem()->Metadata().length_nanosec()) {
      app_->player()->SeekTo(offset / kNsecPerSec);
    }
  }
}

void Mpris2::OpenUri(const QString& uri) {
  app_->playlist_manager()->active()->InsertUrls(QList<QUrl>() << QUrl(uri), -1,
                                                 true);
}

TrackIds Mpris2::Tracks() const {
  // TODO(John Maguire): ?
  return TrackIds();
}

bool Mpris2::CanEditTracks() const { return false; }

TrackMetadata Mpris2::GetTracksMetadata(const TrackIds& tracks) const {
  // TODO(John Maguire): ?
  return TrackMetadata();
}

void Mpris2::AddTrack(const QString& uri, const QDBusObjectPath& afterTrack,
                      bool setAsCurrent) {
  // TODO(John Maguire): ?
}

void Mpris2::RemoveTrack(const QDBusObjectPath& trackId) {
  // TODO(John Maguire): ?
}

void Mpris2::GoTo(const QDBusObjectPath& trackId) {
  // TODO(John Maguire): ?
}

quint32 Mpris2::PlaylistCount() const {
  return app_->playlist_manager()->GetAllPlaylists().size();
}

QStringList Mpris2::Orderings() const { return QStringList() << "User"; }

namespace {

QDBusObjectPath MakePlaylistPath(int id) {
  return QDBusObjectPath(
      QString("/org/clementineplayer/clementine/PlaylistId/%1").arg(id));
}
}

MaybePlaylist Mpris2::ActivePlaylist() const {
  MaybePlaylist maybe_playlist;
  Playlist* current_playlist = app_->playlist_manager()->current();
  maybe_playlist.valid = current_playlist;
  if (!current_playlist) {
    return maybe_playlist;
  }

  maybe_playlist.playlist.id = MakePlaylistPath(current_playlist->id());
  maybe_playlist.playlist.name =
      app_->playlist_manager()->GetPlaylistName(current_playlist->id());
  return maybe_playlist;
}

void Mpris2::ActivatePlaylist(const QDBusObjectPath& playlist_id) {
  QStringList split_path = playlist_id.path().split('/');
  qLog(Debug) << Q_FUNC_INFO << playlist_id.path() << split_path;
  if (split_path.isEmpty()) {
    return;
  }
  bool ok = false;
  int p = split_path.last().toInt(&ok);
  if (!ok) {
    return;
  }
  if (!app_->playlist_manager()->IsPlaylistOpen(p)) {
    qLog(Error) << "Playlist isn't opened!";
    return;
  }
  app_->playlist_manager()->SetActivePlaylist(p);
  app_->player()->Next();
}

// TODO(John Maguire): Support sort orders.
MprisPlaylistList Mpris2::GetPlaylists(quint32 index, quint32 max_count,
                                       const QString& order,
                                       bool reverse_order) {
  MprisPlaylistList ret;
  for (Playlist* p : app_->playlist_manager()->GetAllPlaylists()) {
    MprisPlaylist mpris_playlist;
    mpris_playlist.id = MakePlaylistPath(p->id());
    mpris_playlist.name = app_->playlist_manager()->GetPlaylistName(p->id());
    ret << mpris_playlist;
  }

  if (reverse_order) {
    std::reverse(ret.begin(), ret.end());
  }

  return ret.mid(index, max_count);
}

void Mpris2::PlaylistChanged(Playlist* playlist) {
  MprisPlaylist mpris_playlist;
  mpris_playlist.id = MakePlaylistPath(playlist->id());
  mpris_playlist.name =
      app_->playlist_manager()->GetPlaylistName(playlist->id());
  emit PlaylistChanged(mpris_playlist);
}

void Mpris2::PlaylistCollectionChanged(Playlist* playlist) {
  EmitNotification("PlaylistCount", "", "org.mpris.MediaPlayer2.Playlists");
}

}  // namespace mpris
