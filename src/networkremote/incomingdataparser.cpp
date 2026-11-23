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
      doubleclick_playlist_addmode_(MainWindow::PlaylistAddBehaviour_Play),
      streaming_enabled_(false),
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
  connect(this,
          SIGNAL(InsertSongsInNewPlaylist(const QString&, const SongList&, int,
                                          bool, bool, bool)),
          playlist_manager,
          SLOT(InsertSongsInNewPlaylist(const QString&, const SongList&, int,
                                        bool, bool, bool)));
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

void IncomingDataParser::Parse(const cpb::remote::Message& msg) {
  RemoteClient* client = qobject_cast<RemoteClient*>(sender());
  // Drop queued Parse signals arriving after a client started closing.
  if (!client || client->is_closing()) return;

  // Now check what's to do
  switch (msg.type()) {
    case cpb::remote::CONNECT:
      ClientConnect(msg, client);
      break;
    case cpb::remote::REQUEST_PLAYLISTS:
      SendPlaylists(client, msg);
      break;
    case cpb::remote::REQUEST_PLAYLIST_SONGS:
      GetPlaylistSongs(client, msg);
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
    case cpb::remote::GET_LYRICS:
      emit GetLyrics(client->id());
      break;
    case cpb::remote::DOWNLOAD_SONGS: {
      // Snapshot current song and playlist state on the main thread first.
      const std::string data = msg.request_download_songs().SerializeAsString();
      emit DownloadSongsRequested(client->id(),
                                  QByteArray(data.data(), data.size()));
      break;
    }
    case cpb::remote::SONG_OFFER_RESPONSE:
      client->song_sender()->ResponseSongOffer(
          msg.response_song_offer().accepted());
      break;
    case cpb::remote::GET_LIBRARY:
      emit SendLibrary(client->id());
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
          client->id());
      break;
    case cpb::remote::APPEND_FILES:
      AppendFilesToPlaylist(msg);
      break;
    case cpb::remote::REQUEST_SAVED_RADIOS:
      emit SendSavedRadios(client->id());
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

void IncomingDataParser::GetPlaylistSongs(RemoteClient* client,
                                          const cpb::remote::Message& msg) {
  emit SendPlaylistSongs(client->id(), msg.request_playlist_songs().id());
}

void IncomingDataParser::ChangeSong(const cpb::remote::Message& msg) {
  const cpb::remote::RequestChangeSong& request = msg.request_change_song();
  const bool enqueue_mode =
      doubleclick_playlist_addmode_ == MainWindow::PlaylistAddBehaviour_Enqueue;
  emit ChangeSongRequested(request.playlist_id(), request.song_index(),
                           enqueue_mode);
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
    case cpb::remote::Repeat_OneByOne:
      emit SetRepeatMode(PlaylistSequence::Repeat_OneByOne);
      break;
    case cpb::remote::Repeat_Intro:
      emit SetRepeatMode(PlaylistSequence::Repeat_Intro);
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
  const int playlist_id = request.playlist_id();
  const bool has_new_playlist = request.has_new_playlist_name();
  const QString new_playlist_name =
      QString::fromStdString(request.new_playlist_name());

  QList<QUrl> urls;
  for (const std::string& raw : request.urls()) {
    const QUrl url = RemoteFiles::ParseUrl(raw);
    if (MayAdd(url)) urls << url;
  }

  SongList inline_songs;
  for (int i = 0; i < request.songs().size(); i++) {
    Song song = CreateSongFromProtobuf(request.songs(i));
    if (!MayAdd(song.url())) continue;
    TrustCoverFromLibrary(&song);
    inline_songs << song;
  }

  // Load any plain URLs asynchronously and combine them with any inline songs
  // so new_playlist_name creates at most one playlist.
  if (!urls.isEmpty()) {
    PlaylistManager* manager = app_->playlist_manager();
    const int pos = request.position();
    const bool play_now = request.play_now();
    const bool enqueue = request.enqueue();
    const bool enqueue_next = request.enqueue_next();
    // Pass manager as the callback context so RemoteSongLoader posts
    // insertion onto the main thread instead of mutating playlists off-thread.
    song_loader_->Load(urls, AcceptSong(), manager,
                       [manager, playlist_id, has_new_playlist,
                        new_playlist_name, pos, play_now, enqueue, enqueue_next,
                        inline_songs](const SongList& loaded) {
                         SongList all = loaded;
                         all.append(inline_songs);
                         if (all.isEmpty()) return;
                         const int target_id =
                             has_new_playlist ? manager->New(new_playlist_name)
                                              : playlist_id;
                         Playlist* playlist = manager->playlist(target_id);
                         if (!playlist) return;
                         if (play_now && manager->active_id() != target_id) {
                           manager->SetActivePlaylist(target_id);
                         }
                         playlist->InsertSongsOrLibraryItems(
                             all, pos, play_now, enqueue, enqueue_next);
                       });
    return;
  }

  if (!inline_songs.isEmpty()) {
    // Marshal inline-only inserts onto the main thread via queued signals.
    if (has_new_playlist) {
      emit InsertSongsInNewPlaylist(new_playlist_name, inline_songs,
                                    request.position(), request.play_now(),
                                    request.enqueue(), request.enqueue_next());
    } else {
      emit InsertSongs(playlist_id, inline_songs, request.position(),
                       request.play_now(), request.enqueue(),
                       request.enqueue_next());
    }
  }
}

void IncomingDataParser::RemoveSongs(const cpb::remote::Message& msg) {
  const cpb::remote::RequestRemoveSongs& request = msg.request_remove_songs();

  // Extract indices
  QList<int> songs;
  for (int i = 0; i < request.songs().size(); i++) {
    songs.append(request.songs(i));
  }

  emit RemoveSongs(request.playlist_id(), songs);
}

void IncomingDataParser::ClientConnect(const cpb::remote::Message& msg,
                                       RemoteClient* client) {
  // Always send the Clementine info
  emit SendClementineInfo(client->id());

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
      emit SendFirstData(client->id(), true);
    } else {
      emit SendFirstData(client->id(), false);
    }
  }
}

