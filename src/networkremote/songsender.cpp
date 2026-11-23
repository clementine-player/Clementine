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

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <limits>

#include "core/application.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "networkremote/networkremote.h"
#include "networkremote/outgoingdatacreator.h"
#include "networkremote/remoteclient.h"
#include "networkremote/remotefiles.h"
#include "playlist/playlistitem.h"

const quint32 SongSender::kFileChunkSize = 100000;  // in Bytes

namespace {

constexpr qint64 kMaxBufferedChunkBytes = 512 * 1024;

// Replace only the final suffix so dots earlier in the filename aren't lost.
QString TranscodedFilename(const QString& basefilename, const QString& ext) {
  if (ext.isEmpty()) return basefilename;
  const int dot = basefilename.lastIndexOf('.');
  if (dot <= 0) return basefilename + '.' + ext;
  return basefilename.left(dot + 1) + ext;
}

}  // namespace

SongSender::SongSender(Application* app, RemoteClient* client)
    : app_(app),
      client_(client),
      has_transcoder_preset_(false),
      transcoder_(
          new Transcoder(this, NetworkRemote::kTranscoderSettingPostfix)),
      transcode_lossless_files_(false),
      total_transcode_(0),
      transcode_processed_(0),
      current_row_(-1),
      active_item_(Song(), 0, 0),
      active_file_size_(0),
      active_chunk_count_(0),
      active_chunk_number_(0),
      active_is_transcoded_(false) {
  QSettings s;
  s.beginGroup(NetworkRemote::kSettingsGroup);

  transcode_lossless_files_ = s.value("convert_lossless", false).toBool();

  // Load preset
  QString last_output_format =
      s.value("last_output_format", "audio/x-vorbis").toString();
  QList<TranscoderPreset> presets = transcoder_->GetAllPresets();
  for (int i = 0; i < presets.count(); ++i) {
    if (last_output_format == presets.at(i).codec_mimetype_) {
      transcoder_preset_ = presets.at(i);
      has_transcoder_preset_ = true;
      break;
    }
  }
  if (!has_transcoder_preset_ && !presets.isEmpty()) {
    transcoder_preset_ = presets.first();
    has_transcoder_preset_ = true;
  }
  qLog(Debug) << "Transcoder preset" << transcoder_preset_.codec_mimetype_;

  connect(transcoder_, SIGNAL(JobComplete(QUrl, QString, bool)),
          SLOT(TranscodeJobComplete(QUrl, QString, bool)));
  connect(transcoder_, SIGNAL(AllJobsComplete()), SLOT(StartTransfer()));
  if (client_) {
    connect(client_, &RemoteClient::BytesWritten, this,
            &SongSender::PumpNextChunk);
  }
}

SongSender::~SongSender() {
  // Disconnect first so Cancel() cannot re-enter slots during teardown;
  // Cancel() and CleanupTranscodedFiles() then delete unfinished temp files.
  disconnect(transcoder_, SIGNAL(JobComplete(QUrl, QString, bool)), this,
             SLOT(TranscodeJobComplete(QUrl, QString, bool)));
  disconnect(transcoder_, SIGNAL(AllJobsComplete()), this,
             SLOT(StartTransfer()));
  transcoder_->Cancel();
  if (active_file_) {
    active_file_->close();
    active_file_.reset();
  }
  CleanupTranscodedFiles();
}

void SongSender::SendSongs(const cpb::remote::RequestDownloadSongs& request,
                           const Song& current_song, int current_row,
                           const SongList& playlist_songs) {
  // Reject overlapping downloads until the active queue/transcode finishes.
  if (active_file_ || !download_queue_.isEmpty() ||
      total_transcode_ > transcode_processed_) {
    qLog(Warning)
        << "Ignoring download request while another transfer is active";
    return;
  }

  current_row_ = current_row;

  switch (request.download_item()) {
    case cpb::remote::CurrentItem: {
      if (current_song.is_valid() && current_song.url().scheme() == "file") {
        DownloadItem item(current_song, 1, 1);
        download_queue_.append(item);
      }
      break;
    }
    case cpb::remote::ItemAlbum:
      if (current_song.is_valid()) {
        SendAlbum(current_song);
      }
      break;
    case cpb::remote::APlaylist:
      SendPlaylist(request, playlist_songs);
      break;
    case cpb::remote::Urls:
      SendUrls(request);
      break;
    default:
      break;
  }

  if (transcode_lossless_files_ && has_transcoder_preset_) {
    TranscodeLosslessFiles();
  } else {
    StartTransfer();
  }
}

