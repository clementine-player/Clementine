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

#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <cmath>

#include "core/database.h"
#include "core/logging.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "globalsearch/librarysearchprovider.h"
#include "internet/core/internetmodel.h"
#include "internet/internetradio/savedradio.h"
#include "library/librarybackend.h"
#include "networkremote.h"
#include "ui/iconloader.h"

const quint32 OutgoingDataCreator::kFileChunkSize = 100000;  // in Bytes

OutgoingDataCreator::OutgoingDataCreator(Application* app)
    : app_(app),
      aww_(false),
      ultimate_reader_(new UltimateLyricsReader(this)),
      fetcher_(new SongInfoFetcher(this)) {
  // Create Keep Alive Timer
  keep_alive_timer_ = new QTimer(this);
  connect(keep_alive_timer_, SIGNAL(timeout()), this, SLOT(SendKeepAlive()));
  keep_alive_timeout_ = 10000;
}

OutgoingDataCreator::~OutgoingDataCreator() {}

void OutgoingDataCreator::SetClients(QList<RemoteClient*>* clients) {
  clients_ = clients;
  // After we got some clients, start the keep alive timer
  // Default: every 10 seconds
  keep_alive_timer_->start(keep_alive_timeout_);

  // Create the song position timer
  track_position_timer_ = new QTimer(this);
  connect(track_position_timer_, SIGNAL(timeout()), this,
          SLOT(UpdateTrackPosition()));

  // Parse the ultimate lyrics xml file
  ultimate_reader_->SetThread(this->thread());
  ProviderList provider_list =
      ultimate_reader_->Parse(":lyrics/ultimate_providers.xml");

  // Set up the lyrics parser
  connect(fetcher_, SIGNAL(ResultReady(int, SongInfoFetcher::Result)),
          SLOT(SendLyrics(int, SongInfoFetcher::Result)));

  for (SongInfoProvider* provider : provider_list) {
    fetcher_->AddProvider(provider);
  }

  CheckEnabledProviders();

  // Setup global search
  app_->global_search()->ReloadSettings();

  connect(app_->global_search(),
          SIGNAL(ResultsAvailable(int, SearchProvider::ResultList)),
          SLOT(ResultsAvailable(int, SearchProvider::ResultList)),
          Qt::QueuedConnection);

  connect(app_->global_search(), SIGNAL(SearchFinished(int)),
          SLOT(SearchFinished(int)), Qt::QueuedConnection);
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
                << "azlyrics.com"
                << "songlyrics.com"
                << "elyrics.net"
                << "lyricsdownload.com"
                << "lyrics.com"
                << "lyricsbay.com"
                << "directlyrics.com"
                << "teksty.org"
                << "tekstowo.pl (Polish translations)"
                << "vagalume.uol.com.br"
                << "vagalume.uol.com.br (Portuguese translations)"
                << "darklyrics.com";

  QVariant saved_order = s.value("search_order", default_order);
  for (const QVariant& name : saved_order.toList()) {
    SongInfoProvider* provider = ProviderByName(name.toString());
    if (provider) ordered_providers << provider;
  }

  // Enable all the providers in the list and rank them
  int relevance = 100;
  for (SongInfoProvider* provider : ordered_providers) {
    provider->set_enabled(true);
    qobject_cast<UltimateLyricsProvider*>(provider)->set_relevance(relevance--);
  }

  // Any lyric providers we don't have in ordered_providers are considered
  // disabled
  for (SongInfoProvider* provider : fetcher_->providers()) {
    if (qobject_cast<UltimateLyricsProvider*>(provider) &&
        !ordered_providers.contains(provider)) {
      provider->set_enabled(false);
    }
  }
}

SongInfoProvider* OutgoingDataCreator::ProviderByName(
    const QString& name) const {
  for (SongInfoProvider* provider : fetcher_->providers()) {
    if (UltimateLyricsProvider* lyrics =
            qobject_cast<UltimateLyricsProvider*>(provider)) {
      if (lyrics->name() == name) return provider;
    }
  }
  return nullptr;
}

