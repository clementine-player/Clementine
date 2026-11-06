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

#include "incomingdataparser.h"

#include <QDir>
#include <algorithm>

#include "core/logging.h"
#include "core/mimedata.h"
#include "core/timeconstants.h"
#include "engines/enginebase.h"
#include "internet/core/internetmodel.h"
#include "internet/lastfm/lastfmservice.h"
#include "library/librarybackend.h"
#include "networkremote/networkremote.h"
#include "networkremote/remotefiles.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "playlist/songmimedata.h"

IncomingDataParser::IncomingDataParser(Application* app)
    : app_(app),
      song_loader_(
          new RemoteSongLoader(app->library_backend(), app->player())) {
  // load settings initially and sign up for updates
  ReloadSettings();
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));

  Player* player = app_->player();
  PlaylistManager* playlist_manager = app_->playlist_manager();

  // Connect all the signals
  // due the player is in a different thread, we cannot access these functions
  // directly
  connect(this, SIGNAL(Play()), player, SLOT(Play()));
  connect(this, SIGNAL(PlayPause()), player, SLOT(PlayPause()));
  connect(this, SIGNAL(Pause()), player, SLOT(Pause()));
  connect(this, SIGNAL(Stop()), player, SLOT(Stop()));
  connect(this, SIGNAL(StopAfterCurrent()), player, SLOT(StopAfterCurrent()));
  connect(this, SIGNAL(Next()), player, SLOT(Next()));
  connect(this, SIGNAL(Previous()), player, SLOT(Previous()));
  connect(this, SIGNAL(SetVolume(int)), player, SLOT(SetVolume(int)));
  connect(this, SIGNAL(PlayAt(int, Engine::TrackChangeFlags, bool)), player,
          SLOT(PlayAt(int, Engine::TrackChangeFlags, bool)));
  connect(this, SIGNAL(SeekTo(int)), player, SLOT(SeekTo(int)));
  connect(this, SIGNAL(Enque(int, int)), playlist_manager,
          SLOT(Enque(int, int)));

  connect(this, SIGNAL(SetActivePlaylist(int)), playlist_manager,
          SLOT(SetActivePlaylist(int)));
  connect(this, SIGNAL(ShuffleCurrent()), playlist_manager,
          SLOT(ShuffleCurrent()));
  connect(this, SIGNAL(SetRepeatMode(PlaylistSequence::RepeatMode)),
          playlist_manager->sequence(),
          SLOT(SetRepeatMode(PlaylistSequence::RepeatMode)));
  connect(this, SIGNAL(SetShuffleMode(PlaylistSequence::ShuffleMode)),
          playlist_manager->sequence(),
          SLOT(SetShuffleMode(PlaylistSequence::ShuffleMode)));
  connect(this,
          SIGNAL(InsertSongs(int, const SongList&, int, bool, bool, bool)),
          playlist_manager,
          SLOT(InsertSongs(int, const SongList&, int, bool, bool, bool)));
  connect(this, SIGNAL(RemoveSongs(int, const QList<int>&)), playlist_manager,
          SLOT(RemoveItemsWithoutUndo(int, const QList<int>&)));
  connect(this, SIGNAL(New(const QString&)), playlist_manager,
          SLOT(New(const QString&)));
  connect(this, SIGNAL(Open(int)), playlist_manager, SLOT(Open(int)));
  connect(this, SIGNAL(Close(int)), playlist_manager, SLOT(Close(int)));
  connect(this, SIGNAL(Clear(int)), playlist_manager, SLOT(Clear(int)));
  connect(this, SIGNAL(Rename(int, const QString&)), playlist_manager,
          SLOT(Rename(int, const QString&)));
  connect(this, SIGNAL(Favorite(int, bool)), playlist_manager,
          SLOT(Favorite(int, bool)));

  connect(this, SIGNAL(RateCurrentSong(double)), playlist_manager,
          SLOT(RateCurrentSong(double)));

  connect(this, SIGNAL(Love()), app_->scrobbler(), SLOT(Love()));
}

IncomingDataParser::~IncomingDataParser() {}

void IncomingDataParser::ReloadSettings() {
  QSettings s;
  s.beginGroup(MainWindow::kSettingsGroup);
  // The same default as MainWindow, so a remote's tap on a song does what a
  // double-click does.
  doubleclick_playlist_addmode_ = MainWindow::PlaylistAddBehaviour(
      s.value("doubleclick_playlist_addmode",
              MainWindow::PlaylistAddBehaviour_Play)
          .toInt());
}

