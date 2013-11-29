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

#include <cmath>

#include "networkremote.h"
#include "core/logging.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "library/librarybackend.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include "core/database.h"

const quint32 OutgoingDataCreator::kFileChunkSize = 100000; // in Bytes

OutgoingDataCreator::OutgoingDataCreator(Application* app)
  : app_(app),
    ultimate_reader_(new UltimateLyricsReader(this)),
    fetcher_(new SongInfoFetcher(this))
{
  // Create Keep Alive Timer
  keep_alive_timer_ = new QTimer(this);
  connect(keep_alive_timer_, SIGNAL(timeout()), this, SLOT(SendKeepAlive()));
  keep_alive_timeout_ = 10000;
}

OutgoingDataCreator::~OutgoingDataCreator() {
}

void OutgoingDataCreator::SetClients(QList<RemoteClient*>* clients) {
  clients_ = clients;
  // After we got some clients, start the keep alive timer
  // Default: every 10 seconds
  keep_alive_timer_->start(keep_alive_timeout_);

  // Create the song position timer
  track_position_timer_ = new QTimer(this);
  connect(track_position_timer_, SIGNAL(timeout()), this, SLOT(UpdateTrackPosition()));

  // Parse the ultimate lyrics xml file
  ultimate_reader_->SetThread(this->thread());
  provider_list_ = ultimate_reader_->Parse(":lyrics/ultimate_providers.xml");

  // Set up the lyrics parser
  connect(fetcher_, SIGNAL(ResultReady(int,SongInfoFetcher::Result)),
          SLOT(SendLyrics(int,SongInfoFetcher::Result)));

  foreach (SongInfoProvider* provider, provider_list_) {
    fetcher_->AddProvider(provider);
  }

  CheckEnabledProviders();
}

void OutgoingDataCreator::CheckEnabledProviders() {
  QSettings s;
  s.beginGroup(SongInfoView::kSettingsGroup);

  // Put the providers in the right order
  QList<SongInfoProvider*> ordered_providers;

  QVariantList default_order;
  default_order << "lyrics.wikia.com"
                << "lyricstime.com"
                << "lyricsreg.com"
                << "lyricsmania.com"
                << "metrolyrics.com"
                << "azlyrics.com"
                << "songlyrics.com"
                << "elyrics.net"
                << "lyricsdownload.com"
                << "lyrics.com"
                << "lyricsbay.com"
                << "directlyrics.com"
                << "loudson.gs"
                << "teksty.org"
                << "tekstowo.pl (Polish translations)"
                << "vagalume.uol.com.br"
                << "vagalume.uol.com.br (Portuguese translations)"
                << "darklyrics.com";

  QVariant saved_order = s.value("search_order", default_order);
  foreach (const QVariant& name, saved_order.toList()) {
    SongInfoProvider* provider = ProviderByName(name.toString());
    if (provider)
      ordered_providers << provider;
  }

  // Enable all the providers in the list and rank them
  int relevance = 100;
  foreach (SongInfoProvider* provider, ordered_providers) {
    provider->set_enabled(true);
    qobject_cast<UltimateLyricsProvider*>(provider)->set_relevance(relevance--);
  }

  // Any lyric providers we don't have in ordered_providers are considered disabled
  foreach (SongInfoProvider* provider, fetcher_->providers()) {
    if (qobject_cast<UltimateLyricsProvider*>(provider) && !ordered_providers.contains(provider)) {
      provider->set_enabled(false);
    }
  }
}

SongInfoProvider* OutgoingDataCreator::ProviderByName(const QString& name) const {
  foreach (SongInfoProvider* provider, fetcher_->providers()) {
    if (UltimateLyricsProvider* lyrics = qobject_cast<UltimateLyricsProvider*>(provider)) {
      if (lyrics->name() == name)
        return provider;
    }
  }
  return NULL;
}

void OutgoingDataCreator::SendDataToClients(pb::remote::Message* msg) {
  // Check if we have clients to send data to
  if (clients_->empty()) {
    return;
  }

  RemoteClient* client;
  foreach(client, *clients_) {
    // Do not send data to downloaders
    if (client->isDownloader()) {
      if (client->State() != QTcpSocket::ConnectedState) {
        clients_->removeAt(clients_->indexOf(client));
        download_queue_.remove(client);
        delete client;
      }
      continue;
    }

    // Check if the client is still active
    if (client->State() == QTcpSocket::ConnectedState) {
      client->SendData(msg);
    } else {
      clients_->removeAt(clients_->indexOf(client));
      delete client;
    }
  }
}