void OutgoingDataCreator::SendDataToClients(cpb::remote::Message* msg) {
  // Check if we have clients to send data to
  if (clients_->empty()) {
    return;
  }

  for (RemoteClient* client : *clients_) {
    // Do not send data to downloaders
    if (client->isDownloader()) {
      if (client->State() != QTcpSocket::ConnectedState) {
        clients_->removeAt(clients_->indexOf(client));
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
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::INFO);

  // Now add the message specific data
  cpb::remote::ResponseClementineInfo* info =
      msg.mutable_response_clementine_info();
  SetEngineState(info);

  // allowed extensions for REQUEST_FILES and LIST_FILES
  for (const QString& ext : files_music_extensions_)
    info->add_files_music_extensions(ext.toStdString());

  QString version =
      QString("%1 %2").arg(QCoreApplication::applicationName(),
                           QCoreApplication::applicationVersion());
  info->set_version(version.toLatin1());
  info->set_allow_downloads(allow_downloads_);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::SetEngineState(
    cpb::remote::ResponseClementineInfo* msg) {
  switch (app_->player()->GetState()) {
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

void OutgoingDataCreator::SendAllPlaylists() {
  // Get all Playlists
  PlaylistManager* playlist_manager = app_->playlist_manager();
  int active_playlist = playlist_manager->active_id();

  // Create message
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::PLAYLISTS);

  cpb::remote::ResponsePlaylists* playlists = msg.mutable_response_playlists();
  playlists->set_include_closed(true);

  // Get all playlists, even ones that are hidden in the UI.
  for (const PlaylistBackend::Playlist& p :
       app_->playlist_backend()->GetAllPlaylists()) {
    bool playlist_open = playlist_manager->IsPlaylistOpen(p.id);
    int item_count =
        playlist_open ? playlist_manager->playlist(p.id)->rowCount() : 0;

    // Create a new playlist
    cpb::remote::Playlist* playlist = playlists->add_playlist();
    playlist->set_name(DataCommaSizeFromQString(p.name));
    playlist->set_id(p.id);
    playlist->set_active((p.id == active_playlist));
    playlist->set_item_count(item_count);
    playlist->set_closed(!playlist_open);
    playlist->set_favorite(p.favorite);
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendAllActivePlaylists() {
  // Get all Playlists
  QList<Playlist*> app_playlists = app_->playlist_manager()->GetAllPlaylists();
  int active_playlist = app_->playlist_manager()->active_id();

  // Create message
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::PLAYLISTS);

  cpb::remote::ResponsePlaylists* playlists = msg.mutable_response_playlists();

  QListIterator<Playlist*> it(app_playlists);
  while (it.hasNext()) {
    // Get the next Playlist
    Playlist* p = it.next();
    QString playlist_name = app_->playlist_manager()->GetPlaylistName(p->id());

    // Create a new playlist
    cpb::remote::Playlist* playlist = playlists->add_playlist();
    playlist->set_name(DataCommaSizeFromQString(playlist_name));
    playlist->set_id(p->id());
    playlist->set_active((p->id() == active_playlist));
    playlist->set_item_count(p->rowCount());
    playlist->set_closed(false);
    playlist->set_favorite(p->is_favorite());
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::ActiveChanged(Playlist* playlist) {
  // Send the tracks of the active playlist
  SendPlaylistSongs(playlist->id());

  // Send the changed message after sending the playlist songs
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::ACTIVE_PLAYLIST_CHANGED);
  msg.mutable_response_active_changed()->set_id(playlist->id());
  SendDataToClients(&msg);
}

void OutgoingDataCreator::PlaylistAdded(int id, const QString& name,
                                        bool favorite) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::PlaylistDeleted(int id) { SendAllActivePlaylists(); }

void OutgoingDataCreator::PlaylistClosed(int id) { SendAllActivePlaylists(); }

void OutgoingDataCreator::PlaylistRenamed(int id, const QString& new_name) {
  SendAllActivePlaylists();
}

void OutgoingDataCreator::SendFirstData(bool send_playlist_songs) {
  Player* player = app_->player();
  // First Send the current song
  PlaylistItemPtr item = player->GetCurrentItem();
  if (!item) {
    qLog(Info) << "No current item found!";
  }

  CurrentSongChanged(current_song_, current_uri_, current_image_);

  // then the current volume
  VolumeChanged(player->GetVolume());

  // Check if we need to start the track position timer
  if (!track_position_timer_->isActive() &&
      player->engine()->state() == Engine::Playing) {
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
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::FIRST_DATA_SENT_COMPLETE);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::CurrentSongChanged(const Song& song,
                                             const QString& uri,
                                             const QImage& img) {
  current_song_ = song;
  current_uri_ = uri;

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
  int i = app_->playlist_manager()->active()->current_row();
  CreateSong(current_song_, current_image_, i,
             msg.mutable_response_current_metadata()->mutable_song_metadata());

  SendDataToClients(&msg);
}

void OutgoingDataCreator::CreateSong(const Song& song, const QImage& art,
                                     const int index,
                                     cpb::remote::SongMetadata* song_metadata) {
  if (song.is_valid()) {
    song_metadata->set_id(song.id());
    song_metadata->set_index(index);
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

void OutgoingDataCreator::SendPlaylistSongs(int id) {
  // Get the PlaylistQByteArray(data.data(), data.size()
  Playlist* playlist = app_->playlist_manager()->playlist(id);
  if (!playlist) {
    qLog(Info) << "Could not find playlist with id = " << id;
    return;
  }

  // Create the message and the playlist
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::PLAYLIST_SONGS);

  // Create the Response message
  cpb::remote::ResponsePlaylistSongs* pb_response_playlist_songs =
      msg.mutable_response_playlist_songs();

  // Create a new playlist
  cpb::remote::Playlist* pb_playlist =
      pb_response_playlist_songs->mutable_requested_playlist();
  pb_playlist->set_id(id);

  // Send all songs
  int index = 0;
  SongList song_list = playlist->GetAllSongs();
  QListIterator<Song> it(song_list);
  QImage null_img;
  while (it.hasNext()) {
    Song song = it.next();
    cpb::remote::SongMetadata* pb_song =
        pb_response_playlist_songs->add_songs();
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

void OutgoingDataCreator::SendRepeatMode(PlaylistSequence::RepeatMode mode) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::REPEAT);

  switch (mode) {
    case PlaylistSequence::Repeat_Off:
      msg.mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Off);
      break;
    case PlaylistSequence::Repeat_Track:
      msg.mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Track);
      break;
    case PlaylistSequence::Repeat_Album:
      msg.mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Album);
      break;
    case PlaylistSequence::Repeat_Playlist:
      msg.mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Playlist);
      break;
    case PlaylistSequence::Repeat_OneByOne:
      msg.mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_OneByOne);
      break;
    case PlaylistSequence::Repeat_Intro:
      msg.mutable_repeat()->set_repeat_mode(cpb::remote::Repeat_Intro);
      break;
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendShuffleMode(PlaylistSequence::ShuffleMode mode) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::SHUFFLE);

  switch (mode) {
    case PlaylistSequence::Shuffle_Off:
      msg.mutable_shuffle()->set_shuffle_mode(cpb::remote::Shuffle_Off);
      break;
    case PlaylistSequence::Shuffle_All:
      msg.mutable_shuffle()->set_shuffle_mode(cpb::remote::Shuffle_All);
      break;
    case PlaylistSequence::Shuffle_InsideAlbum:
      msg.mutable_shuffle()->set_shuffle_mode(cpb::remote::Shuffle_InsideAlbum);
      break;
    case PlaylistSequence::Shuffle_Albums:
      msg.mutable_shuffle()->set_shuffle_mode(cpb::remote::Shuffle_Albums);
      break;
  }

  SendDataToClients(&msg);
}

void OutgoingDataCreator::SendKeepAlive() {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::KEEP_ALIVE);
  SendDataToClients(&msg);
}

void OutgoingDataCreator::UpdateTrackPosition() {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::UPDATE_TRACK_POSITION);

  qint64 position_nanosec = app_->player()->engine()->position_nanosec();
  int position = static_cast<int>(
      std::floor(static_cast<double>(position_nanosec) / kNsecPerSec + 0.5));

  if (position_nanosec > current_song_.length_nanosec())
    position = last_track_position_;

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

void OutgoingDataCreator::GetLyrics() { fetcher_->FetchInfo(current_song_); }

void OutgoingDataCreator::SendLyrics(int id,
                                     const SongInfoFetcher::Result& result) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::LYRICS);
  cpb::remote::ResponseLyrics* response = msg.mutable_response_lyrics();

  for (const CollapsibleInfoPane::Data& data : result.info_) {
    // If the size is zero, do not send the provider
    UltimateLyricsLyric* editor =
        qobject_cast<UltimateLyricsLyric*>(data.content_object_);
    if (editor->toPlainText().length() == 0) continue;

    cpb::remote::Lyric* lyric = response->mutable_lyrics()->Add();

    lyric->set_id(DataCommaSizeFromQString(data.id_));
    lyric->set_title(DataCommaSizeFromQString(data.title_));
    lyric->set_content(DataCommaSizeFromQString(editor->toPlainText()));
  }
  SendDataToClients(&msg);

  results_.take(id);
}

