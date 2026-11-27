/* This file is part of Clementine.
   Copyright 2012, Andreas Muttscheller <asfa194@gmail.com>

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

#include "outgoingdatacreator.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <cmath>

#include "core/database.h"
#include "core/logging.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "networkremote.h"
#include "networkremote/remotefiles.h"
#include "networkremote/sync/serverid.h"
#include "networkremote/sync/syncsong.h"

const quint32 OutgoingDataCreator::kFileChunkSize = 100000;  // in Bytes

namespace {

constexpr qint64 kMaxLibraryBufferedBytes = 512 * 1024;

// Detaches songs_export on scope exit after QSqlQuery completes so SQL errors
// cannot leave the temp database attached (risking SQLITE_BUSY or crashes).
struct ScopedDetach {
  explicit ScopedDetach(QSqlDatabase* database) : db(database), active(false) {}
  ~ScopedDetach() {
    if (!active) return;
    QSqlQuery detach(*db);
    if (!detach.exec("DETACH DATABASE songs_export;")) {
      qLog(Warning) << "Failed to detach songs_export:"
                    << detach.lastError().text();
    }
  }
  QSqlDatabase* db;
  bool active;
};

}  // namespace

OutgoingDataCreator::OutgoingDataCreator(Application* app)
    : app_(app),
      clients_(nullptr),
      current_row_(-1),
      current_downloadability_(cpb::remote::DOWNLOADABILITY_UNSPECIFIED),
      last_state_(Engine::Empty),
      keep_alive_timer_(new QTimer(this)),
      track_position_timer_(new QTimer(this)),
      keep_alive_timeout_(10000),
      last_track_position_(0),
      aww_(false),
      allow_downloads_(false),
      streaming_enabled_(false),
      sync_enabled_(false),
      lyrics_fetcher_(nullptr) {
  connect(keep_alive_timer_, SIGNAL(timeout()), this, SLOT(SendKeepAlive()));
  connect(track_position_timer_, SIGNAL(timeout()), this,
          SLOT(UpdateTrackPosition()));
}

OutgoingDataCreator::~OutgoingDataCreator() {}

void OutgoingDataCreator::SetClients(QList<RemoteClient*>* clients) {
  clients_ = clients;
  // After we got some clients, start the keep alive timer
  // Default: every 10 seconds
  keep_alive_timer_->start(keep_alive_timeout_);

  // Created here, in the remote's thread, where its replies arrive.
  if (!lyrics_fetcher_) {
    lyrics_fetcher_ = new LyricsFetcher(this);
    connect(lyrics_fetcher_, &LyricsFetcher::Finished, this,
            &OutgoingDataCreator::SendLyrics);
  }
}

RemoteClient* OutgoingDataCreator::FindClient(int client_id) const {
  if (!clients_) return nullptr;
  for (RemoteClient* client : *clients_) {
    if (client->id() == client_id) return client;
  }
  return nullptr;
}

void OutgoingDataCreator::SendDataToClient(int client_id,
                                           cpb::remote::Message* msg) {
  RemoteClient* client = FindClient(client_id);
  if (!client || client->is_closing() || client->isDownloader()) return;
  if (client->State() == QTcpSocket::ConnectedState) {
    client->SendData(msg);
  }
}

void OutgoingDataCreator::SendRawToClient(int client_id,
                                          const QByteArray& serialized_proto) {
  RemoteClient* client = FindClient(client_id);
  if (!client || client->is_closing() || client->isDownloader()) return;
  if (client->State() == QTcpSocket::ConnectedState) {
    client->SendRaw(serialized_proto);
  }
}

void OutgoingDataCreator::SendRawToAllClients(
    const QByteArray& serialized_proto) {
  if (!clients_ || clients_->empty()) return;

  // Snapshot clients_ because writing can trigger disconnects that mutate it.
  const QList<RemoteClient*> snapshot = *clients_;
  for (RemoteClient* client : snapshot) {
    if (!clients_->contains(client) || client->isDownloader() ||
        client->is_closing()) {
      continue;
    }
    if (client->State() == QTcpSocket::ConnectedState) {
      client->SendRaw(serialized_proto);
    }
  }
}

void OutgoingDataCreator::SendDataToClients(cpb::remote::Message* msg) {
  if (!clients_ || clients_->empty()) return;

  // Snapshot clients_ because writing can trigger disconnects that mutate it.
  const QList<RemoteClient*> snapshot = *clients_;
  for (RemoteClient* client : snapshot) {
    if (!clients_->contains(client) || client->isDownloader() ||
        client->is_closing()) {
      continue;
    }
    if (client->State() == QTcpSocket::ConnectedState) {
      client->SendData(msg);
    }
  }
}

void OutgoingDataCreator::SendClementineInfo(int client_id,
                                             Engine::State state) {
  // Create the general message and set the message type
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::INFO);

  // Now add the message specific data
  cpb::remote::ResponseClementineInfo* info =
      msg.mutable_response_clementine_info();
  SetEngineState(state, info);

  // allowed extensions for REQUEST_FILES and LIST_FILES
  for (const QString& ext : files_music_extensions_)
    info->add_files_music_extensions(ext.toStdString());

  QString version =
      QString("%1 %2").arg(QCoreApplication::applicationName(),
                           QCoreApplication::applicationVersion());
  info->set_version(version.toLatin1().toStdString());
  info->set_allow_downloads(allow_downloads_);
  info->add_features(cpb::remote::SERVER_FEATURE_BROWSE);
  info->add_features(cpb::remote::SERVER_FEATURE_ENQUEUE_NEXT);
  info->add_features(cpb::remote::SERVER_FEATURE_SMART_PLAYLISTS);
  if (streaming_enabled_) {
    info->add_features(cpb::remote::SERVER_FEATURE_RENDERING);
  }
  info->set_server_id(NetworkRemoteServerId().toStdString());
  if (sync_enabled_) {
    info->add_features(cpb::remote::SERVER_FEATURE_SYNC);
  }
  SendDataToClient(client_id, &msg);
}

void OutgoingDataCreator::SetEngineState(
    Engine::State state, cpb::remote::ResponseClementineInfo* msg) {
  switch (state) {
    case Engine::Idle:
      msg->set_state(cpb::remote::Idle);
      break;
    case Engine::Error:
    case Engine::Empty:
      msg->set_state(cpb::remote::Empty);
      break;
    case Engine::Playing:
      msg->set_state(cpb::remote::Playing);
      break;
    case Engine::Paused:
      msg->set_state(cpb::remote::Paused);
      break;
  }
}

void OutgoingDataCreator::SendFirstData(
    int client_id, const Song& current_song, int current_row,
    int current_downloadability, Engine::State state, int volume,
    qint64 position_nanosec, const QByteArray& playlists_pb,
    const QByteArray& playlist_songs_pb,
    PlaylistSequence::RepeatMode repeat_mode,
    PlaylistSequence::ShuffleMode shuffle_mode) {
  RemoteClient* client = FindClient(client_id);
  if (!client || client->is_closing() || client->isDownloader()) return;

  // Keep track of current song/state locally for kittens & position updates.
  const QImage art = (current_song.url() == current_song_.url() || aww_)
                         ? current_image_
                         : QImage();
  if (current_song.is_valid() || !current_song.url().isEmpty()) {
    current_song_ = current_song;
    current_row_ = current_row;
    current_downloadability_ = current_downloadability;
  }
  last_state_ = state;

  cpb::remote::Message meta_msg;
  meta_msg.set_type(cpb::remote::CURRENT_METAINFO);
  cpb::remote::SongMetadata* meta =
      meta_msg.mutable_response_current_metadata()->mutable_song_metadata();
  CreateSong(current_song, art, current_row, meta);
  if (NetworkRemote::MediaSyncEnabled() && meta->has_id() &&
      current_downloadability != cpb::remote::DOWNLOADABILITY_UNSPECIFIED) {
    meta->set_downloadable(
        static_cast<cpb::remote::Downloadability>(current_downloadability));
  }
  SendDataToClient(client_id, &meta_msg);

  cpb::remote::Message vol_msg;
  vol_msg.set_type(cpb::remote::SET_VOLUME);
  vol_msg.mutable_request_set_volume()->set_volume(volume);
  SendDataToClient(client_id, &vol_msg);

  if (!track_position_timer_->isActive() && state == Engine::Playing) {
    track_position_timer_->start(1000);
  }

  // Current track position (unicast to joining client)
  cpb::remote::Message pos_msg;
  pos_msg.set_type(cpb::remote::UPDATE_TRACK_POSITION);
  int position = static_cast<int>(std::floor(
      static_cast<double>(qMax<qint64>(0, position_nanosec)) / kNsecPerSec +
      0.5));
  // Check length_nanosec() > 0 so streams with unknown length (-1) aren't
  // clamped to last_track_position_.
  if (current_song.length_nanosec() > 0 &&
      position_nanosec > current_song.length_nanosec()) {
    position = last_track_position_;
  }
  pos_msg.mutable_response_update_track_position()->set_position(position);
  SendDataToClient(client_id, &pos_msg);

  if (!playlists_pb.isEmpty()) SendRawToClient(client_id, playlists_pb);
  if (!playlist_songs_pb.isEmpty())
    SendRawToClient(client_id, playlist_songs_pb);

  cpb::remote::Message shuffle_msg;
  FillShuffleMessage(shuffle_mode, &shuffle_msg);
  SendDataToClient(client_id, &shuffle_msg);

  cpb::remote::Message repeat_msg;
  FillRepeatMessage(repeat_mode, &repeat_msg);
  SendDataToClient(client_id, &repeat_msg);

  cpb::remote::Message done_msg;
  done_msg.set_type(cpb::remote::FIRST_DATA_SENT_COMPLETE);
  SendDataToClient(client_id, &done_msg);
}

void OutgoingDataCreator::CurrentSongChanged(const Song& song,
                                             const QString& uri,
                                             const QImage& img, int current_row,
                                             int downloadability) {
  current_song_ = song;
  current_uri_ = uri;
  current_row_ = current_row;
  current_downloadability_ = downloadability;

  if (!aww_) {
    current_image_ = img;
  }

  SendSongMetadata();
}

void OutgoingDataCreator::SendSongMetadata() {
  // Create the message
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::CURRENT_METAINFO);

  // If there is no song, create an empty node, otherwise fill it with data
  cpb::remote::SongMetadata* song_metadata =
      msg.mutable_response_current_metadata()->mutable_song_metadata();
  CreateSong(current_song_, current_image_, current_row_, song_metadata);
  if (NetworkRemote::MediaSyncEnabled() && song_metadata->has_id() &&
      current_downloadability_ != cpb::remote::DOWNLOADABILITY_UNSPECIFIED) {
    song_metadata->set_downloadable(
        static_cast<cpb::remote::Downloadability>(current_downloadability_));
  }

  SendDataToClients(&msg);
}

cpb::remote::Downloadability OutgoingDataCreator::DownloadabilityOf(
    const Song& song, const UrlHandler* handler) {
  switch (SyncSong::Downloadability(song, handler)) {
    case UrlHandler::Downloadability::Downloadable:
      return cpb::remote::DOWNLOADABILITY_DOWNLOADABLE;
    case UrlHandler::Downloadability::NotDownloadable:
      return cpb::remote::DOWNLOADABILITY_NOT_DOWNLOADABLE;
  }
  return cpb::remote::DOWNLOADABILITY_UNSPECIFIED;
}

void OutgoingDataCreator::CreateSong(const Song& song, const QImage& art,
                                     const int index,
                                     cpb::remote::SongMetadata* song_metadata) {
  // Even for a song that isn't valid (one whose file can't be read, say): the
  // remotes tell a playlist's songs apart, and pick one to play, by its index.
  song_metadata->set_index(index);
  // A song that isn't valid still has what the playlist shows of it, so the
  // remotes get that too. Only an empty song (none playing) has no fields: the
  // remotes take a song without an id for no song.
  if (song.is_valid() || !song.url().isEmpty()) {
    song_metadata->set_id(song.id());
    song_metadata->set_title(DataCommaSizeFromQString(song.PrettyTitle()));
    song_metadata->set_artist(DataCommaSizeFromQString(song.artist()));
    song_metadata->set_album(DataCommaSizeFromQString(song.album()));
    song_metadata->set_albumartist(
        DataCommaSizeFromQString(song.albumartist()));
    song_metadata->set_length(song.length_nanosec() / kNsecPerSec);
    song_metadata->set_pretty_length(
        DataCommaSizeFromQString(song.PrettyLength()));
    song_metadata->set_genre(DataCommaSizeFromQString(song.genre()));
    song_metadata->set_pretty_year(DataCommaSizeFromQString(song.PrettyYear()));
    song_metadata->set_track(song.track());
    song_metadata->set_disc(song.disc());
    song_metadata->set_playcount(song.playcount());
    song_metadata->set_is_local(song.url().scheme() == "file");
    song_metadata->set_filename(DataCommaSizeFromQString(song.basefilename()));
    song_metadata->set_file_size(song.filesize());
    song_metadata->set_rating(song.rating());
    song_metadata->set_url(DataCommaSizeFromQString(song.url().toString()));
    song_metadata->set_art_automatic(
        DataCommaSizeFromQString(song.art_automatic()));
    song_metadata->set_art_manual(DataCommaSizeFromQString(song.art_manual()));
    song_metadata->set_type(
        static_cast<::cpb::remote::SongMetadata_Type>(song.filetype()));

    // Append coverart
    if (!art.isNull()) {
      QImage small;
      // Check if we resize the image
      if (art.width() > 1000 || art.height() > 1000) {
        small = art.scaled(1000, 1000, Qt::KeepAspectRatio);
      } else {
        small = art;
      }

      // Read the image in a buffer and compress it
      QByteArray data;
      QBuffer buf(&data);
      buf.open(QIODevice::WriteOnly);
      small.save(&buf, "JPG");

      // Append the Data in the protocol buffer
      song_metadata->set_art(data.constData(), data.size());
    }
  }
}

void OutgoingDataCreator::VolumeChanged(int volume) {
  // Create the message
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::SET_VOLUME);
  msg.mutable_request_set_volume()->set_volume(volume);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::StateChanged(Engine::State state) {
  // Send state only if it changed
  // When selecting next song, StateChanged is emitted, but we already know
  // that we are playing
  if (state == last_state_) {
    return;
  }
  last_state_ = state;

  cpb::remote::Message msg;

  switch (state) {
    case Engine::Playing:
      msg.set_type(cpb::remote::PLAY);
      track_position_timer_->start(1000);
      break;
    case Engine::Paused:
      msg.set_type(cpb::remote::PAUSE);
      track_position_timer_->stop();
      break;
    case Engine::Empty:
      msg.set_type(cpb::remote::STOP);  // Empty is called when player stopped
      track_position_timer_->stop();
      break;
    default:
      msg.set_type(cpb::remote::STOP);
      track_position_timer_->stop();
      break;
  };

  SendDataToClients(&msg);
}

void OutgoingDataCreator::FillRepeatMessage(PlaylistSequence::RepeatMode mode,
                                            cpb::remote::Message* msg) {
  msg->set_type(cpb::remote::REPEAT);

  switch (mode) {
    case PlaylistSequence::Repeat_Off:
      msg->mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Off);
      break;
    case PlaylistSequence::Repeat_Track:
      msg->mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Track);
      break;
    case PlaylistSequence::Repeat_Album:
      msg->mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Album);
      break;
    case PlaylistSequence::Repeat_Playlist:
      msg->mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Playlist);
      break;
    case PlaylistSequence::Repeat_OneByOne:
      msg->mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_OneByOne);
      break;
    case PlaylistSequence::Repeat_Intro:
      msg->mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Intro);
      break;
  }
}

void OutgoingDataCreator::SendRepeatMode(PlaylistSequence::RepeatMode mode) {
  cpb::remote::Message msg;
  FillRepeatMessage(mode, &msg);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::FillShuffleMessage(PlaylistSequence::ShuffleMode mode,
                                             cpb::remote::Message* msg) {
  msg->set_type(cpb::remote::SHUFFLE);

  switch (mode) {
    case PlaylistSequence::Shuffle_Off:
      msg->mutable_shuffle()->set_shuffle_mode(cpb::remote::Shuffle_Off);
      break;
    case PlaylistSequence::Shuffle_All:
      msg->mutable_shuffle()->set_shuffle_mode(cpb::remote::Shuffle_All);
      break;
    case PlaylistSequence::Shuffle_InsideAlbum:
      msg->mutable_shuffle()->set_shuffle_mode(
          cpb::remote::Shuffle_InsideAlbum);
      break;
    case PlaylistSequence::Shuffle_Albums:
      msg->mutable_shuffle()->set_shuffle_mode(cpb::remote::Shuffle_Albums);
      break;
  }
}

void OutgoingDataCreator::SendShuffleMode(PlaylistSequence::ShuffleMode mode) {
  cpb::remote::Message msg;
  FillShuffleMessage(mode, &msg);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendKeepAlive() {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::KEEP_ALIVE);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::UpdateTrackPosition() {
  if (!clients_ || clients_->empty()) return;
  // Query the main-thread engine only while an active client is connected.
  for (RemoteClient* client : *clients_) {
    if (!client->isDownloader() && !client->is_closing() &&
        client->State() == QTcpSocket::ConnectedState) {
      emit RequestTrackPosition();
      return;
    }
  }
}

void OutgoingDataCreator::SendTrackPosition(qint64 position_nanosec) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::UPDATE_TRACK_POSITION);

  int position = static_cast<int>(std::floor(
      static_cast<double>(qMax<qint64>(0, position_nanosec)) / kNsecPerSec +
      0.5));

  // Skip clamping for streams with unknown length (-1).
  if (current_song_.length_nanosec() > 0 &&
      position_nanosec > current_song_.length_nanosec()) {
    position = last_track_position_;
  }

  msg.mutable_response_update_track_position()->set_position(position);

  last_track_position_ = position;

  SendDataToClients(&msg);
}

void OutgoingDataCreator::DisconnectAllClients() {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::DISCONNECT);
  msg.mutable_response_disconnect()->set_reason_disconnect(
      cpb::remote::Server_Shutdown);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::GetLyrics(int client_id) {
  if (!lyrics_fetcher_) return;
  const int req_id = lyrics_fetcher_->Fetch(current_song_);
  if (lyrics_requests_.size() > 64) lyrics_requests_.clear();
  lyrics_requests_.insert(req_id, client_id);
}

void OutgoingDataCreator::SendLyrics(int id, const Lyrics& lyrics) {
  const int client_id = lyrics_requests_.take(id);
  if (!client_id) return;

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::LYRICS);
  cpb::remote::ResponseLyrics* response = msg.mutable_response_lyrics();

  // No lyrics is an empty list.
  if (!lyrics.IsEmpty()) {
    const QString lyrics_id = "lyrics";
    const QString content =
        lyrics.instrumental ? tr("Instrumental") : lyrics.plain;
    cpb::remote::Lyric* lyric = response->mutable_lyrics()->Add();
    lyric->set_id(DataCommaSizeFromQString(lyrics_id));
    lyric->set_title(DataCommaSizeFromQString(lyrics.title));
    lyric->set_content(DataCommaSizeFromQString(content));
  }
  SendDataToClient(client_id, &msg);
}

void OutgoingDataCreator::SendLibrary(int client_id) {
  RemoteClient* client = FindClient(client_id);
  if (!client || client->is_closing()) return;

  // Get a temporary file name
  const QString temp_file_name = Utilities::GetTemporaryFileName();
  bool export_ok = false;

  {
    // Serialize access to the shared DB connection across attach/export/detach.
    QMutexLocker l(app_->database()->Mutex());
    QSqlDatabase db(app_->database()->Connect());
    ScopedDetach detach_guard(&db);

    QSqlQuery attach(db);
    attach.prepare("ATTACH DATABASE :filename AS songs_export");
    attach.bindValue(":filename", temp_file_name);
    if (!attach.exec()) {
      qLog(Warning) << "Failed to attach songs_export database:"
                    << attach.lastError().text();
    } else {
      detach_guard.active = true;
      QSqlQuery q(db);
      if (!q.exec("CREATE TABLE songs_export.songs AS SELECT * FROM songs "
                  "WHERE unavailable = 0;")) {
        qLog(Warning) << "Failed to export songs table:"
                      << q.lastError().text();
      } else {
        export_ok = true;
      }
    }
  }

  if (!export_ok) {
    QFile::remove(temp_file_name);
    // Disconnect instead of returning silently so mobile downloaders don't
    // hang waiting for chunks.
    client->DisconnectClient(cpb::remote::Server_Shutdown);
    return;
  }

  // Compute sha1 while file is closed (Sha1File opens and closes it).
  QFile sha_file(temp_file_name);
  const QByteArray sha1 = Utilities::Sha1File(sha_file).toHex();
  qLog(Debug) << "Library sha1" << sha1;

  // Tie temp-file cleanup to file destruction so disconnects free it.
  QFile* file = new QFile(temp_file_name, client);
  connect(file, &QObject::destroyed,
          [temp_file_name]() { QFile::remove(temp_file_name); });

  if (!file->open(QIODevice::ReadOnly) || file->size() <= 0) {
    qLog(Warning) << "Failed to open exported library file" << temp_file_name;
    file->close();
    file->deleteLater();
    client->DisconnectClient(cpb::remote::Server_Shutdown);
    return;
  }

  const qint64 file_size = file->size();
  // Round up with integer division; exact kFileChunkSize multiples must not
  // emit an extra phantom chunk.
  const int chunk_count =
      static_cast<int>((file_size + kFileChunkSize - 1) / kFileChunkSize);
  auto chunk_number = std::make_shared<int>(1);

  // Stream chunks via BytesWritten with bounded buffering instead of
  // queuing the entire SQLite export synchronously in socket memory.
  auto pump = [client, file, sha1, file_size, chunk_count, chunk_number]() {
    if (!file->isOpen()) return;
    if (client->is_closing() || client->State() != QTcpSocket::ConnectedState) {
      file->close();
      file->deleteLater();
      return;
    }

    cpb::remote::Message msg;
    cpb::remote::ResponseLibraryChunk* chunk =
        msg.mutable_response_library_chunk();
    msg.set_type(cpb::remote::LIBRARY_CHUNK);

    while (file->isOpen() && !file->atEnd() &&
           client->bytesToWrite() < kMaxLibraryBufferedBytes) {
      const QByteArray data = file->read(kFileChunkSize);
      if (data.isEmpty()) break;

      chunk->set_chunk_count(chunk_count);
      chunk->set_chunk_number(*chunk_number);
      chunk->set_size(file_size);
      chunk->set_data(data.data(), data.size());
      chunk->set_file_hash(sha1.data(), sha1.size());

      client->SendData(&msg);
      chunk->Clear();
      (*chunk_number)++;
    }

    if (!file->isOpen() || file->atEnd() || file->error() != QFile::NoError) {
      file->close();
      file->deleteLater();
    }
  };

  connect(client, &RemoteClient::BytesWritten, file, pump);
  pump();
}

void OutgoingDataCreator::EnableKittens(bool aww) { aww_ = aww; }

void OutgoingDataCreator::SendKitten(const QImage& kitten) {
  if (aww_) {
    current_image_ = kitten;
    SendSongMetadata();
  }
}

void OutgoingDataCreator::SendListFiles(QString relative_path, int client_id) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::LIST_FILES);
  cpb::remote::ResponseListFiles* files = msg.mutable_response_list_files();

  const RemoteFiles::DirResult result =
      RemoteFiles::ResolveDir(files_root_folder_, relative_path);
  switch (result.status) {
    case RemoteFiles::DirStatus::RootNotSet:
      files->set_error(cpb::remote::ResponseListFiles::ROOT_DIR_NOT_SET);
      break;
    case RemoteFiles::DirStatus::NotAccessible:
      files->set_error(cpb::remote::ResponseListFiles::DIR_NOT_ACCESSIBLE);
      break;
    case RemoteFiles::DirStatus::NotExist:
      files->set_error(cpb::remote::ResponseListFiles::DIR_NOT_EXIST);
      break;
    case RemoteFiles::DirStatus::Ok: {
      files->set_relative_path(result.relative_path.toStdString());
      QDir dir(result.dir);
      dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
      dir.setSorting(QDir::Name | QDir::DirsFirst);

      for (const QFileInfo& fi : dir.entryInfoList()) {
        if (fi.isDir() || files_music_extensions_.contains(
                              fi.suffix(), Qt::CaseInsensitive)) {
          cpb::remote::FileMetadata* pb_file = files->add_files();
          pb_file->set_is_dir(fi.isDir());
          pb_file->set_filename(fi.fileName().toStdString());
        }
      }
      break;
    }
  }
  SendDataToClient(client_id, &msg);
}
