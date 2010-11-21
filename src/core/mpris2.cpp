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
#include "mpris.h"
#include "mpris2.h"
#include "core/albumcoverloader.h"
#include "core/mpris2_player.h"
#include "core/mpris2_root.h"
#include "core/mpris2_tracklist.h"
#include "core/player.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QDBusConnection>

#ifdef HAVE_LIBINDICATE
# include <qindicateserver.h>
#endif

const char* MPRIS2::kMprisObjectPath = "/org/mpris/MediaPlayer2";
const char* MPRIS2::kServiceName = "org.mpris.MediaPlayer2.clementine";
const char* MPRIS2::kFreedesktopPath = "org.freedesktop.DBus.Properties";

MPRIS2::MPRIS2(MainWindow* main_window, Player* player, QObject* parent)
  : QObject(parent),
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    art_request_id_(0),
    ui_(main_window),
    player_(player)
{
  cover_loader_->Start();
  connect(cover_loader_, SIGNAL(Initialised()), SLOT(Initialised()));

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
}

void MPRIS2::emitNotification(const QString& name, const QVariant& val) {
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

void MPRIS2::emitNotification(const QString& name) {
  QVariant value;
  if      (name == "PlaybackStatus")  value = PlaybackStatus();
  else if (name == "LoopStatus")      value = LoopStatus();
  else if (name == "Shuffle")         value = Shuffle();
  else if (name == "Metadata")        value = Metadata();
  else if (name == "Volume")          value = Volume();
  else if (name == "Position")        value = Position();

  if (value.isValid())
    emitNotification(name, value);
}

//------------------Root Interface--------------------------//

bool MPRIS2::CanQuit() const {
  return true;
}

bool MPRIS2::CanRaise() const {
  return true;
}

bool MPRIS2::HasTrackList() const {
  return true;
}

QString MPRIS2::Identity() const {
  return QString("%1 %2").arg(
      QCoreApplication::applicationName(),
      QCoreApplication::applicationVersion());
}

QString MPRIS2::DesktopEntry() const {
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

QStringList MPRIS2::SupportedUriSchemes() const {
  static QStringList res = QStringList()
                           << "file"
                           << "http"
                           << "cdda"
                           << "smb"
                           << "sftp";
  return res;
}

QStringList MPRIS2::SupportedMimeTypes() const {
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

void MPRIS2::Raise() {
  ui_->show();
  ui_->activateWindow();
}

void MPRIS2::Quit() {
  qApp->quit();
}

QString MPRIS2::PlaybackStatus() const {
  switch (player_->GetState()) {
    case Engine::Playing: return "Playing";
    case Engine::Paused:  return "Paused";
    default:              return "Stopped";
  }
}

QString MPRIS2::LoopStatus() const {
  switch (player_->GetPlaylists()->sequence()->repeat_mode()) {
    case PlaylistSequence::Repeat_Album:
    case PlaylistSequence::Repeat_Playlist: return "Playlist";
    case PlaylistSequence::Repeat_Track:    return "Track";
    default:                                return "None";
  }
}

void MPRIS2::SetLoopStatus(const QString& value) {
  if (value == "None") {
    player_->SetLoop(PlaylistSequence::Repeat_Off);
  } else if (value == "Track") {
    player_->SetLoop(PlaylistSequence::Repeat_Track);
  } else if (value == "Playlist") {
    player_->SetLoop(PlaylistSequence::Repeat_Playlist);
  }
  emitNotification("LoopStatus", value);
}

double MPRIS2::Rate() const {
  return 1.0;
}

void MPRIS2::SetRate(double) {
  // Do nothing
}

bool MPRIS2::Shuffle() const {
  return player_->GetPlaylists()->sequence()->shuffle_mode() !=
      PlaylistSequence::Shuffle_Off;
}

void MPRIS2::SetShuffle(bool value) {
  player_->SetRandom(value);
  emitNotification("Shuffle", value);
}

QVariantMap MPRIS2::Metadata() const {
  return last_metadata_;
}

void MPRIS2::UpdateMetadata(PlaylistItemPtr item) {
  last_metadata_ = QVariantMap();

  // Load the cover art
  art_request_item_ = item;
  art_request_id_ = cover_loader_->Worker()->LoadImageAsync(item->Metadata());
}

void MPRIS2::Initialised() {
  cover_loader_->Worker()->SetPadOutputImage(true);
  cover_loader_->Worker()->SetDefaultOutputImage(QImage(":nocover.png"));
  connect(cover_loader_->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(TempArtLoaded(quint64,QImage)));
}

void MPRIS2::TempArtLoaded(quint64 id, const QImage& image) {
  if (id != art_request_id_ || !art_request_item_)
    return;
  art_request_id_ = 0;
  last_metadata_ = QVariantMap();

  Song song = art_request_item_->Metadata();

  QString track_id = QString("/org/mpris/MediaPlayer2/Track/%1").
                     arg(QString::number(player_->GetCurrentTrack()));

  using metadata::AddMetadata;
  AddMetadata("mpris:trackid", track_id, &last_metadata_);
  AddMetadata("xesam:url", art_request_item_->Url().toString(), &last_metadata_);
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

  if (!image.isNull()) {
    temp_art_.reset(new QTemporaryFile(QDir::tempPath() + "/clementine-art-XXXXXX.jpg"));
    temp_art_->open();
    image.save(temp_art_->fileName(), "JPEG");
    AddMetadata("mpris:artUrl", "file://" + temp_art_->fileName(), &last_metadata_);
  }

  emitNotification("Metadata", last_metadata_);
  art_request_item_.reset();
}

double MPRIS2::Volume() const {
  return player_->VolumeGet() / 100;
}

void MPRIS2::SetVolume(double value) {
  player_->SetVolume(value * 100);
  emitNotification("Volume",value);
}

qlonglong MPRIS2::Position() const {
  return player_->PositionGet() * 1e6;
}

double MPRIS2::MaximumRate() const {
  return 1.0;
}

double MPRIS2::MinimumRate() const {
  return 1.0;
}

bool MPRIS2::CanGoNext() const {
  return true;
}

bool MPRIS2::CanGoPrevious() const {
  return true;
}

bool MPRIS2::CanPlay() const {
  return true;
}

bool MPRIS2::CanPause() const {
  return true;
}

bool MPRIS2::CanSeek() const {
  return true;
}

bool MPRIS2::CanControl() const {
  return true;
}

void MPRIS2::Next() {
  player_->Next();
  emitNotification("PlaybackStatus",PlaybackStatus());
  emitNotification("Metadata",Metadata());
}

void MPRIS2::Previous() {
  player_->Previous();
  emitNotification("PlaybackStatus",PlaybackStatus());
  emitNotification("Metadata",Metadata());
}

void MPRIS2::Pause() {
  player_->Pause();
  emitNotification("PlaybackStatus",PlaybackStatus());
  emitNotification("Metadata",Metadata());
}

void MPRIS2::PlayPause() {
  player_->PlayPause();
  emitNotification("PlaybackStatus",PlaybackStatus());
  emitNotification("Metadata",Metadata());
}

void MPRIS2::Stop() {
  player_->Stop();
  emitNotification("PlaybackStatus",PlaybackStatus());
  emitNotification("Metadata",Metadata());
}

void MPRIS2::Play() {
  player_->Play();
  emitNotification("PlaybackStatus",PlaybackStatus());
  emitNotification("Metadata",Metadata());
}

void MPRIS2::Seek(qlonglong offset) {
  player_->Seek(offset*1e6);
}

void MPRIS2::SetPosition(const QDBusObjectPath& trackId, qlonglong offset) {
  //TODO
}

void MPRIS2::OpenUri(const QString &uri) {
  player_->AddTrack(uri,true);
}

TrackIds MPRIS2::Tracks() const {
  //TODO
  return TrackIds();
}

bool MPRIS2::CanEditTracks() const {
  return false;
}

TrackMetadata MPRIS2::GetTracksMetadata(const TrackIds &tracks) const {
  //TODO
  return TrackMetadata();
}

void MPRIS2::AddTrack(const QString &uri, const QDBusObjectPath &afterTrack, bool setAsCurrent) {
  //TODO
}

void MPRIS2::RemoveTrack(const QDBusObjectPath &trackId) {
  //TODO
}

void MPRIS2::GoTo(const QDBusObjectPath &trackId) {
  //TODO
}
