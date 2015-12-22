/* This file is part of Clementine.
 Copyright 2014, Andre Siviero <altsiviero@gmail.com>

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

#include "ripper.h"

#include <QFile>
#include <QMutexLocker>
#include <QtConcurrentRun>

#include "core/closure.h"
#include "core/logging.h"
#include "core/tagreaderclient.h"
#include "transcoder/transcoder.h"
#include "core/utilities.h"

// winspool.h defines this :(
#ifdef AddJob
#undef AddJob
#endif

namespace {
const char kWavHeaderRiffMarker[] = "RIFF";
const char kWavFileTypeFormatChunk[] = "WAVEfmt ";
const char kWavDataString[] = "data";
}  // namespace

Ripper::Ripper(QObject* parent)
    : QObject(parent),
      transcoder_(new Transcoder(this)),
      cancel_requested_(false),
      finished_success_(0),
      finished_failed_(0),
      files_tagged_(0) {
  cdio_ = cdio_open(NULL, DRIVER_UNKNOWN);

  connect(this, SIGNAL(RippingComplete()), transcoder_, SLOT(Start()));
  connect(transcoder_, SIGNAL(JobComplete(QString, QString, bool)),
          SLOT(TranscodingJobComplete(QString, QString, bool)));
  connect(transcoder_, SIGNAL(AllJobsComplete()),
          SLOT(AllTranscodingJobsComplete()));
  connect(transcoder_, SIGNAL(LogLine(QString)), SLOT(LogLine(QString)));
}

Ripper::~Ripper() { cdio_destroy(cdio_); }

void Ripper::AddTrack(int track_number, const QString& title,
                      const QString& transcoded_filename,
                      const TranscoderPreset& preset) {
  if (track_number < 1 || track_number > TracksOnDisc()) {
    qLog(Warning) << "Invalid track number:" << track_number << "Ignoring";
    return;
  }
  TrackInformation track(track_number, title, transcoded_filename, preset);
  tracks_.append(track);
}

void Ripper::SetAlbumInformation(const QString& album, const QString& artist,
                                 const QString& genre, int year, int disc,
                                 Song::FileType type) {
  album_.album = album;
  album_.artist = artist;
  album_.genre = genre;
  album_.year = year;
  album_.disc = disc;
  album_.type = type;
}

int Ripper::TracksOnDisc() const {
  int number_of_tracks = cdio_get_num_tracks(cdio_);
  // Return zero tracks if there is an error, e.g. no medium found.
  if (number_of_tracks == CDIO_INVALID_TRACK) number_of_tracks = 0;
  return number_of_tracks;
}

int Ripper::TrackDurationSecs(int track) const {
  Q_ASSERT(track <= TracksOnDisc());

  int first_frame = cdio_get_track_lsn(cdio_, track);
  int last_frame = cdio_get_track_last_lsn(cdio_, track);
  if (first_frame != CDIO_INVALID_LSN && last_frame != CDIO_INVALID_LSN) {
    return (last_frame - first_frame + 1) / CDIO_CD_FRAMES_PER_SEC;
  } else {
    qLog(Error) << "Could not compute duration of track" << track;
    return 0;
  }
}

int Ripper::AddedTracks() const { return tracks_.length(); }

void Ripper::ClearTracks() { tracks_.clear(); }

bool Ripper::CheckCDIOIsValid() {
  if (cdio_) {
    cdio_destroy(cdio_);
  }
  cdio_ = cdio_open(NULL, DRIVER_UNKNOWN);
  // Refresh the status of the cd media. This will prevent unnecessary
  // rebuilds of the track list table.
  if (cdio_) {
    cdio_get_media_changed(cdio_);
  }
  return cdio_;
}

bool Ripper::MediaChanged() const {
  if (cdio_ && cdio_get_media_changed(cdio_))
    return true;
  else
    return false;
}

void Ripper::Start() {
  {
    QMutexLocker l(&mutex_);
    cancel_requested_ = false;
  }
  SetupProgressInterval();

  qLog(Debug) << "Ripping" << AddedTracks() << "tracks.";
  QtConcurrent::run(this, &Ripper::Rip);
}

void Ripper::Cancel() {
  {
    QMutexLocker l(&mutex_);
    cancel_requested_ = true;
  }
  transcoder_->Cancel();
  RemoveTemporaryDirectory();
  emit(Cancelled());
}

void Ripper::TranscodingJobComplete(const QString& input, const QString& output,
                                    bool success) {
  if (success)
    finished_success_++;
  else
    finished_failed_++;
  UpdateProgress();

  // The the transcoder does not overwrite files. Instead, it changes
  // the name of the output file. We need to update the transcoded
  // filename for the corresponding track so that we tag the correct
  // file later on.
  for (QList<TrackInformation>::iterator it = tracks_.begin();
       it != tracks_.end(); ++it) {
    if (it->temporary_filename == input) {
      it->transcoded_filename = output;
    }
  }
}

void Ripper::AllTranscodingJobsComplete() {
  RemoveTemporaryDirectory();
  TagFiles();
}

void Ripper::LogLine(const QString& message) { qLog(Debug) << message; }

/*
 * WAV Header documentation
 * as taken from:
 * http://www.topherlee.com/software/pcm-tut-wavformat.html
 * Pos   Value   Description
 * 0-3 | "RIFF" |  Marks the file as a riff file.
 *              |  Characters are each 1 byte long.
 * 4-7 | File size (integer) | Size of the overall file - 8 bytes,
 *                           | in bytes (32-bit integer).
 * 8-11  | "WAVE" | File Type Header. For our purposes,
 *                | it always equals "WAVE".
 * 13-16 | "fmt " | Format chunk marker. Includes trailing null.
 * 17-20 |  16   | Length of format data as listed above
 * 21-22 |   1   | Type of format (1 is PCM) - 2 byte integer
 * 23-24 |   2   | Number of Channels - 2 byte integer
 * 25-28 | 44100 | Sample Rate - 32 byte integer. Common values
 *               | are 44100 (CD), 48000 (DAT).
 *               | Sample Rate = Number of Samples per second, or Hertz.
 * 29-32 | 176400 |  (Sample Rate * BitsPerSample * Channels) / 8.
 * 33-34 | 4 |  (BitsPerSample * Channels) / 8.1 - 8 bit mono2 - 8 bit stereo/16
 * bit mono4 - 16 bit stereo
 * 35-36 | 16 |  Bits per sample
 * 37-40 | "data" | "data" chunk header.
 *                | Marks the beginning of the data section.
 * 41-44 | File size (data) | Size of the data section.
 */