bool IncomingDataParser::close_connection() { return close_connection_; }

void IncomingDataParser::Parse(const cpb::remote::Message& msg) {
  close_connection_ = false;
  RemoteClient* client = qobject_cast<RemoteClient*>(sender());

  // Now check what's to do
  switch (msg.type()) {
    case cpb::remote::CONNECT:
      ClientConnect(msg, client);
      break;
    case cpb::remote::DISCONNECT:
      close_connection_ = true;
      break;
    case cpb::remote::REQUEST_PLAYLISTS:
      SendPlaylists(msg);
      break;
    case cpb::remote::REQUEST_PLAYLIST_SONGS:
      GetPlaylistSongs(msg);
      break;
    case cpb::remote::SET_VOLUME:
      emit SetVolume(msg.request_set_volume().volume());
      break;
    case cpb::remote::PLAY:
      emit Play();
      break;
    case cpb::remote::PLAYPAUSE:
      emit PlayPause();
      break;
    case cpb::remote::PAUSE:
      emit Pause();
      break;
    case cpb::remote::STOP:
      emit Stop();
      break;
    case cpb::remote::STOP_AFTER:
      emit StopAfterCurrent();
      break;
    case cpb::remote::NEXT:
      emit Next();
      break;
    case cpb::remote::PREVIOUS:
      emit Previous();
      break;
    case cpb::remote::CHANGE_SONG:
      ChangeSong(msg);
      break;
    case cpb::remote::SHUFFLE_PLAYLIST:
      emit ShuffleCurrent();
      break;
    case cpb::remote::REPEAT:
      SetRepeatMode(msg.repeat());
      break;
    case cpb::remote::SHUFFLE:
      SetShuffleMode(msg.shuffle());
      break;
    case cpb::remote::SET_TRACK_POSITION:
      emit SeekTo(msg.request_set_track_position().position());
      break;
    case cpb::remote::INSERT_URLS:
      InsertUrls(msg);
      break;
    case cpb::remote::REMOVE_SONGS:
      RemoveSongs(msg);
      break;
    case cpb::remote::OPEN_PLAYLIST:
      OpenPlaylist(msg);
      break;
    case cpb::remote::CLOSE_PLAYLIST:
      ClosePlaylist(msg);
      break;
    case cpb::remote::UPDATE_PLAYLIST:
      UpdatePlaylist(msg);
      break;
    case cpb::remote::LOVE:
      emit Love();
      break;
    case cpb::remote::BAN:
      emit Ban();
      break;
    case cpb::remote::GET_LYRICS:
      emit GetLyrics();
      break;
    case cpb::remote::DOWNLOAD_SONGS:
      client->song_sender()->SendSongs(msg.request_download_songs());
      break;
    case cpb::remote::SONG_OFFER_RESPONSE:
      client->song_sender()->ResponseSongOffer(
          msg.response_song_offer().accepted());
      break;
    case cpb::remote::GET_LIBRARY:
      emit SendLibrary(client);
      break;
    case cpb::remote::RATE_SONG:
      RateSong(msg);
      break;
    case cpb::remote::GLOBAL_SEARCH:
      GlobalSearch(client, msg);
      break;
    case cpb::remote::REQUEST_FILES:
      emit SendListFiles(
          QString::fromStdString(msg.request_list_files().relative_path()),
          client);
      break;
    case cpb::remote::APPEND_FILES:
      AppendFilesToPlaylist(msg);
      break;
    case cpb::remote::REQUEST_SAVED_RADIOS:
      emit SendSavedRadios(client);
      break;

    case cpb::remote::RENDERER_STATUS:
    case cpb::remote::RENDERER_TRACK_ENDED:
    case cpb::remote::RENDERER_ERROR:
    case cpb::remote::REQUEST_OUTPUTS:
    case cpb::remote::SET_OUTPUT:
      if (streaming_enabled_) {
        const std::string data = msg.SerializeAsString();
        emit RendererMessage(client->id(),
                             QByteArray(data.data(), data.size()));
      }
      break;

    case cpb::remote::REQUEST_BROWSE:
    case cpb::remote::REQUEST_BROWSE_ADD: {
      const std::string data = msg.SerializeAsString();
      emit BrowseMessage(client->id(), QByteArray(data.data(), data.size()));
      break;
    }

    case cpb::remote::REQUEST_SMART_PLAYLISTS:
    case cpb::remote::REQUEST_SMART_PLAYLIST_ADD: {
      const std::string data = msg.SerializeAsString();
      emit SmartPlaylistMessage(client->id(),
                                QByteArray(data.data(), data.size()));
      break;
    }

    case cpb::remote::REQUEST_SYNC_SOURCES:
    case cpb::remote::REQUEST_SYNC_MANIFEST:
      if (NetworkRemote::MediaSyncEnabled()) {
        const std::string data = msg.SerializeAsString();
        emit SyncMessage(client->id(), client->peer_address().toString(),
                         QByteArray(data.data(), data.size()));
      }
      break;

    default:
      break;
  }
}

