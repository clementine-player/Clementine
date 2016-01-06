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

#include "songsender.h"

#include "networkremote.h"

#include <QFileInfo>

#include "core/application.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "playlist/playlistitem.h"

const quint32 SongSender::kFileChunkSize = 100000;  // in Bytes

SongSender::SongSender(Application* app, RemoteClient* client)
    : app_(app),
      client_(client),
      transcoder_(new Transcoder(this, NetworkRemote::kTranscoderSettingPostfix)) {
  QSettings s;
  s.beginGroup(NetworkRemote::kSettingsGroup);

  transcode_lossless_files_ = s.value("convert_lossless", false).toBool();

  // Load preset
  QString last_output_format = s.value("last_output_format", "audio/x-vorbis").toString();
  QList<TranscoderPreset> presets = transcoder_->GetAllPresets();
  for (int i = 0; i<presets.count(); ++i) {
    if (last_output_format == presets.at(i).codec_mimetype_) {
      transcoder_preset_ = presets.at(i);
      break;
    }
  }
  qLog(Debug) << "Transcoder preset" << transcoder_preset_.codec_mimetype_;

  connect(transcoder_, SIGNAL(JobComplete(QString, QString, bool)),
          SLOT(TranscodeJobComplete(QString, QString, bool)));
  connect(transcoder_, SIGNAL(AllJobsComplete()), SLOT(StartTransfer()));

  total_transcode_ = 0;
}

SongSender::~SongSender() {
  disconnect(transcoder_, SIGNAL(JobComplete(QString, QString, bool)), this,
          SLOT(TranscodeJobComplete(QString, QString, bool)));
  disconnect(transcoder_, SIGNAL(AllJobsComplete()), this, SLOT(StartTransfer()));
  transcoder_->Cancel();
}

void SongSender::SendSongs(const pb::remote::RequestDownloadSongs& request) {
  Song current_song;
  if (app_->player()->GetCurrentItem()) {
    current_song = app_->player()->GetCurrentItem()->Metadata();
  }

  switch (request.download_item()) {
    case pb::remote::CurrentItem: {
      if (current_song.is_valid()) {
        DownloadItem item(current_song, 1, 1);
        download_queue_.append(item);
      }
      break;
    }
    case pb::remote::ItemAlbum:
      if (current_song.is_valid()) {
        SendAlbum(current_song);
      }
      break;
    case pb::remote::APlaylist:
      SendPlaylist(request.playlist_id());
      break;
    case pb::remote::Urls:
      SendUrls(request);
      break;
    default:
      break;
  }

  if (transcode_lossless_files_) {
    TranscodeLosslessFiles();
  } else {
    StartTransfer();
  }
}

void SongSender::TranscodeLosslessFiles() {
  for (DownloadItem item : download_queue_) {
    // Check only lossless files
    if (!item.song_.IsFileLossless())
      continue;

    // Add the file to the transcoder
    QString local_file = item.song_.url().toLocalFile();

    transcoder_->AddTemporaryJob(local_file, transcoder_preset_);

    qLog(Debug) << "transcoding" << local_file;
    total_transcode_++;
  }

  if (total_transcode_ > 0) {
    transcoder_->Start();
    SendTranscoderStatus();
  } else {
    StartTransfer();
  }
}

void SongSender::TranscodeJobComplete(const QString& input, const QString& output, bool success) {
  qLog(Debug) << input << "transcoded to" << output << success;

  // If it wasn't successful send original file
  if (success) {
    transcoder_map_.insert(input, output);
  }

  SendTranscoderStatus();
}

void SongSender::SendTranscoderStatus() {
  // Send a message to the remote that we are converting files
  pb::remote::Message msg;
  msg.set_type(pb::remote::TRANSCODING_FILES);

  pb::remote::ResponseTranscoderStatus* status =
      msg.mutable_response_transcoder_status();
  status->set_processed(transcoder_map_.count());
  status->set_total(total_transcode_);

  client_->SendData(&msg);
}

void SongSender::StartTransfer() {
  total_transcode_ = 0;

  // Send total file size & file count
  SendTotalFileSize();

  // Send first file
  OfferNextSong();
}

void SongSender::SendTotalFileSize() {
  pb::remote::Message msg;
  msg.set_type(pb::remote::DOWNLOAD_TOTAL_SIZE);

  pb::remote::ResponseDownloadTotalSize* response =
      msg.mutable_response_download_total_size();

  response->set_file_count(download_queue_.size());

  int total = 0;
  for (DownloadItem item : download_queue_) {
    QString local_file = item.song_.url().toLocalFile();
    bool is_transcoded = transcoder_map_.contains(local_file);

    if (is_transcoded) {
      local_file = transcoder_map_.value(local_file);
    }
    total += QFileInfo(local_file).size();
  }

  response->set_total_size(total);

  client_->SendData(&msg);
}