void Ripper::WriteWAVHeader(QFile* stream, int32_t i_bytecount) {
  QDataStream data_stream(stream);
  data_stream.setByteOrder(QDataStream::LittleEndian);
  // sizeof() - 1 to avoid including "\0" in the file too
  data_stream.writeRawData(kWavHeaderRiffMarker,
                           sizeof(kWavHeaderRiffMarker) - 1); /* 0-3 */
  data_stream << qint32(i_bytecount + 44 - 8);                /* 4-7 */
  data_stream.writeRawData(kWavFileTypeFormatChunk,
                           sizeof(kWavFileTypeFormatChunk) - 1); /*  8-15 */
  data_stream << (qint32)16;                                     /* 16-19 */
  data_stream << (qint16)1;                                      /* 20-21 */
  data_stream << (qint16)2;                                      /* 22-23 */
  data_stream << (qint32)44100;                                  /* 24-27 */
  data_stream << (qint32)(44100 * 2 * 2);                        /* 28-31 */
  data_stream << (qint16)4;                                      /* 32-33 */
  data_stream << (qint16)16;                                     /* 34-35 */
  data_stream.writeRawData(kWavDataString,
                           sizeof(kWavDataString) - 1); /* 36-39 */
  data_stream << (qint32)i_bytecount;                   /* 40-43 */
}