void SongSender::TranscodeLosslessFiles() {
  QSet<QString> scheduled;
  for (const DownloadItem& item : download_queue_) {
    // Check only local lossless files.
    if (!item.song_.IsFileLossless() || !item.song_.url().isLocalFile()) {
      continue;
    }

    const QUrl local_url = item.song_.url();
    const QString local_file = local_url.toLocalFile();
    // Ref-count duplicate tracks in the queue so one transcode serves all
    // occurrences and isn't deleted until the last one finishes or declines.
    transcode_refs_[local_file] += 1;

    if (scheduled.contains(local_file) ||
        transcoder_map_.contains(local_file)) {
      continue;
    }
    scheduled.insert(local_file);

    transcoder_->AddTemporaryJob(local_url, transcoder_preset_);
    qLog(Debug) << "transcoding" << local_file;
    total_transcode_++;
  }

  if (total_transcode_ > 0) {
    transcode_processed_ = 0;
    transcoder_->Start();
    SendTranscoderStatus();
  } else {
    StartTransfer();
  }
}

void SongSender::TranscodeJobComplete(const QUrl& input, const QString& output,
                                      bool success) {
  if (!input.isLocalFile()) return;
  qLog(Debug) << input.toLocalFile() << "transcoded to" << output << success;

  transcode_processed_++;
  // If it wasn't successful send original file
  if (success) {
    transcoder_map_.insert(input.toLocalFile(), output);
  }

  SendTranscoderStatus();
}

void SongSender::SendTranscoderStatus() {
  // Send a message to the remote that we are converting files
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::TRANSCODING_FILES);

  cpb::remote::ResponseTranscoderStatus* status =
      msg.mutable_response_transcoder_status();
  status->set_processed(transcode_processed_);
  status->set_total(total_transcode_);

  client_->SendData(&msg);
}

void SongSender::StartTransfer() {
  total_transcode_ = 0;
  transcode_processed_ = 0;

  // Send total file size & file count
  SendTotalFileSize();

  // Send first file
  OfferNextSong();
}

void SongSender::SendTotalFileSize() {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::DOWNLOAD_TOTAL_SIZE);

  cpb::remote::ResponseDownloadTotalSize* response =
      msg.mutable_response_download_total_size();

  response->set_file_count(download_queue_.size());

  qint64 total = 0;
  for (const DownloadItem& item : download_queue_) {
    QString local_file = item.song_.url().toLocalFile();
    if (transcoder_map_.contains(local_file)) {
      local_file = transcoder_map_.value(local_file);
    }
    total += QFileInfo(local_file).size();
  }

  // Cap at INT32_MAX because ResponseDownloadTotalSize::total_size is int32.
  response->set_total_size(static_cast<qint32>(
      qMin<qint64>(total, std::numeric_limits<qint32>::max())));

  client_->SendData(&msg);
}

void SongSender::OfferNextSong() {
  cpb::remote::Message msg;

  if (download_queue_.isEmpty()) {
    msg.set_type(cpb::remote::DOWNLOAD_QUEUE_EMPTY);
  } else {
    // Get the item and send the single song offer
    DownloadItem item = download_queue_.head();

    msg.set_type(cpb::remote::SONG_FILE_CHUNK);
    cpb::remote::ResponseSongFileChunk* chunk =
        msg.mutable_response_song_file_chunk();

    QString local_file = item.song_.url().toLocalFile();
    const bool is_transcoded = transcoder_map_.contains(local_file);
    if (is_transcoded) {
      local_file = transcoder_map_.value(local_file);
    }
    const qint64 file_size = QFileInfo(local_file).size();

    // Song offer is chunk no 0
    chunk->set_chunk_count(0);
    chunk->set_chunk_number(0);
    chunk->set_file_count(item.song_count_);
    chunk->set_file_number(item.song_no_);
    chunk->set_size(file_size);

    cpb::remote::SongMetadata* song_metadata = chunk->mutable_song_metadata();
    OutgoingDataCreator::CreateSong(item.song_, QImage(), -1, song_metadata);
    // Advertise the transcoded filename and size on chunk 0 so clients
    // match it against chunk 1 instead of saving OGG/MP3 bytes as .flac.
    if (is_transcoded) {
      song_metadata->set_file_size(file_size);
      song_metadata->set_filename(DataCommaSizeFromQString(TranscodedFilename(
          item.song_.basefilename(), transcoder_preset_.extension_)));
    }
  }

  client_->SendData(&msg);
}

