/* This file is part of Clementine.

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

#include "player.h"
#include "playlist.h"
#include "lastfmservice.h"
#include "engines/enginebase.h"

#ifdef HAVE_GSTREAMER
#  include "engines/gstengine.h"
#endif
#ifdef HAVE_LIBVLC
#  include "engines/vlcengine.h"
#endif
#ifdef HAVE_LIBXINE
#  include "engines/xine-engine.h"
#endif
#ifdef HAVE_QT_PHONON
#  include "engines/phononengine.h"
#endif

#ifdef Q_WS_X11
#  include "mpris_player.h"
#  include "mpris_tracklist.h"
#  include <QDBusConnection>
#endif

#include <QtDebug>
#include <QtConcurrentRun>

#include <boost/bind.hpp>

using boost::shared_ptr;

#ifdef Q_WS_X11
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
#endif

Player::Player(Playlist* playlist, LastFMService* lastfm, Engine::Type engine, QObject* parent)
  : QObject(parent),
    playlist_(playlist),
    lastfm_(lastfm),
    current_item_options_(PlaylistItem::Default),
    stream_change_type_(Engine::First)
{
  engine_ = createEngine(engine);

  settings_.beginGroup("Player");

  SetVolume(settings_.value("volume", 50).toInt());

  connect(engine_, SIGNAL(Error(QString)), SIGNAL(Error(QString)));

  // MPRIS DBus interface.
#ifdef Q_WS_X11
  MprisPlayer* mpris = new MprisPlayer(this);
  // Hack so the next registerObject() doesn't override this one.
  QDBusConnection::sessionBus().registerObject(
      "/Player", mpris, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
  new MprisTrackList(this);
  QDBusConnection::sessionBus().registerObject("/TrackList", this);
#endif
}

EngineBase* Player::createEngine(Engine::Type engine) {

  switch(engine) {
#ifdef HAVE_GSTREAMER
    case Engine::gstreamer:
      return new GstEngine();
      break;
#endif
#ifdef HAVE_LIBVLC
    case Engine::vlc:
      return new VlcEngine();
      break;
#endif
#ifdef HAVE_LIBXINE
    case Engine::xine:
      return new XineEngine();
      break;
#endif
#ifdef HAVE_QT_PHONON
    case Engine::qt_phonon:
      return new PhononEngine();
      break;
#endif
    default:
      qFatal("Selected engine not compiled in");
      break;
  }
  /* NOT REACHED */
  return NULL;
}

void Player::Init() {
  if (!engine_->Init())
    qFatal("Error initialising audio engine");

  connect(engine_, SIGNAL(StateChanged(Engine::State)), SLOT(EngineStateChanged(Engine::State)));
  connect(engine_, SIGNAL(TrackAboutToEnd()), SLOT(TrackAboutToEnd()));
  connect(engine_, SIGNAL(TrackEnded()), SLOT(TrackEnded()));
  connect(engine_, SIGNAL(MetaData(Engine::SimpleMetaBundle)),
                   SLOT(EngineMetadataReceived(Engine::SimpleMetaBundle)));

  engine_->SetVolume(settings_.value("volume", 50).toInt());
}

void Player::ReloadSettings() {
  engine_->ReloadSettings();
}

void Player::RadioStreamFinished() {
  NextItem(Engine::Auto);
}

void Player::Next() {
  NextInternal(Engine::Manual);
}

void Player::NextInternal(Engine::TrackChangeType change) {
  if (playlist_->current_item_options() & PlaylistItem::ContainsMultipleTracks) {
    stream_change_type_ = change;
    playlist_->current_item()->LoadNext();
    return;
  }

  NextItem(Engine::Manual);
}

void Player::NextItem(Engine::TrackChangeType change) {
  int i = playlist_->next_index();
  playlist_->set_current_index(i);
  if (i == -1) {
    emit PlaylistFinished();
    Stop();
    return;
  }

  PlayAt(i, change, false);
}

void Player::TrackEnded() {
  if (playlist_->stop_after_current()) {
    Stop();
    return;
  }

  NextInternal(Engine::Auto);
}

void Player::PlayPause() {
  switch (engine_->state()) {
  case Engine::Paused:
    qDebug() << "Unpausing";
    engine_->Unpause();
    break;

  case Engine::Playing:
    // We really shouldn't pause last.fm streams
    if (current_item_options_ & PlaylistItem::PauseDisabled)
      break;

    qDebug() << "Pausing";
    engine_->Pause();
    break;

  case Engine::Empty:
  case Engine::Idle: {
    if (playlist_->rowCount() == 0)
      break;

             int i = playlist_->current_index();
    if (i == -1) i = playlist_->last_played_index();
    if (i == -1) i = 0;

    PlayAt(i, Engine::First, true);
    break;
  }
  }
}