void Ripper::Rip() {
  temporary_directory_ = Utilities::MakeTempDir() + "/";
  finished_success_ = 0;
  finished_failed_ = 0;

  // Set up progress bar
  UpdateProgress();

  for (QList<TrackInformation>::iterator it = tracks_.begin();
       it != tracks_.end(); ++it) {
    QString filename =
        QString("%1%2.wav").arg(temporary_directory_).arg(it->track_number);
    QFile destination_file(filename);
    destination_file.open(QIODevice::WriteOnly);

    lsn_t i_first_lsn = cdio_get_track_lsn(cdio_, it->track_number);
    lsn_t i_last_lsn = cdio_get_track_last_lsn(cdio_, it->track_number);
    WriteWAVHeader(&destination_file,
                   (i_last_lsn - i_first_lsn + 1) * CDIO_CD_FRAMESIZE_RAW);

    QByteArray buffered_input_bytes(CDIO_CD_FRAMESIZE_RAW, '\0');
    for (lsn_t i_cursor = i_first_lsn; i_cursor <= i_last_lsn; i_cursor++) {
      {
        QMutexLocker l(&mutex_);
        if (cancel_requested_) {
          qLog(Debug) << "CD ripping canceled.";
          return;
        }
      }
      if (cdio_read_audio_sector(cdio_, buffered_input_bytes.data(),
                                 i_cursor) == DRIVER_OP_SUCCESS) {
        destination_file.write(buffered_input_bytes.data(),
                               buffered_input_bytes.size());
      } else {
        qLog(Error) << "CD read error";
        break;
      }
    }
    finished_success_++;
    UpdateProgress();

    it->temporary_filename = filename;
    transcoder_->AddJob(it->temporary_filename, it->preset,
                        it->transcoded_filename);
  }
  emit(RippingComplete());
}

// The progress interval is [0, 200*AddedTracks()], where the first
// half corresponds to the CD ripping and the second half corresponds
// to the transcoding.
void Ripper::SetupProgressInterval() {
  int max = AddedTracks() * 2 * 100;
  emit ProgressInterval(0, max);
}

void Ripper::UpdateProgress() {
  int progress = (finished_success_ + finished_failed_) * 100;
  QMap<QString, float> current_jobs = transcoder_->GetProgress();
  for (float value : current_jobs.values()) {
    progress += qBound(0, static_cast<int>(value * 100), 99);
  }
  emit Progress(progress);
  qLog(Debug) << "Progress:" << progress;
}

void Ripper::RemoveTemporaryDirectory() {
  if (!temporary_directory_.isEmpty())
    Utilities::RemoveRecursive(temporary_directory_);
  temporary_directory_.clear();
}

void Ripper::TagFiles() {
  files_tagged_ = 0;
  for (const TrackInformation& track : tracks_) {
    Song song;
    song.InitFromFilePartial(track.transcoded_filename);
    song.set_track(track.track_number);
    song.set_title(track.title);
    song.set_album(album_.album);
    song.set_artist(album_.artist);
    song.set_genre(album_.genre);
    song.set_year(album_.year);
    song.set_disc(album_.disc);
    song.set_filetype(album_.type);

    TagReaderReply* reply =
        TagReaderClient::Instance()->SaveFile(song.url().toLocalFile(), song);
    NewClosure(reply, SIGNAL(Finished(bool)), this,
               SLOT(FileTagged(TagReaderReply*)), reply);
  }
}

void Ripper::FileTagged(TagReaderReply* reply) {
  files_tagged_++;
  qLog(Debug) << "Tagged" << files_tagged_ << "of" << tracks_.length()
              << "files";
  if (files_tagged_ == tracks_.length()) {
    qLog(Debug) << "CD ripper finished.";
    emit(Finished());
  }

  reply->deleteLater();
}