void SongSender::ResponseSongOffer(bool accepted) {
  // Ignore stale or out-of-order offer replies while a file is pumping.
  if (active_file_ || download_queue_.isEmpty()) return;

  // Get the item and send the single song
  DownloadItem item = download_queue_.dequeue();
  if (accepted) {
    SendSingleSong(item);
  } else {
    ReleaseTranscodedRef(item.song_.url().toLocalFile());
    OfferNextSong();
  }
}

void SongSender::SendSingleSong(const DownloadItem& download_item) {
  // Only local files!!!
  if (download_item.song_.url().scheme() != "file") {
    ReleaseTranscodedRef(download_item.song_.url().toLocalFile());
    OfferNextSong();
    return;
  }

  const QString source_path = download_item.song_.url().toLocalFile();
  const bool is_transcoded = transcoder_map_.contains(source_path);
  const QString local_file =
      is_transcoded ? transcoder_map_.value(source_path) : source_path;

  // Open the file and compute sha1 while closed, as Sha1File opens/closes it.
  QFile sha_file(local_file);
  const QByteArray sha1 = Utilities::Sha1File(sha_file).toHex();
  qLog(Debug) << "sha1 for file" << local_file << "=" << sha1;

  std::unique_ptr<QFile> file(new QFile(local_file));
  if (!file->open(QIODevice::ReadOnly) || file->size() <= 0) {
    qLog(Warning) << "Failed to open song for remote download:" << local_file;
    ReleaseTranscodedRef(source_path);
    OfferNextSong();
    return;
  }

  active_file_ = std::move(file);
  active_item_ = download_item;
  active_source_path_ = source_path;
  active_sha1_ = sha1;
  active_file_size_ = active_file_->size();
  // Ceiling division avoids an extra chunk on exact kFileChunkSize multiples.
  active_chunk_count_ = static_cast<int>(
      (active_file_size_ + kFileChunkSize - 1) / kFileChunkSize);
  active_chunk_number_ = 1;
  active_is_transcoded_ = is_transcoded;

  PumpNextChunk();
}

void SongSender::PumpNextChunk() {
  if (!active_file_) return;

  if (!client_ || client_->is_closing() ||
      client_->State() != QTcpSocket::ConnectedState) {
    FinishActiveTransfer(false);
    return;
  }

  cpb::remote::Message msg;
  cpb::remote::ResponseSongFileChunk* chunk =
      msg.mutable_response_song_file_chunk();
  msg.set_type(cpb::remote::SONG_FILE_CHUNK);
  QImage null_image;

  // Pump chunks on BytesWritten up to kMaxBufferedChunkBytes to avoid
  // buffering whole audio files in memory.
  while (active_file_->isOpen() && !active_file_->atEnd() &&
         client_->bytesToWrite() < kMaxBufferedChunkBytes) {
    const QByteArray data = active_file_->read(kFileChunkSize);
    if (data.isEmpty()) break;

    chunk->set_chunk_count(active_chunk_count_);
    chunk->set_chunk_number(active_chunk_number_);
    chunk->set_file_count(active_item_.song_count_);
    chunk->set_file_number(active_item_.song_no_);
    chunk->set_size(active_file_size_);
    chunk->set_data(data.data(), data.size());
    chunk->set_file_hash(active_sha1_.data(), active_sha1_.size());

    // On the first chunk send the metadata, so the client knows
    // what file it receives.
    if (active_chunk_number_ == 1) {
      cpb::remote::SongMetadata* song_metadata =
          msg.mutable_response_song_file_chunk()->mutable_song_metadata();
      OutgoingDataCreator::CreateSong(active_item_.song_, null_image,
                                      current_row_, song_metadata);

      // if the file was transcoded, we have to change the filename and filesize
      if (active_is_transcoded_) {
        song_metadata->set_file_size(active_file_size_);
        song_metadata->set_filename(DataCommaSizeFromQString(TranscodedFilename(
            active_item_.song_.basefilename(), transcoder_preset_.extension_)));
      }
    }

    client_->SendData(&msg);
    chunk->Clear();
    active_chunk_number_++;
  }

  if (!active_file_->isOpen() || active_file_->atEnd() ||
      active_file_->error() != QFile::NoError) {
    FinishActiveTransfer(true);
  }
}