void OutgoingDataCreator::SendLibrary(RemoteClient* client) {
  // Get a temporary file name
  QString temp_file_name = Utilities::GetTemporaryFileName();

  // Attach this file to the database
  Database::AttachedDatabase adb(temp_file_name, "", true);
  QSqlDatabase db(app_->database()->Connect());

  app_->database()->AttachDatabaseOnDbConnection("songs_export", adb, db);

  // Copy the content of the song table to this temporary database
  QSqlQuery q(QString("create table songs_export.songs as SELECT * FROM songs "
                      "where unavailable = 0;"),
              db);

  if (app_->database()->CheckErrors(q)) return;

  // Detach the database
  app_->database()->DetachDatabase("songs_export");

  // Open the file
  QFile file(temp_file_name);

  // Get the sha1 hash
  QByteArray sha1 = Utilities::Sha1File(file).toHex();
  qLog(Debug) << "Library sha1" << sha1;

  file.open(QIODevice::ReadOnly);

  QByteArray data;
  cpb::remote::Message msg;
  cpb::remote::ResponseLibraryChunk* chunk =
      msg.mutable_response_library_chunk();
  msg.set_type(cpb::remote::LIBRARY_CHUNK);

  // Calculate the number of chunks
  int chunk_count = qRound((file.size() / kFileChunkSize) + 0.5);
  int chunk_number = 1;

  while (!file.atEnd()) {
    // Read file chunk
    data = file.read(kFileChunkSize);

    // Set chunk data
    chunk->set_chunk_count(chunk_count);
    chunk->set_chunk_number(chunk_number);
    chunk->set_size(file.size());
    chunk->set_data(data.data(), data.size());
    chunk->set_file_hash(sha1.data(), sha1.size());

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

void OutgoingDataCreator::EnableKittens(bool aww) { aww_ = aww; }

void OutgoingDataCreator::SendKitten(const QImage& kitten) {
  if (aww_) {
    current_image_ = kitten;
    SendSongMetadata();
  }
}

void OutgoingDataCreator::DoGlobalSearch(const QString& query,
                                         RemoteClient* client) {
  int id = app_->global_search()->SearchAsync(query);

  GlobalSearchRequest request(id, query, client);
  global_search_result_map_.insert(id, request);

  // Send status message
  cpb::remote::Message msg;
  cpb::remote::ResponseGlobalSearchStatus* status =
      msg.mutable_response_global_search_status();

  msg.set_type(cpb::remote::GLOBAL_SEARCH_STATUS);
  status->set_id(id);
  status->set_query(DataCommaSizeFromQString(query));
  status->set_status(cpb::remote::GlobalSearchStarted);

  client->SendData(&msg);

  qLog(Debug) << "DoGlobalSearch" << id << query;
}

void OutgoingDataCreator::ResultsAvailable(
    int id, const SearchProvider::ResultList& results) {
  if (!global_search_result_map_.contains(id)) return;

  GlobalSearchRequest search_request = global_search_result_map_.value(id);
  RemoteClient* client = search_request.client_;
  QImage null_img;

  cpb::remote::Message msg;
  cpb::remote::ResponseGlobalSearch* response =
      msg.mutable_response_global_search();

  msg.set_type(cpb::remote::GLOBAL_SEARCH_RESULT);
  response->set_id(search_request.id_);
  response->set_query(DataCommaSizeFromQString(search_request.query_));
  response->set_search_provider(
      DataCommaSizeFromQString(results.first().provider_->name()));

  // Append the icon
  QImage icon_image(results.first().provider_->icon_as_image());
  QByteArray byte_array;
  QBuffer buf(&byte_array);
  buf.open(QIODevice::WriteOnly);
  icon_image.save(&buf, "PNG");
  response->set_search_provider_icon(byte_array.constData(), byte_array.size());

  for (const SearchProvider::Result& result : results) {
    cpb::remote::SongMetadata* pb_song = response->add_song_metadata();
    CreateSong(result.metadata_, null_img, 0, pb_song);
  }

  client->SendData(&msg);

  qLog(Debug) << "ResultsAvailable" << id << results.first().provider_->name()
              << results.size();
}

void OutgoingDataCreator::SearchFinished(int id) {
  if (!global_search_result_map_.contains(id)) return;

  GlobalSearchRequest req = global_search_result_map_.take(id);

  // Send status message
  cpb::remote::Message msg;
  cpb::remote::ResponseGlobalSearchStatus* status =
      msg.mutable_response_global_search_status();

  msg.set_type(cpb::remote::GLOBAL_SEARCH_STATUS);
  status->set_id(req.id_);
  status->set_query(DataCommaSizeFromQString(req.query_));
  status->set_status(cpb::remote::GlobalSearchFinished);

  req.client_->SendData(&msg);

  qLog(Debug) << "SearchFinished" << req.id_ << req.query_;
}

void OutgoingDataCreator::SendListFiles(QString relative_path,
                                        RemoteClient* client) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::LIST_FILES);
  cpb::remote::ResponseListFiles* files = msg.mutable_response_list_files();
  // Security checks
  if (files_root_folder_.isEmpty()) {
    files->set_error(cpb::remote::ResponseListFiles::ROOT_DIR_NOT_SET);
    SendDataToClients(&msg);
    return;
  }

  QDir root_dir(files_root_folder_);
  if (!root_dir.exists())
    files->set_error(cpb::remote::ResponseListFiles::ROOT_DIR_NOT_SET);
  else if (relative_path.startsWith("..") || relative_path.startsWith("./.."))
    files->set_error(cpb::remote::ResponseListFiles::DIR_NOT_ACCESSIBLE);
  else {
    if (relative_path.startsWith("/")) relative_path.remove(0, 1);

    QFileInfo fi_folder(root_dir, relative_path);
    if (!fi_folder.exists())
      files->set_error(cpb::remote::ResponseListFiles::DIR_NOT_EXIST);
    else if (!fi_folder.isDir())
      files->set_error(cpb::remote::ResponseListFiles::DIR_NOT_EXIST);
    else if (root_dir.relativeFilePath(fi_folder.absoluteFilePath())
                 .startsWith("../"))
      files->set_error(cpb::remote::ResponseListFiles::DIR_NOT_ACCESSIBLE);
    else {
      files->set_relative_path(
          root_dir.relativeFilePath(fi_folder.absoluteFilePath())
              .toStdString());
      QDir dir(fi_folder.absoluteFilePath());
      dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
      dir.setSorting(QDir::Name | QDir::DirsFirst);

      for (const QFileInfo& fi : dir.entryInfoList()) {
        if (fi.isDir() || files_music_extensions_.contains(fi.suffix())) {
          cpb::remote::FileMetadata* pb_file = files->add_files();
          pb_file->set_is_dir(fi.isDir());
          pb_file->set_filename(fi.fileName().toStdString());
        }
      }
    }
  }
  client->SendData(&msg);
}

void OutgoingDataCreator::SendSavedRadios(RemoteClient* client) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::REQUEST_SAVED_RADIOS);

  SavedRadio* radio_service = static_cast<SavedRadio*>(
      InternetModel::ServiceByName(SavedRadio::kServiceName));
  if (radio_service) {
    cpb::remote::ResponseSavedRadios* radios =
        msg.mutable_response_saved_radios();
    for (const auto& stream : radio_service->Streams()) {
      cpb::remote::Stream* pb_stream = radios->add_streams();
      pb_stream->set_name(stream.name_.toStdString());
      pb_stream->set_url(stream.url_.toString().toStdString());
      if (!stream.url_logo_.isEmpty())
        pb_stream->set_url_logo(stream.url_logo_.toString().toStdString());
    }
  }
  client->SendData(&msg);
}