void Player::Stop() {
  engine_->Stop();
  playlist_->set_current_index(-1);
}

void Player::Previous() {
  int i = playlist_->previous_index();
  playlist_->set_current_index(i);
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
  emit StatusChange(GetStatus());
  emit CapsChange(GetCaps());
}

void Player::SetVolume(int value) {
  int old_volume = engine_->volume();

  int volume = qBound(0, value, 100);
  settings_.setValue("volume", volume);
  engine_->SetVolume(volume);

  if (volume != old_volume)
    emit VolumeChanged(volume);
}

int Player::GetVolume() const {
  return engine_->volume();
}

Engine::State Player::GetState() const {
  return engine_->state();
}

void Player::PlayAt(int index, Engine::TrackChangeType change, bool reshuffle) {
  if (reshuffle)
    playlist_->set_current_index(-1);
  playlist_->set_current_index(index);

  shared_ptr<PlaylistItem> item = playlist_->item_at(index);
  current_item_options_ = item->options();
  current_item_ = item->Metadata();

  if (item->options() & PlaylistItem::SpecialPlayBehaviour)
    item->StartLoading();
  else {
    engine_->Play(item->Url(), change);

    if (lastfm_->IsScrobblingEnabled())
      lastfm_->NowPlaying(item->Metadata());
  }

  emit CapsChange(GetCaps());
}

void Player::StreamReady(const QUrl& original_url, const QUrl& media_url) {
  int current_index = playlist_->current_index();
  if (current_index == -1)
    return;

  shared_ptr<PlaylistItem> item = playlist_->item_at(current_index);
  if (!item || item->Url() != original_url)
    return;

  engine_->Play(media_url, stream_change_type_);

  current_item_ = item->Metadata();
  current_item_options_ = item->options();
}

void Player::CurrentMetadataChanged(const Song &metadata) {
  lastfm_->NowPlaying(metadata);
  current_item_ = metadata;
  emit TrackChange(GetMetadata());
}

void Player::Seek(int seconds) {
  int msec = qBound(0, seconds * 1000, int(engine_->length()));
  engine_->Seek(msec);

  // If we seek the track we don't want to submit it to last.fm
  playlist_->set_scrobbled(true);
}

void Player::EngineMetadataReceived(const Engine::SimpleMetaBundle& bundle) {
  shared_ptr<PlaylistItem> item = playlist_->current_item();
  if (item == NULL)
    return;

  Song song = item->Metadata();
  song.MergeFromSimpleMetaBundle(bundle);

  // Ignore useless metadata
  if (song.title().isEmpty() && song.artist().isEmpty())
    return;

  playlist_->SetStreamMetadata(item->Url(), song);
}

int Player::GetCaps() const {
  int caps = CAN_HAS_TRACKLIST;
  if (current_item_.is_valid()) { caps |= CAN_PROVIDE_METADATA; }
  if (GetState() == Engine::Playing && current_item_options_ & PlaylistItem::PauseDisabled) {
    caps |= CAN_PAUSE;
  }
  if (GetState() == Engine::Paused) {
    caps |= CAN_PLAY;
  }
  if (GetState() != Engine::Empty && current_item_.filetype() != Song::Type_Stream) {
    caps |= CAN_SEEK;
  }
  if (playlist_->next_index() != -1 ||
      playlist_->current_item_options() & PlaylistItem::ContainsMultipleTracks) {
    caps |= CAN_GO_NEXT;
  }
  if (playlist_->previous_index() != -1) {
    caps |= CAN_GO_PREV;
  }
  return caps;
}