void SongSender::FinishActiveTransfer(bool offer_next) {
  if (!active_file_) return;
  active_file_->close();
  active_file_.reset();
  if (!active_source_path_.isEmpty()) {
    ReleaseTranscodedRef(active_source_path_);
  }
  active_source_path_.clear();
  active_sha1_.clear();
  active_file_size_ = 0;
  active_chunk_count_ = 0;
  active_chunk_number_ = 0;
  active_is_transcoded_ = false;

  if (offer_next) {
    OfferNextSong();
  }
}

void SongSender::ReleaseTranscodedRef(const QString& local_file) {
  auto ref_it = transcode_refs_.find(local_file);
  if (ref_it != transcode_refs_.end()) {
    if (--(*ref_it) > 0) return;
    transcode_refs_.erase(ref_it);
  }
  const QString temp_file = transcoder_map_.take(local_file);
  if (!temp_file.isEmpty()) {
    QFile::remove(temp_file);
  }
}

void SongSender::CleanupTranscodedFiles() {
  for (const QString& temp_file : transcoder_map_) {
    if (!temp_file.isEmpty()) QFile::remove(temp_file);
  }
  transcoder_map_.clear();
  transcode_refs_.clear();
}

void SongSender::SendAlbum(const Song& song) {
  // No streams!
  if (song.url().scheme() != "file" || !app_) return;

  const SongList album = app_->library_backend()->GetSongsByAlbum(song.album());
  SongList local_songs;
  for (const Song& s : album) {
    if (s.url().scheme() == "file") local_songs.append(s);
  }

  int song_no = 1;
  for (const Song& s : local_songs) {
    download_queue_.append(DownloadItem(s, song_no++, local_songs.size()));
  }
}

void SongSender::SendPlaylist(const cpb::remote::RequestDownloadSongs& request,
                              const SongList& song_list) {
  QList<int> requested_ids;
  for (auto song_id : request.songs_ids()) requested_ids << song_id;

  // Count only matching local files so file_number never exceeds file_count.
  int count = 0;
  for (const Song& s : song_list) {
    if (s.url().scheme() == "file" &&
        (requested_ids.isEmpty() || requested_ids.contains(s.id()))) {
      count++;
    }
  }

  int song_no = 1;
  for (const Song& s : song_list) {
    // Only local files!
    if (s.url().scheme() == "file" &&
        (requested_ids.isEmpty() || requested_ids.contains(s.id()))) {
      download_queue_.append(DownloadItem(s, song_no++, count));
    }
  }
}

void SongSender::SendUrls(const cpb::remote::RequestDownloadSongs& request) {
  SongList song_list;

  // First gather all valid songs
  if (request.has_relative_path()) {
    const RemoteFiles::DirResult dir = RemoteFiles::ResolveDir(
        client_->files_root_folder(),
        QString::fromStdString(request.relative_path()));
    if (!dir.ok()) return;

    const QStringList& files_music_extensions =
        client_->files_music_extensions();
    for (const std::string& s : request.urls()) {
      QFileInfo fi(dir.dir, QString::fromStdString(s));
      if (fi.exists() && fi.isFile() && RemoteFiles::IsWithin(dir.root, fi) &&
          files_music_extensions.contains(fi.suffix(), Qt::CaseInsensitive)) {
        Song song;
        song.set_basefilename(fi.fileName());
        song.set_filesize(fi.size());
        song.set_url(QUrl::fromLocalFile(fi.absoluteFilePath()));
        song.set_valid(true);
        song_list.append(song);
      }
    }
  } else if (app_) {
    for (const std::string& s : request.urls()) {
      // Preserve percent-encoded URLs (e.g. %23/%3F) from the library.
      const QUrl url = RemoteFiles::ParseUrl(s);
      Song song = app_->library_backend()->GetSongByUrl(url);
      if (song.is_valid() && song.url().scheme() == "file") {
        song_list.append(song);
      }
    }
  }

  // Then send them to Clementine Remote
  // Increment sequentially so duplicate tracks get distinct file_numbers.
  int song_no = 1;
  for (const Song& s : song_list) {
    download_queue_.append(DownloadItem(s, song_no++, song_list.count()));
  }
}
