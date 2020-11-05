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
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"

#ifdef HAVE_LIBLASTFM
#include "internet/lastfm/lastfmservice.h"
#endif

IncomingDataParser::IncomingDataParser(Application* app) : app_(app) {
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
  connect(this, SIGNAL(InsertUrls(int, const QList<QUrl>&, int, bool, bool)),
          playlist_manager,
          SLOT(InsertUrls(int, const QList<QUrl>&, int, bool, bool)));
  connect(this, SIGNAL(InsertSongs(int, const SongList&, int, bool, bool)),
          playlist_manager,
          SLOT(InsertSongs(int, const SongList&, int, bool, bool)));
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

#ifdef HAVE_LIBLASTFM
  connect(this, SIGNAL(Love()), app_->scrobbler(), SLOT(Love()));
#endif
}

IncomingDataParser::~IncomingDataParser() {}

void IncomingDataParser::ReloadSettings() {
  QSettings s;
  s.beginGroup(MainWindow::kSettingsGroup);
  doubleclick_playlist_addmode_ = MainWindow::PlaylistAddBehaviour(
      s.value("doubleclick_playlist_addmode",
              MainWindow::PlaylistAddBehaviour_Enqueue)
          .toInt());
}

bool IncomingDataParser::close_connection() { return close_connection_; }

void IncomingDataParser::Parse(const pb::remote::Message& msg) {
  close_connection_ = false;

  RemoteClient* client = qobject_cast<RemoteClient*>(sender());

  // Now check what's to do
  switch (msg.type()) {
    case pb::remote::CONNECT:
      ClientConnect(msg, client);
      break;
    case pb::remote::DISCONNECT:
      close_connection_ = true;
      break;
    case pb::remote::REQUEST_PLAYLISTS:
      SendPlaylists(msg);
      break;
    case pb::remote::REQUEST_PLAYLIST_SONGS:
      GetPlaylistSongs(msg);
      break;
    case pb::remote::SET_VOLUME:
      emit SetVolume(msg.request_set_volume().volume());
      break;
    case pb::remote::PLAY:
      emit Play();
      break;
    case pb::remote::PLAYPAUSE:
      emit PlayPause();
      break;
    case pb::remote::PAUSE:
      emit Pause();
      break;
    case pb::remote::STOP:
      emit Stop();
      break;
    case pb::remote::STOP_AFTER:
      emit StopAfterCurrent();
      break;
    case pb::remote::NEXT:
      emit Next();
      break;
    case pb::remote::PREVIOUS:
      emit Previous();
      break;
    case pb::remote::CHANGE_SONG:
      ChangeSong(msg);
      break;
    case pb::remote::SHUFFLE_PLAYLIST:
      emit ShuffleCurrent();
      break;
    case pb::remote::REPEAT:
      SetRepeatMode(msg.repeat());
      break;
    case pb::remote::SHUFFLE:
      SetShuffleMode(msg.shuffle());
      break;
    case pb::remote::SET_TRACK_POSITION:
      emit SeekTo(msg.request_set_track_position().position());
      break;
    case pb::remote::INSERT_URLS:
      InsertUrls(msg);
      break;
    case pb::remote::REMOVE_SONGS:
      RemoveSongs(msg);
      break;
    case pb::remote::OPEN_PLAYLIST:
      OpenPlaylist(msg);
      break;
    case pb::remote::CLOSE_PLAYLIST:
      ClosePlaylist(msg);
      break;
    case pb::remote::UPDATE_PLAYLIST:
      UpdatePlaylist(msg);
      break;
    case pb::remote::LOVE:
      emit Love();
      break;
    case pb::remote::BAN:
      emit Ban();
      break;
    case pb::remote::GET_LYRICS:
      emit GetLyrics();
      break;
    case pb::remote::DOWNLOAD_SONGS:
      client->song_sender()->SendSongs(msg.request_download_songs());
      break;
    case pb::remote::SONG_OFFER_RESPONSE:
      client->song_sender()->ResponseSongOffer(
          msg.response_song_offer().accepted());
      break;
    case pb::remote::GET_LIBRARY:
      emit SendLibrary(client);
      break;
    case pb::remote::RATE_SONG:
      RateSong(msg);
      break;
    case pb::remote::GLOBAL_SEARCH:
      GlobalSearch(client, msg);
      break;
    case pb::remote::REQUEST_FILES:
      emit SendListFiles(
          QString::fromStdString(msg.request_list_files().relative_path()));
      break;
    case pb::remote::APPEND_FILES:
      AppendFilesToPlaylist(msg);
      break;

    default:
      break;
  }
}

void IncomingDataParser::GetPlaylistSongs(const pb::remote::Message& msg) {
  emit SendPlaylistSongs(msg.request_playlist_songs().id());
}