DBusStatus Player::GetStatus() const {
  DBusStatus status;
  switch (GetState()) {
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
  status.random = playlist_->sequence()->shuffle_mode() == PlaylistSequence::Shuffle_Off ? 0 : 1;
  PlaylistSequence::RepeatMode repeat_mode = playlist_->sequence()->repeat_mode();
  status.repeat = repeat_mode == PlaylistSequence::Repeat_Track ? 1 : 0;
  status.repeat_playlist = (repeat_mode == PlaylistSequence::Repeat_Album ||
                           repeat_mode == PlaylistSequence::Repeat_Playlist) ? 1 : 0;
  return status;
}

namespace {
inline void AddMetadata(const QString& key, const QString& metadata, QVariantMap* map) {
  if (!metadata.isEmpty()) {
    (*map)[key] = metadata;
  }
}

inline void AddMetadata(const QString& key, int metadata, QVariantMap* map) {
  if (metadata > 0) {
    (*map)[key] = metadata;
  }
}

}  // namespace

QVariantMap Player::GetMetadata(const PlaylistItem& item) const {
  QVariantMap ret;

  const Song& song = item.Metadata();
  AddMetadata("location", item.Url().toString(), &ret);
  AddMetadata("title", song.PrettyTitle(), &ret);
  AddMetadata("artist", song.artist(), &ret);
  AddMetadata("album", song.album(), &ret);
  AddMetadata("time", song.length(), &ret);
  AddMetadata("tracknumber", song.track(), &ret);

  return ret;
}

QVariantMap Player::GetMetadata() const {
  shared_ptr<PlaylistItem> item = playlist_->current_item();
  if (item) {
    return GetMetadata(*item);
  }
  return QVariantMap();
}

QVariantMap Player::GetMetadata(int track) const {
  if (track >= playlist_->rowCount() || track < 0) {
    return QVariantMap();
  }
  const PlaylistItem& item = *(playlist_->item_at(track));
  return GetMetadata(item);
}

void Player::Mute() {
  SetVolume(0);
}

void Player::Pause() {
  switch (GetState()) {
    case Engine::Playing:
      engine_->Pause();
      break;
    case Engine::Paused:
      engine_->Pause();
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

void Player::Prev() {
  Previous();
}

int Player::PositionGet() const {
  return engine_->position();
}

void Player::PositionSet(int x) {
  Seek(x / 1000);
}

void Player::Repeat(bool enable) {
  playlist_->sequence()->SetRepeatMode(
      enable ? PlaylistSequence::Repeat_Track : PlaylistSequence::Repeat_Off);
}

void Player::ShowOSD() {
  emit ForceShowOSD(current_item_);
}

void Player::VolumeDown(int change) {
  SetVolume(GetVolume() - change);
}

void Player::VolumeUp(int change) {
  SetVolume(GetVolume() + change);
}

int Player::VolumeGet() const {
  return GetVolume();
}

void Player::VolumeSet(int volume) {
  SetVolume(volume);
}

int Player::AddTrack(const QString& track, bool play_now) {
  QUrl url(track);
  QList<QUrl> list;
  list << url;
  QModelIndex index;
  if (url.scheme() == "file") {
    index = playlist_->InsertPaths(list, play_now ? playlist_->current_index() + 1 : -1);
  } else {
    index = playlist_->InsertStreamUrls(list, play_now ? playlist_->current_index() + 1: -1);
  }

  if (index.isValid()) {
    if (play_now) {
      PlayAt(index.row(), Engine::First, true);
    }
    return 0;  // Success.
  }
  return -1;  // Anything else for failure.
}

void Player::DelTrack(int index) {
  playlist_->removeRows(index, 1);
}

int Player::GetCurrentTrack() const {
  return playlist_->current_index();
}

int Player::GetLength() const {
  return playlist_->rowCount();
}

void Player::SetLoop(bool enable) {
  playlist_->sequence()->SetRepeatMode(
      enable ? PlaylistSequence::Repeat_Playlist : PlaylistSequence::Repeat_Off);
}

void Player::SetRandom(bool enable) {
  playlist_->sequence()->SetShuffleMode(
      enable ? PlaylistSequence::Shuffle_All : PlaylistSequence::Shuffle_Off);
}

void Player::PlayTrack(int index) {
  PlayAt(index, Engine::Manual, true);
}

void Player::PlaylistChanged() {
  emit TrackListChange(GetLength());
}

void Player::TrackAboutToEnd() {
  if (engine_->is_autocrossfade_enabled()) {
    // Crossfade is on, so just start playing the next track.  The current one
    // will fade out, and the new one will fade in
    NextInternal(Engine::Auto);
  } else {
    // Crossfade is off, so start preloading the next track so we don't get a
    // gap between songs.
    if (current_item_options_ & PlaylistItem::ContainsMultipleTracks)
      return;
    if (playlist_->next_index() == -1)
      return;

    shared_ptr<PlaylistItem> item = playlist_->item_at(playlist_->next_index());
    if (!item)
      return;

    engine_->StartPreloading(item->Url());
  }
}