void IncomingDataParser::GetPlaylistSongs(const cpb::remote::Message& msg) {
  emit SendPlaylistSongs(msg.request_playlist_songs().id());
}

void IncomingDataParser::ChangeSong(const cpb::remote::Message& msg) {
  // Get the first entry and check if there is a song
  const cpb::remote::RequestChangeSong& request = msg.request_change_song();

  // Check if we need to change the playlist
  if (request.playlist_id() != app_->playlist_manager()->active_id()) {
    emit SetActivePlaylist(request.playlist_id());
  }

  switch (doubleclick_playlist_addmode_) {
    // Play the selected song
    case MainWindow::PlaylistAddBehaviour_Play:
      emit PlayAt(request.song_index(), Engine::Manual, false);
      break;

    // Enque the selected song
    case MainWindow::PlaylistAddBehaviour_Enqueue:
      emit Enque(request.playlist_id(), request.song_index());
      if (app_->player()->GetState() != Engine::Playing) {
        emit PlayAt(request.song_index(), Engine::Manual, false);
      }

      break;
  }
}

void IncomingDataParser::SetRepeatMode(const cpb::remote::Repeat& repeat) {
  switch (repeat.repeat_mode()) {
    case cpb::remote::Repeat_Off:
      emit SetRepeatMode(PlaylistSequence::Repeat_Off);
      break;
    case cpb::remote::Repeat_Track:
      emit SetRepeatMode(PlaylistSequence::Repeat_Track);
      break;
    case cpb::remote::Repeat_Album:
      emit SetRepeatMode(PlaylistSequence::Repeat_Album);
      break;
    case cpb::remote::Repeat_Playlist:
      emit SetRepeatMode(PlaylistSequence::Repeat_Playlist);
      break;
    default:
      break;
  }
}

void IncomingDataParser::SetShuffleMode(const cpb::remote::Shuffle& shuffle) {
  switch (shuffle.shuffle_mode()) {
    case cpb::remote::Shuffle_Off:
      emit SetShuffleMode(PlaylistSequence::Shuffle_Off);
      break;
    case cpb::remote::Shuffle_All:
      emit SetShuffleMode(PlaylistSequence::Shuffle_All);
      break;
    case cpb::remote::Shuffle_InsideAlbum:
      emit SetShuffleMode(PlaylistSequence::Shuffle_InsideAlbum);
      break;
    case cpb::remote::Shuffle_Albums:
      emit SetShuffleMode(PlaylistSequence::Shuffle_Albums);
      break;
    default:
      break;
  }
}

void IncomingDataParser::InsertUrls(const cpb::remote::Message& msg) {
  const cpb::remote::RequestInsertUrls& request = msg.request_insert_urls();
  int playlist_id = request.playlist_id();

  // Insert plain urls without metadata
  if (!request.urls().empty()) {
    QList<QUrl> urls;
    for (auto it = request.urls().begin(); it != request.urls().end(); ++it) {
      const QUrl url(QString::fromStdString(*it));
      if (MayAdd(url)) urls << url;
    }

    if (request.has_new_playlist_name())
      playlist_id =
          app_->playlist_manager()->New(request.new_playlist_name().c_str());

    // A URL can be a folder or a playlist file, so the songs it names are
    // loaded and checked here, before they're added.
    if (!urls.isEmpty()) {
      PlaylistManager* manager = app_->playlist_manager();
      const int pos = request.position();
      const bool play_now = request.play_now();
      const bool enqueue = request.enqueue();
      const bool enqueue_next = request.enqueue_next();
      song_loader_->Load(urls, AcceptSong(), manager,
                         [manager, playlist_id, pos, play_now, enqueue,
                          enqueue_next](const SongList& songs) {
                           Playlist* playlist = manager->playlist(playlist_id);
                           if (!playlist || songs.isEmpty()) return;
                           if (play_now && manager->active_id() != playlist_id)
                             manager->SetActivePlaylist(playlist_id);
                           playlist->InsertSongsOrLibraryItems(
                               songs, pos, play_now, enqueue, enqueue_next);
                         });
    }
  }

  // Add songs with metadata if present
  if (!request.songs().empty()) {
    SongList songs;
    for (int i = 0; i < request.songs().size(); i++) {
      Song song = CreateSongFromProtobuf(request.songs(i));
      if (!MayAdd(song.url())) continue;
      TrustCoverFromLibrary(&song);
      songs << song;
    }

    // create a new playlist if required and not already done above by
    // InsertUrls
    if (request.has_new_playlist_name() && playlist_id == request.playlist_id())
      playlist_id =
          app_->playlist_manager()->New(request.new_playlist_name().c_str());

    emit InsertSongs(playlist_id, songs, request.position(), request.play_now(),
                     request.enqueue(), request.enqueue_next());
  }
}