void IncomingDataParser::SendPlaylists(RemoteClient* client,
                                       const cpb::remote::Message& msg) {
  const bool include_closed =
      msg.has_request_playlists() && msg.request_playlists().include_closed();
  emit SendPlaylistsRequested(client->id(), include_closed);
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
                 ? QString::fromStdString(req_update.new_playlist_name())
                 : tr("New Playlist"));
    return;
  }
  if (req_update.has_clear_playlist() && req_update.clear_playlist()) {
    emit Clear(req_update.playlist_id());
    return;
  }
  if (req_update.has_new_playlist_name() &&
      !req_update.new_playlist_name().empty())
    emit Rename(req_update.playlist_id(),
                QString::fromStdString(req_update.new_playlist_name()));
  if (req_update.has_favorite())
    emit Favorite(req_update.playlist_id(), req_update.favorite());
}

void IncomingDataParser::RateSong(const cpb::remote::Message& msg) {
  double rating = (double)msg.request_rate_song().rating();
  emit RateCurrentSong(rating);
}

void IncomingDataParser::GlobalSearch(RemoteClient* client,
                                      const cpb::remote::Message& msg) {
  emit DoGlobalSearch(
      client->id(), QStringFromStdString(msg.request_global_search().query()));
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
  song.set_url(RemoteFiles::ParseUrl(pb.url()));
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
  const cpb::remote::RequestAppendFiles& req_append =
      msg.request_append_files();
  const QString relative_path =
      QString::fromStdString(req_append.relative_path());
  const RemoteFiles::DirResult dir =
      RemoteFiles::ResolveDir(files_root_folder_, relative_path);
  if (!dir.ok()) {
    qLog(Warning) << "Cannot append files from remote relative path"
                  << relative_path;
    return;
  }

  QList<QUrl> urls;
  for (const auto& file : req_append.files()) {
    QFileInfo fi(dir.dir, QString::fromStdString(file));
    // Use absoluteFilePath() rather than canonicalFilePath() so symlinks under
    // files_root follow consistently with IsWithin() and MayAdd() (containment
    // is checked on the unresolved path).
    if (fi.exists() && RemoteFiles::IsWithin(dir.root, fi))
      urls << QUrl::fromLocalFile(fi.absoluteFilePath());
  }
  if (urls.isEmpty()) return;

  // A file can be a playlist file, whose entries are checked as they're
  // loaded.
  if (!req_append.has_new_playlist_name() && req_append.has_playlist_id()) {
    emit SetCurrentPlaylistIfNotPlaying(req_append.playlist_id());
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
