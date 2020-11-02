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

#include <algorithm>
#include <QDir>

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

  // Connect all the signals
  // due the player is in a different thread, we cannot access these functions
  // directly
  connect(this, SIGNAL(Play()), app_->player(), SLOT(Play()));
  connect(this, SIGNAL(PlayPause()), app_->player(), SLOT(PlayPause()));
  connect(this, SIGNAL(Pause()), app_->player(), SLOT(Pause()));
  connect(this, SIGNAL(Stop()), app_->player(), SLOT(Stop()));
  connect(this, SIGNAL(StopAfterCurrent()), app_->player(),
          SLOT(StopAfterCurrent()));
  connect(this, SIGNAL(Next()), app_->player(), SLOT(Next()));
  connect(this, SIGNAL(Previous()), app_->player(), SLOT(Previous()));
  connect(this, SIGNAL(SetVolume(int)), app_->player(), SLOT(SetVolume(int)));
  connect(this, SIGNAL(PlayAt(int, Engine::TrackChangeFlags, bool)),
          app_->player(), SLOT(PlayAt(int, Engine::TrackChangeFlags, bool)));
  connect(this, SIGNAL(SeekTo(int)), app_->player(), SLOT(SeekTo(int)));
  connect(this, SIGNAL(Enque(int, int)), app_->playlist_manager(),
          SLOT(Enque(int, int)));

  connect(this, SIGNAL(SetActivePlaylist(int)), app_->playlist_manager(),
          SLOT(SetActivePlaylist(int)));
  connect(this, SIGNAL(ShuffleCurrent()), app_->playlist_manager(),
          SLOT(ShuffleCurrent()));
  connect(this, SIGNAL(SetRepeatMode(PlaylistSequence::RepeatMode)),
          app_->playlist_manager()->sequence(),
          SLOT(SetRepeatMode(PlaylistSequence::RepeatMode)));
  connect(this, SIGNAL(SetShuffleMode(PlaylistSequence::ShuffleMode)),
          app_->playlist_manager()->sequence(),
          SLOT(SetShuffleMode(PlaylistSequence::ShuffleMode)));
  connect(this, SIGNAL(InsertUrls(int, const QList<QUrl>&, int, bool, bool)),
          app_->playlist_manager(),
          SLOT(InsertUrls(int, const QList<QUrl>&, int, bool, bool)));
  connect(this, SIGNAL(InsertSongs(int, const SongList&, int, bool, bool)),
          app_->playlist_manager(),
          SLOT(InsertSongs(int, const SongList&, int, bool, bool)));
  connect(this, SIGNAL(RemoveSongs(int, const QList<int>&)),
          app_->playlist_manager(),
          SLOT(RemoveItemsWithoutUndo(int, const QList<int>&)));
  connect(this, SIGNAL(Open(int)), app_->playlist_manager(), SLOT(Open(int)));
  connect(this, SIGNAL(Close(int)), app_->playlist_manager(), SLOT(Close(int)));

  connect(this, SIGNAL(RateCurrentSong(double)), app_->playlist_manager(),
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

qLog(Debug) << "[MB_TRACE][IncomingDataParser::Parse] type: " << msg.type();

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
qLog(Debug) << "[MB_TRACE] REQUEST_FILES: " << msg.request_list_files().relative_path().c_str();
      emit SendListFiles(QString::fromStdString(msg.request_list_files().relative_path()));
    break;
  case pb::remote::APPEND_FILES:
qLog(Debug) << "[MB_TRACE] APPEND_FILES from : " << msg.request_append_files().relative_path().c_str()
            << ", nb: " << msg.request_append_files().files_size();
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

void IncomingDataParser::AppendFilesToPlaylist(const pb::remote::Message &msg)
{
    if (remote_root_files_.isEmpty()) {// should never happen...
        qLog(Warning) << "Remote root dir is not set although receiving APPEND_FILES request...";
        return;
    }
    QDir rootDir(remote_root_files_);
    if (!rootDir.exists()) {
        qLog(Warning) << "Remote root dir doesn't exist...";
        return;
    }

    const pb::remote::RequestAppendFiles &reqAppend = msg.request_append_files();
    QString relativePath = QString::fromStdString(reqAppend.relative_path());
    if (relativePath.startsWith("/"))
        relativePath.remove(0, 1);

    QFileInfo fiFolder(rootDir, relativePath);
    if (!fiFolder.exists())
        qLog(Warning) << "Remote relative path "<< relativePath << " doesn't exist...";
    else if (!fiFolder.isDir())
        qLog(Warning) << "Remote relative path "<< relativePath << " is not a directory...";
    else if (rootDir.relativeFilePath(fiFolder.absoluteFilePath()).startsWith("../"))
        qLog(Warning) << "Remote relative path "<< relativePath << " should not be accessed...";
    else {
        QList<QUrl> urls;
        QDir dir(fiFolder.absoluteFilePath());
        for (auto it = reqAppend.files().cbegin(), itEnd = reqAppend.files().cend();
             it != itEnd; ++it) {
            QFileInfo fi(dir, it->c_str());
            if (fi.exists())
                urls << QUrl::fromLocalFile(fi.canonicalFilePath());
        }
        if (urls.size()) {
            MimeData* data = new MimeData;
            data->setUrls(urls);
            if (reqAppend.has_play_now())
                data->play_now_ = reqAppend.play_now();
            if (reqAppend.has_clear_first())
                data->clear_first_ = reqAppend.clear_first();
            if (reqAppend.has_new_playlist_name())
            {
                QString playlistName = QString::fromStdString(reqAppend.new_playlist_name());
                if (!playlistName.isEmpty())
                {
                    data->open_in_new_playlist_  = true;
                    data->name_for_new_playlist_ = playlistName;
                }
            }
            emit AddToPlaylistSignal(data);
        }
    }
}