void IncomingDataParser::RemoveSongs(const cpb::remote::Message& msg) {
  const cpb::remote::RequestRemoveSongs& request = msg.request_remove_songs();

  // Extract urls
  QList<int> songs;
  for (int i = 0; i < request.songs().size(); i++) {
    songs.append(request.songs(i));
  }

  // Insert the urls
  emit RemoveSongs(request.playlist_id(), songs);
}

void IncomingDataParser::ClientConnect(const cpb::remote::Message& msg,
                                       RemoteClient* client) {
  // Always sned the Clementine infos
  emit SendClementineInfo();

  if (streaming_enabled_ && msg.request_connect().has_renderer()) {
    const std::string caps =
        msg.request_connect().renderer().SerializeAsString();
    emit RendererConnected(client->id(), QByteArray(caps.data(), caps.size()),
                           client->local_address().toString(),
                           client->local_port(),
                           client->peer_address().toString());
  }

  // Check if we should send the first data
  if (!client->isDownloader()) {
    if (!msg.request_connect().has_send_playlist_songs()  // legacy
        || msg.request_connect().send_playlist_songs()) {
      emit SendFirstData(true);
    } else {
      emit SendFirstData(false);
    }
  }
}

void IncomingDataParser::SendPlaylists(const cpb::remote::Message& msg) {
  if (!msg.has_request_playlists() ||
      !msg.request_playlists().include_closed()) {
    emit SendAllActivePlaylists();
  } else {
    emit SendAllPlaylists();
  }
}

void IncomingDataParser::OpenPlaylist(const cpb::remote::Message& msg) {
  emit Open(msg.request_open_playlist().playlist_id());
}

void IncomingDataParser::ClosePlaylist(const cpb::remote::Message& msg) {
  emit Close(msg.request_close_playlist().playlist_id());
}

void IncomingDataParser::UpdatePlaylist(const cpb::remote::Message& msg) {
  const cpb::remote::RequestUpdatePlaylist& req_update =
      msg.request_update_playlist();
  if (req_update.has_create_new_playlist() &&
      req_update.create_new_playlist()) {
    emit New(req_update.has_new_playlist_name()
                 ? req_update.new_playlist_name().c_str()
                 : "New Playlist");
    return;
  }
  if (req_update.has_clear_playlist() && req_update.clear_playlist()) {
    emit Clear(req_update.playlist_id());
    return;
  }
  if (req_update.has_new_playlist_name() &&
      req_update.new_playlist_name().size())
    emit Rename(req_update.playlist_id(),
                req_update.new_playlist_name().c_str());
  if (req_update.has_favorite())
    emit Favorite(req_update.playlist_id(), req_update.favorite());
}

void IncomingDataParser::RateSong(const cpb::remote::Message& msg) {
  double rating = (double)msg.request_rate_song().rating();
  emit RateCurrentSong(rating);
}

void IncomingDataParser::GlobalSearch(RemoteClient* client,
                                      const cpb::remote::Message& msg) {
  emit DoGlobalSearch(QStringFromStdString(msg.request_global_search().query()),
                      client);
}

bool IncomingDataParser::MayAdd(const QUrl& url) const {
  LibraryBackend* library = app_->library_backend();
  if (RemoteFiles::MayAdd(url, files_root_folder_, [library](const QUrl& file) {
        return !library->GetSongsByUrl(file).isEmpty();
      }))
    return true;
  qLog(Warning) << "Refused a remote's request to add" << url
                << "which isn't in the library or the Files tab's folder";
  return false;
}

void IncomingDataParser::TrustCoverFromLibrary(Song* song) const {
  RemoteFiles::TrustCovers(app_->library_backend(), song);
}