void OutgoingDataCreator::SendClementineInfo() {
  // Create the general message and set the message type
  pb::remote::Message msg;
  msg.set_type(pb::remote::INFO);

  // Now add the message specific data
  pb::remote::ResponseClementineInfo* info =
      msg.mutable_response_clementine_info();
  SetEngineState(info);

  QString version = QString("%1 %2").arg(QCoreApplication::applicationName(),
                                         QCoreApplication::applicationVersion());
  info->set_version(version.toAscii());
  SendDataToClients(&msg);
}

void OutgoingDataCreator::SetEngineState(pb::remote::ResponseClementineInfo* msg) {
  switch(app_->player()->GetState()) {
    case Engine::Idle:    msg->set_state(pb::remote::Idle);
                          break;
    case Engine::Empty:   msg->set_state(pb::remote::Empty);
                          break;
    case Engine::Playing: msg->set_state(pb::remote::Playing);
                          break;
    case Engine::Paused:  msg->set_state(pb::remote::Paused);
                          break;
  }
}

void OutgoingDataCreator::SendAllPlaylists() {
  // Get all Playlists
  QList<Playlist*> app_playlists = app_->playlist_manager()->GetAllPlaylists();
  int active_playlist = app_->playlist_manager()->active_id();

  // Create message
  pb::remote::Message msg;
  msg.set_type(pb::remote::PLAYLISTS);

  pb::remote::ResponsePlaylists* playlists = msg.mutable_response_playlists();

  // Get all playlists, even ones that are hidden in the UI.
  foreach (const PlaylistBackend::Playlist& p,
           app_->playlist_backend()->GetAllPlaylists()) {
    bool playlist_open = app_->playlist_manager()->IsPlaylistOpen(p.id);
    int item_count = playlist_open ?
                        app_playlists.at(p.id)->rowCount() : 0;

    // Create a new playlist
    pb::remote::Playlist* playlist = playlists->add_playlist();
    playlist->set_name(DataCommaSizeFromQString(p.name));
    playlist->set_id(p.id);
    playlist->set_active((p.id == active_playlist));
    playlist->set_item_count(item_count);
    playlist->set_closed(!playlist_open);
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendAllActivePlaylists() {
  // Get all Playlists
  QList<Playlist*> app_playlists = app_->playlist_manager()->GetAllPlaylists();
  int active_playlist = app_->playlist_manager()->active_id();

  // Create message
  pb::remote::Message msg;
  msg.set_type(pb::remote::PLAYLISTS);

  pb::remote::ResponsePlaylists* playlists = msg.mutable_response_playlists();

  QListIterator<Playlist*> it(app_playlists);
  while(it.hasNext()) {
    // Get the next Playlist
    Playlist* p = it.next();
    QString playlist_name = app_->playlist_manager()->GetPlaylistName(p->id());

    // Create a new playlist
    pb::remote::Playlist* playlist = playlists->add_playlist();
    playlist->set_name(DataCommaSizeFromQString(playlist_name));
    playlist->set_id(p->id());
    playlist->set_active((p->id() == active_playlist));
    playlist->set_item_count(p->rowCount());
    playlist->set_closed(false);
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::ActiveChanged(Playlist* playlist) {
  // Send the tracks of the active playlist
  SendPlaylistSongs(playlist->id());

  // Send the changed message after sending the playlist songs
  pb::remote::Message msg;
  msg.set_type(pb::remote::ACTIVE_PLAYLIST_CHANGED);
  msg.mutable_response_active_changed()->set_id(playlist->id());
  SendDataToClients(&msg);
}

void OutgoingDataCreator::PlaylistAdded(int id, const QString& name, bool favorite) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::PlaylistDeleted(int id) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::PlaylistClosed(int id) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::PlaylistRenamed(int id, const QString& new_name) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::SendFirstData(bool send_playlist_songs) {
  // First Send the current song
  PlaylistItemPtr item = app_->player()->GetCurrentItem();
  if (!item) {
    qLog(Info) << "No current item found!";
  }

  CurrentSongChanged(current_song_, current_uri_, current_image_);

  // then the current volume
  VolumeChanged(app_->player()->GetVolume());

  // Check if we need to start the track position timer
  if (!track_position_timer_->isActive() &&
      app_->player()->engine()->state() == Engine::Playing) {
    track_position_timer_->start(1000);
  }

  // And the current track position
  UpdateTrackPosition();

  // And the current playlists
  SendAllActivePlaylists();

  // Send the tracks of the active playlist
  if (send_playlist_songs) {
    SendPlaylistSongs(app_->playlist_manager()->active_id());
  }

  // Send the current random and repeat mode
  SendShuffleMode(app_->playlist_manager()->sequence()->shuffle_mode());
  SendRepeatMode(app_->playlist_manager()->sequence()->repeat_mode());

  // We send all first data
  pb::remote::Message msg;
  msg.set_type(pb::remote::FIRST_DATA_SENT_COMPLETE);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::CurrentSongChanged(const Song& song, const QString& uri, const QImage& img) {
  current_song_  = song;
  current_uri_   = uri;
  current_image_ = img;

  if (!clients_->empty()) {
    // Create the message
    pb::remote::Message msg;
    msg.set_type(pb::remote::CURRENT_METAINFO);

    // If there is no song, create an empty node, otherwise fill it with data
    int i = app_->playlist_manager()->active()->current_row();
    CreateSong(
        current_song_, img, i,
        msg.mutable_response_current_metadata()->mutable_song_metadata());

    SendDataToClients(&msg);
  }
}

void OutgoingDataCreator::CreateSong(
    const Song& song,
    const QImage& art,
    const int index,
    pb::remote::SongMetadata* song_metadata) {
  if (song.is_valid()) {
    song_metadata->set_id(song.id());
    song_metadata->set_index(index);
    song_metadata->set_title(DataCommaSizeFromQString(song.PrettyTitle()));
    song_metadata->set_artist(DataCommaSizeFromQString(song.artist()));
    song_metadata->set_album(DataCommaSizeFromQString(song.album()));
    song_metadata->set_albumartist(DataCommaSizeFromQString(song.albumartist()));
    song_metadata->set_length(song.length_nanosec() / kNsecPerSec);
    song_metadata->set_pretty_length(DataCommaSizeFromQString(song.PrettyLength()));
    song_metadata->set_genre(DataCommaSizeFromQString(song.genre()));
    song_metadata->set_pretty_year(DataCommaSizeFromQString(song.PrettyYear()));
    song_metadata->set_track(song.track());
    song_metadata->set_disc(song.disc());
    song_metadata->set_playcount(song.playcount());
    song_metadata->set_is_local(song.url().scheme() == "file");
    song_metadata->set_filename(DataCommaSizeFromQString(song.basefilename()));
    song_metadata->set_file_size(song.filesize());
    song_metadata->set_rating(song.rating());

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
  pb::remote::Message msg;
  msg.set_type(pb::remote::SET_VOLUME);
  msg.mutable_request_set_volume()->set_volume(volume);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendPlaylistSongs(int id) {
  // Get the PlaylistQByteArray(data.data(), data.size()
  Playlist* playlist = app_->playlist_manager()->playlist(id);
  if(!playlist) {
    qLog(Info) << "Could not find playlist with id = " << id;
    return;
  }

  // Create the message and the playlist
  pb::remote::Message msg;
  msg.set_type(pb::remote::PLAYLIST_SONGS);

  // Create the Response message
  pb::remote::ResponsePlaylistSongs* pb_response_playlist_songs =
                                     msg.mutable_response_playlist_songs();

  // Create a new playlist
  pb::remote::Playlist* pb_playlist =
      pb_response_playlist_songs->mutable_requested_playlist();
  pb_playlist->set_id(id);

  // Send all songs
  int index = 0;
  SongList song_list = playlist->GetAllSongs();
  QListIterator<Song> it(song_list);
  QImage null_img;
  while(it.hasNext()) {
    Song song   = it.next();
    pb::remote::SongMetadata* pb_song = pb_response_playlist_songs->add_songs();
    CreateSong(song, null_img, index, pb_song);
    ++index;
  }
  SendDataToClients(&msg);
}

void OutgoingDataCreator::PlaylistChanged(Playlist* playlist) {
  // If a playlist changed, then send the new songs to the client
  SendPlaylistSongs(playlist->id());
}

void OutgoingDataCreator::StateChanged(Engine::State state) {
  // Send state only if it changed
  // When selecting next song, StateChanged is emitted, but we already know
  // that we are playing
  if (state == last_state_) {
    return;
  }
  last_state_ = state;

  pb::remote::Message msg;

  switch (state) {
  case Engine::Playing: msg.set_type(pb::remote::PLAY);
                        track_position_timer_->start(1000);
                        break;
  case Engine::Paused:  msg.set_type(pb::remote::PAUSE);
                        track_position_timer_->stop();
                        break;
  case Engine::Empty:   msg.set_type(pb::remote::STOP); // Empty is called when player stopped
                        track_position_timer_->stop();
                        break;
  default:              msg.set_type(pb::remote::STOP);
                        track_position_timer_->stop();
                        break;
  };

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendRepeatMode(PlaylistSequence::RepeatMode mode) {
  pb::remote::Message msg;
  msg.set_type(pb::remote::REPEAT);

  switch (mode) {
  case PlaylistSequence::Repeat_Off:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Off);
       break;
  case PlaylistSequence::Repeat_Track:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Track);
       break;
  case PlaylistSequence::Repeat_Album:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Album);
       break;
  case PlaylistSequence::Repeat_Playlist:
       msg.mutable_repeat()->set_repeat_mode(pb::remote::Repeat_Playlist);
       break;
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendShuffleMode(PlaylistSequence::ShuffleMode mode) {
  pb::remote::Message msg;
  msg.set_type(pb::remote::SHUFFLE);

  switch (mode) {
  case PlaylistSequence::Shuffle_Off:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_Off);
       break;
  case PlaylistSequence::Shuffle_All:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_All);
       break;
  case PlaylistSequence::Shuffle_InsideAlbum:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_InsideAlbum);
       break;
  case PlaylistSequence::Shuffle_Albums:
       msg.mutable_shuffle()->set_shuffle_mode(pb::remote::Shuffle_Albums);
       break;
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendKeepAlive() {
  pb::remote::Message msg;
  msg.set_type(pb::remote::KEEP_ALIVE);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::UpdateTrackPosition() {
  pb::remote::Message msg;
  msg.set_type(pb::remote::UPDATE_TRACK_POSITION);

  const int position = std::floor(
    float(app_->player()->engine()->position_nanosec()) / kNsecPerSec + 0.5);

  msg.mutable_response_update_track_position()->set_position(position);

  SendDataToClients(&msg);
}

void OutgoingDataCreator::DisconnectAllClients() {
  pb::remote::Message msg;
  msg.set_type(pb::remote::DISCONNECT);
  msg.mutable_response_disconnect()->set_reason_disconnect(pb::remote::Server_Shutdown);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::GetLyrics() {
  fetcher_->FetchInfo(current_song_);
}

void OutgoingDataCreator::SendLyrics(int id, const SongInfoFetcher::Result& result) {
  pb::remote::Message msg;
  msg.set_type(pb::remote::LYRICS);
  pb::remote::ResponseLyrics* response = msg.mutable_response_lyrics();

  foreach (const CollapsibleInfoPane::Data& data, result.info_) {
    // If the size is zero, do not send the provider
    UltimateLyricsLyric* editor = qobject_cast<UltimateLyricsLyric*>(data.content_object_);
    if (editor->toPlainText().length() == 0)
      continue;

    pb::remote::Lyric* lyric = response->mutable_lyrics()->Add();

    lyric->set_id(DataCommaSizeFromQString(data.id_));
    lyric->set_title(DataCommaSizeFromQString(data.title_));
    lyric->set_content(DataCommaSizeFromQString(editor->toPlainText()));
  }
  SendDataToClients(&msg);

  results_.take(id);
}

void OutgoingDataCreator::SendSongs(const pb::remote::RequestDownloadSongs &request,
                                    RemoteClient* client) {

  if (!download_queue_.contains(client)) {
    download_queue_.insert(client, QQueue<DownloadItem>() );
  }

  switch (request.download_item()) {
  case pb::remote::CurrentItem: {
    DownloadItem item(current_song_, 1 , 1);
    download_queue_[client].append(item);
    break;
  }
  case pb::remote::ItemAlbum:
    SendAlbum(client, current_song_);
    break;
  case pb::remote::APlaylist:
    SendPlaylist(client, request.playlist_id());
    break;
  default:
    break;
  }

  // Send first file
  OfferNextSong(client);
}

void OutgoingDataCreator::OfferNextSong(RemoteClient *client) {
  if (!download_queue_.contains(client))
    return;

  pb::remote::Message msg;

  if (download_queue_.value(client).isEmpty()) {
    // We sent all songs, tell the client the queue is empty
    msg.set_type(pb::remote::DOWNLOAD_QUEUE_EMPTY);
  } else {
    // Get the item and send the single song
    DownloadItem item = download_queue_[client].head();

    msg.set_type(pb::remote::SONG_FILE_CHUNK);
    pb::remote::ResponseSongFileChunk* chunk = msg.mutable_response_song_file_chunk();

    // Song offer is chunk no 0
    chunk->set_chunk_count(0);
    chunk->set_chunk_number(0);
    chunk->set_file_count(item.song_count_);
    chunk->set_file_number(item.song_no_);

    CreateSong(item.song_, QImage() , -1,
               chunk->mutable_song_metadata());
  }

  client->SendData(&msg);
}

void OutgoingDataCreator::ResponseSongOffer(RemoteClient *client, bool accepted) {
  if (!download_queue_.contains(client))
    return;

  if (download_queue_.value(client).isEmpty())
    return;

  // Get the item and send the single song
  DownloadItem item = download_queue_[client].dequeue();
  if (accepted)
    SendSingleSong(client, item.song_, item.song_no_, item.song_count_);

  // And offer the next song
  OfferNextSong(client);
}

void OutgoingDataCreator::SendSingleSong(RemoteClient* client, const Song &song,
                                         int song_no, int song_count) {
  // Only local files!!!
  if (!(song.url().scheme() == "file"))
    return;

  // Open the file
  QFile file(song.url().toLocalFile());
  file.open(QIODevice::ReadOnly);

  QByteArray data;
  pb::remote::Message msg;
  pb::remote::ResponseSongFileChunk* chunk = msg.mutable_response_song_file_chunk();
  msg.set_type(pb::remote::SONG_FILE_CHUNK);

  QImage null_image;

  // Calculate the number of chunks
  int chunk_count  = qRound((file.size() / kFileChunkSize) + 0.5);
  int chunk_number = 1;

  while (!file.atEnd()) {
    // Read file chunk
    data = file.read(kFileChunkSize);

    // Set chunk data
    chunk->set_chunk_count(chunk_count);
    chunk->set_chunk_number(chunk_number);
    chunk->set_file_count(song_count);
    chunk->set_file_number(song_no);
    chunk->set_size(file.size());
    chunk->set_data(data.data(), data.size());

    // On the first chunk send the metadata, so the client knows
    // what file it receives.
    if (chunk_number == 1) {
      int i = app_->playlist_manager()->active()->current_row();
      CreateSong(
        song, null_image, i,
        msg.mutable_response_song_file_chunk()->mutable_song_metadata());
    }

    // Send data directly to the client
    client->SendData(&msg);

    // Clear working data
    chunk->Clear();
    data.clear();

    chunk_number++;
  }

  file.close();
}

void OutgoingDataCreator::SendAlbum(RemoteClient *client, const Song &song) {
  // No streams!
  if (song.url().scheme() != "file")
    return;

  SongList album = app_->library_backend()->GetSongsByAlbum(song.album());

  foreach (Song s, album) {
    DownloadItem item(s, album.indexOf(s)+1, album.size());
    download_queue_[client].append(item);
  }
}

void OutgoingDataCreator::SendPlaylist(RemoteClient *client, int playlist_id) {
  Playlist* playlist = app_->playlist_manager()->playlist(playlist_id);
  if(!playlist) {
    qLog(Info) << "Could not find playlist with id = " << playlist_id;
    return;
  }
  SongList song_list = playlist->GetAllSongs();

  // Count the local songs
  int count = 0;
  foreach (Song s, song_list) {
    if (s.url().scheme() == "file") {
      count++;
    }
  }

  foreach (Song s, song_list) {
    // Only local files!
    if (s.url().scheme() == "file") {
      DownloadItem item(s, song_list.indexOf(s)+1, count);
      download_queue_[client].append(item);
    }
  }
}

void OutgoingDataCreator::SendLibrary(RemoteClient *client) {
  // Get a temporary file name
  QString temp_file_name = Utilities::GetTemporaryFileName();

  // Attach this file to the database
  Database::AttachedDatabase adb(temp_file_name, "", true);
  QSqlDatabase db(app_->database()->Connect());

  app_->database()->AttachDatabaseOnDbConnection("songs_export", adb, db);

  // Copy the content of the song table to this temporary database
  QSqlQuery q(QString("create table songs_export.songs as SELECT * FROM songs;"), db);

  if (app_->database()->CheckErrors(q)) return;

  // Detach the database
  app_->database()->DetachDatabase("songs_export");

  // Open the file
  QFile file(temp_file_name);
  file.open(QIODevice::ReadOnly);

  QByteArray data;
  pb::remote::Message msg;
  pb::remote::ResponseLibraryChunk* chunk = msg.mutable_response_library_chunk();
  msg.set_type(pb::remote::LIBRARY_CHUNK);

  // Calculate the number of chunks
  int chunk_count  = qRound((file.size() / kFileChunkSize) + 0.5);
  int chunk_number = 1;

  while (!file.atEnd()) {
    // Read file chunk
    data = file.read(kFileChunkSize);

    // Set chunk data
    chunk->set_chunk_count(chunk_count);
    chunk->set_chunk_number(chunk_number);
    chunk->set_size(file.size());
    chunk->set_data(data.data(), data.size());

    // Send data directly to the client
    client->SendData(&msg);

    // Clear working data
    chunk->Clear();
    data.clear();

    chunk_number++;
  }

  // Remove temporary file
  file.remove();
}