void SongSender::OfferNextSong() {
  pb::remote::Message msg;

  if (download_queue_.isEmpty()) {
    msg.set_type(pb::remote::DOWNLOAD_QUEUE_EMPTY);
  } else {
    // Get the item and send the single song
    DownloadItem item = download_queue_.head();

    msg.set_type(pb::remote::SONG_FILE_CHUNK);
    pb::remote::ResponseSongFileChunk* chunk =
        msg.mutable_response_song_file_chunk();

    // Open the file
    QFile file(item.song_.url().toLocalFile());

    // Song offer is chunk no 0
    chunk->set_chunk_count(0);
    chunk->set_chunk_number(0);
    chunk->set_file_count(item.song_count_);
    chunk->set_file_number(item.song_no_);
    chunk->set_size(file.size());

    OutgoingDataCreator::CreateSong(item.song_, QImage(), -1, chunk->mutable_song_metadata());
  }

  client_->SendData(&msg);
}

void SongSender::ResponseSongOffer(bool accepted) {
  if (download_queue_.isEmpty()) return;

  // Get the item and send the single song
  DownloadItem item = download_queue_.dequeue();
  if (accepted)
    SendSingleSong(item);

  // And offer the next song
  OfferNextSong();
}

void SongSender::SendSingleSong(DownloadItem download_item) {
  // Only local files!!!
  if (!(download_item.song_.url().scheme() == "file")) return;

  QString local_file = download_item.song_.url().toLocalFile();
  bool is_transcoded = transcoder_map_.contains(local_file);

  if (is_transcoded) {
    local_file = transcoder_map_.take(local_file);
  }

  // Open the file
  QFile file(local_file);

  // Get sha1 for file
  QByteArray sha1 = Utilities::Sha1File(file).toHex();
  qLog(Debug) << "sha1 for file" << local_file << "=" << sha1;

  file.open(QIODevice::ReadOnly);

  QByteArray data;
  pb::remote::Message msg;
  pb::remote::ResponseSongFileChunk* chunk =
      msg.mutable_response_song_file_chunk();
  msg.set_type(pb::remote::SONG_FILE_CHUNK);

  QImage null_image;

  // Calculate the number of chunks
  int chunk_count = qRound((file.size() / kFileChunkSize) + 0.5);
  int chunk_number = 1;

  while (!file.atEnd()) {
    // Read file chunk
    data = file.read(kFileChunkSize);

    // Set chunk data
    chunk->set_chunk_count(chunk_count);
    chunk->set_chunk_number(chunk_number);
    chunk->set_file_count(download_item.song_count_);
    chunk->set_file_number(download_item.song_no_);
    chunk->set_size(file.size());
    chunk->set_data(data.data(), data.size());
    chunk->set_file_hash(sha1.data(), sha1.size());

    // On the first chunk send the metadata, so the client knows
    // what file it receives.
    if (chunk_number == 1) {
      int i = app_->playlist_manager()->active()->current_row();
      pb::remote::SongMetadata* song_metadata =
          msg.mutable_response_song_file_chunk()->mutable_song_metadata();
      OutgoingDataCreator::CreateSong(download_item.song_, null_image, i,song_metadata);

      // if the file was transcoded, we have to change the filename and filesize
      if (is_transcoded) {
        song_metadata->set_file_size(file.size());
        QString basefilename = download_item.song_.basefilename();
        QFileInfo info(basefilename);
        basefilename.replace("." + info.suffix(),
                             "." + transcoder_preset_.extension_);
        song_metadata->set_filename(DataCommaSizeFromQString(basefilename));
      }
    }

    // Send data directly to the client
    client_->SendData(&msg);

    // Clear working data
    chunk->Clear();
    data.clear();

    chunk_number++;
  }

  // If the file was transcoded, delete the temporary one
  if (is_transcoded) {
    file.remove();
  } else {
    file.close();
  }
}

void SongSender::SendAlbum(const Song& song) {
  // No streams!
  if (song.url().scheme() != "file") return;

  SongList album = app_->library_backend()->GetSongsByAlbum(song.album());

  for (Song s : album) {
    DownloadItem item(s, album.indexOf(s) + 1, album.size());
    download_queue_.append(item);
  }
}

void SongSender::SendPlaylist(int playlist_id) {
  Playlist* playlist = app_->playlist_manager()->playlist(playlist_id);
  if (!playlist) {
    qLog(Info) << "Could not find playlist with id = " << playlist_id;
    return;
  }
  SongList song_list = playlist->GetAllSongs();

  // Count the local songs
  int count = 0;
  for (Song s : song_list) {
    if (s.url().scheme() == "file") {
      count++;
    }
  }

  for (Song s : song_list) {
    // Only local files!
    if (s.url().scheme() == "file") {
      DownloadItem item(s, song_list.indexOf(s) + 1, count);
      download_queue_.append(item);
    }
  }
}

void SongSender::SendUrls(const pb::remote::RequestDownloadSongs &request) {
  SongList song_list;

  // First gather all valid songs
  for (auto it = request.urls().begin(); it != request.urls().end(); ++it) {
    std::string s = *it;
    QUrl url = QUrl(QStringFromStdString(s));

    Song song = app_->library_backend()->GetSongByUrl(url);

    if (song.is_valid() && song.url().scheme() == "file") {
      song_list.append(song);
    }
  }

  // Then send them to Clementine Remote
  for (Song s : song_list) {
    DownloadItem item(s, song_list.indexOf(s) + 1, song_list.count());
    download_queue_.append(item);
  }
}