RemoteSongLoader::Accept IncomingDataParser::AcceptSong() const {
  const QString files_root = files_root_folder_;
  LibraryBackend* library = app_->library_backend();
  return [files_root, library](Song* song) {
    return RemoteFiles::AcceptSong(library, files_root, song);
  };
}

Song IncomingDataParser::CreateSongFromProtobuf(
    const cpb::remote::SongMetadata& pb) {
  Song song;
  song.Init(QStringFromStdString(pb.title()), QStringFromStdString(pb.artist()),
            QStringFromStdString(pb.album()), pb.length() * kNsecPerSec);

  song.set_albumartist(QStringFromStdString(pb.albumartist()));
  song.set_genre(QStringFromStdString(pb.genre()));
  song.set_year(QStringFromStdString(pb.pretty_year()).toInt());
  song.set_track(pb.track());
  song.set_disc(pb.disc());
  song.set_url(QUrl(QStringFromStdString(pb.url())));
  song.set_filesize(pb.file_size());
  song.set_rating(pb.rating());
  song.set_basefilename(QStringFromStdString(pb.filename()));
  song.set_art_automatic(QStringFromStdString(pb.art_automatic()));
  song.set_art_manual(QStringFromStdString(pb.art_manual()));
  song.set_filetype(static_cast<Song::FileType>(pb.type()));

  return song;
}

void IncomingDataParser::AppendFilesToPlaylist(
    const cpb::remote::Message& msg) {
  if (files_root_folder_.isEmpty()) {  // should never happen...
    qLog(Warning) << "Remote root dir is not set although receiving "
                     "APPEND_FILES request...";
    return;
  }
  QDir root_dir(files_root_folder_);
  if (!root_dir.exists()) {
    qLog(Warning) << "Remote root dir doesn't exist...";
    return;
  }

  const cpb::remote::RequestAppendFiles& req_append =
      msg.request_append_files();
  QString relative_path = QString::fromStdString(req_append.relative_path());
  if (relative_path.startsWith("/")) relative_path.remove(0, 1);

  QFileInfo fi_folder(root_dir, relative_path);
  if (!fi_folder.exists()) {
    qLog(Warning) << "Remote relative path " << relative_path
                  << " doesn't exist...";
    return;
  } else if (!fi_folder.isDir()) {
    qLog(Warning) << "Remote relative path " << relative_path
                  << " is not a directory...";
    return;
  } else if (root_dir.relativeFilePath(fi_folder.absoluteFilePath())
                 .startsWith("../")) {
    qLog(Warning) << "Remote relative path " << relative_path
                  << " should not be accessed...";
    return;
  }

  QList<QUrl> urls;
  QDir dir(fi_folder.absoluteFilePath());
  for (const auto& file : req_append.files()) {
    QFileInfo fi(dir, file.c_str());
    if (fi.exists() && RemoteFiles::IsWithin(root_dir, fi))
      urls << QUrl::fromLocalFile(fi.canonicalFilePath());
  }
  if (urls.isEmpty()) return;

  // A file can be a playlist file, whose entries are checked as they're
  // loaded.
  const bool playing = app_->player()->GetState() == Engine::Playing;
  if (!req_append.has_new_playlist_name() && req_append.has_playlist_id() &&
      !playing) {
    // as me may play the song, we change the current playlist
    emit SetCurrentPlaylist(req_append.playlist_id());
  }
  song_loader_->Load(urls, AcceptSong(), this,
                     [this, req_append](const SongList& songs) {
                       if (!songs.isEmpty())
                         AddFilesToPlaylist(req_append, songs);
                     });
}

void IncomingDataParser::AddFilesToPlaylist(
    const cpb::remote::RequestAppendFiles& req_append, const SongList& songs) {
  SongMimeData* data = new SongMimeData;
  data->songs = songs;
  if (req_append.has_play_now()) data->play_now_ = req_append.play_now();
  if (req_append.has_clear_first())
    data->clear_first_ = req_append.clear_first();
  if (req_append.has_new_playlist_name()) {
    QString playlist_name =
        QString::fromStdString(req_append.new_playlist_name());
    if (!playlist_name.isEmpty()) {
      data->open_in_new_playlist_ = true;
      data->name_for_new_playlist_ = playlist_name;
    }
  } else if (req_append.has_playlist_id()) {
    // The songs go to the playlist the client named, even if the current one
    // changed while they were loading.
    data->playlist_id = req_append.playlist_id();
  }
  emit AddToPlaylistSignal(data);
}