void IncomingDataParser::ChangeSong(const pb::remote::Message& msg) {
  // Get the first entry and check if there is a song
  const pb::remote::RequestChangeSong& request = msg.request_change_song();

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

void IncomingDataParser::SetRepeatMode(const pb::remote::Repeat& repeat) {
  switch (repeat.repeat_mode()) {
    case pb::remote::Repeat_Off:
      emit SetRepeatMode(PlaylistSequence::Repeat_Off);
      break;
    case pb::remote::Repeat_Track:
      emit SetRepeatMode(PlaylistSequence::Repeat_Track);
      break;
    case pb::remote::Repeat_Album:
      emit SetRepeatMode(PlaylistSequence::Repeat_Album);
      break;
    case pb::remote::Repeat_Playlist:
      emit SetRepeatMode(PlaylistSequence::Repeat_Playlist);
      break;
    default:
      break;
  }
}

void IncomingDataParser::SetShuffleMode(const pb::remote::Shuffle& shuffle) {
  switch (shuffle.shuffle_mode()) {
    case pb::remote::Shuffle_Off:
      emit SetShuffleMode(PlaylistSequence::Shuffle_Off);
      break;
    case pb::remote::Shuffle_All:
      emit SetShuffleMode(PlaylistSequence::Shuffle_All);
      break;
    case pb::remote::Shuffle_InsideAlbum:
      emit SetShuffleMode(PlaylistSequence::Shuffle_InsideAlbum);
      break;
    case pb::remote::Shuffle_Albums:
      emit SetShuffleMode(PlaylistSequence::Shuffle_Albums);
      break;
    default:
      break;
  }
}

void IncomingDataParser::InsertUrls(const pb::remote::Message& msg) {
  const pb::remote::RequestInsertUrls& request = msg.request_insert_urls();

  // Insert plain urls without metadata
  if (!request.urls().empty()) {
    QList<QUrl> urls;
    for (auto it = request.urls().begin(); it != request.urls().end(); ++it) {
      std::string s = *it;
      urls << QUrl(QStringFromStdString(s));
    }

    // Insert the urls
    emit InsertUrls(request.playlist_id(), urls, request.position(),
                    request.play_now(), request.enqueue());
  }

  // Add songs with metadata if present
  if (!request.songs().empty()) {
    SongList songs;
    for (int i = 0; i < request.songs().size(); i++) {
      songs << CreateSongFromProtobuf(request.songs(i));
    }
    emit InsertSongs(request.playlist_id(), songs, request.position(),
                     request.play_now(), request.enqueue());
  }
}

void IncomingDataParser::RemoveSongs(const pb::remote::Message& msg) {
  const pb::remote::RequestRemoveSongs& request = msg.request_remove_songs();

  // Extract urls
  QList<int> songs;
  for (int i = 0; i < request.songs().size(); i++) {
    songs.append(request.songs(i));
  }

  // Insert the urls
  emit RemoveSongs(request.playlist_id(), songs);
}

void IncomingDataParser::ClientConnect(const pb::remote::Message& msg,
                                       RemoteClient* client) {
  // Always sned the Clementine infos
  emit SendClementineInfo();

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

void IncomingDataParser::SendPlaylists(const pb::remote::Message& msg) {
  if (!msg.has_request_playlists() ||
      !msg.request_playlists().include_closed()) {
    emit SendAllActivePlaylists();
  } else {
    emit SendAllPlaylists();
  }
}

void IncomingDataParser::OpenPlaylist(const pb::remote::Message& msg) {
  emit Open(msg.request_open_playlist().playlist_id());
}

void IncomingDataParser::ClosePlaylist(const pb::remote::Message& msg) {
  emit Close(msg.request_close_playlist().playlist_id());
}

void IncomingDataParser::UpdatePlaylist(const pb::remote::Message& msg) {
  const pb::remote::RequestUpdatePlaylist& req_update =
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

void IncomingDataParser::RateSong(const pb::remote::Message& msg) {
  double rating = (double)msg.request_rate_song().rating();
  emit RateCurrentSong(rating);
}

void IncomingDataParser::GlobalSearch(RemoteClient* client,
                                      const pb::remote::Message& msg) {
  emit DoGlobalSearch(QStringFromStdString(msg.request_global_search().query()),
                      client);
}

Song IncomingDataParser::CreateSongFromProtobuf(
    const pb::remote::SongMetadata& pb) {
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

void IncomingDataParser::AppendFilesToPlaylist(const pb::remote::Message& msg) {
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

  const pb::remote::RequestAppendFiles& req_append = msg.request_append_files();
  QString relative_path = QString::fromStdString(req_append.relative_path());
  if (relative_path.startsWith("/")) relative_path.remove(0, 1);

  QFileInfo fi_folder(root_dir, relative_path);
  if (!fi_folder.exists())
    qLog(Warning) << "Remote relative path " << relative_path
                  << " doesn't exist...";
  else if (!fi_folder.isDir())
    qLog(Warning) << "Remote relative path " << relative_path
                  << " is not a directory...";
  else if (root_dir.relativeFilePath(fi_folder.absoluteFilePath())
               .startsWith("../"))
    qLog(Warning) << "Remote relative path " << relative_path
                  << " should not be accessed...";
  else {
    QList<QUrl> urls;
    QDir dir(fi_folder.absoluteFilePath());
    for (const auto& file : req_append.files()) {
      QFileInfo fi(dir, file.c_str());
      if (fi.exists()) urls << QUrl::fromLocalFile(fi.canonicalFilePath());
    }
    if (urls.size()) {
      MimeData* data = new MimeData;
      data->setUrls(urls);
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
      }
      emit AddToPlaylistSignal(data);
    }
  }
}
