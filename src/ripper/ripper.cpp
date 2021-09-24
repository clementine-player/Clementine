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
#include <QUrl>
#include <QtConcurrentRun>

#include "core/closure.h"
#include "core/logging.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"
#include "devices/cddadevice.h"
#include "transcoder/transcoder.h"

// winspool.h defines this :(
#ifdef AddJob
#undef AddJob
#endif

namespace {
const char kWavHeaderRiffMarker[] = "RIFF";
const char kWavFileTypeFormatChunk[] = "WAVEfmt ";
const char kWavDataString[] = "data";
}  // namespace

Ripper::Ripper(int track_count, QObject* parent)
    : QObject(parent),
      track_count_(track_count),
      transcoder_(new Transcoder(this)),
      cancel_requested_(false),
      finished_success_(0),
      finished_failed_(0),
      files_tagged_(0) {
  Q_ASSERT(track_count >= 0);

  transcoder_->set_max_threads(1);  // we want transcoder to read only one song
                                    // at once from disc to prevent seeking
  connect(transcoder_, SIGNAL(JobComplete(QUrl, QString, bool)),
          SLOT(TranscodingJobComplete(QUrl, QString, bool)));
  connect(transcoder_, SIGNAL(AllJobsComplete()),
          SLOT(AllTranscodingJobsComplete()));
  connect(transcoder_, SIGNAL(LogLine(QString)), SLOT(LogLine(QString)));
}

Ripper::~Ripper() {}

void Ripper::AddTrack(int track_number, const QString& title,
                      const QString& transcoded_filename,
                      const TranscoderPreset& preset, bool overwrite_existing) {
  if (track_number < 1 || track_number > TracksOnDisc()) {
    qLog(Warning) << "Invalid track number:" << track_number << "Ignoring";
    return;
  }
  TrackInformation track(track_number, title, transcoded_filename, preset,
                         overwrite_existing);
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

int Ripper::TracksOnDisc() const { return track_count_; }

int Ripper::AddedTracks() const { return tracks_.length(); }

void Ripper::ClearTracks() { tracks_.clear(); }

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
  emit(Cancelled());
}

void Ripper::TranscodingJobComplete(const QUrl& input, const QString& output,
                                    bool success) {
  if (success)
    finished_success_++;
  else
    finished_failed_++;
  UpdateProgress();

  // The transcoder does not necessarily overwrite files. If not, it changes
  // the name of the output file. We need to update the transcoded
  // filename for the corresponding track so that we tag the correct
  // file later on.
  for (QList<TrackInformation>::iterator it = tracks_.begin();
       it != tracks_.end(); ++it) {
    QUrl track_url =
        CddaDevice::TrackStrToUrl(QString("cdda://%1").arg(it->track_number));
    if (track_url == input) {
      it->transcoded_filename = output;
    }
  }
}

void Ripper::AllTranscodingJobsComplete() { TagFiles(); }

void Ripper::LogLine(const QString& message) { qLog(Debug) << message; }

void Ripper::Rip() {
  if (tracks_.isEmpty()) {
    emit Finished();
    return;
  }

  finished_success_ = 0;
  finished_failed_ = 0;

  // Set up progress bar
  UpdateProgress();

  for (QList<TrackInformation>::iterator it = tracks_.begin();
       it != tracks_.end(); ++it) {
    QUrl track_url =
        CddaDevice::TrackStrToUrl(QString("cdda://%1").arg(it->track_number));
    transcoder_->AddJob(track_url, it->preset, it->transcoded_filename);
  }
  transcoder_->Start();
  emit RippingComplete();
}

// The progress interval is [0, 100*AddedTracks()].
void Ripper::SetupProgressInterval() {
  int max = AddedTracks() * 100;
  emit ProgressInterval(0, max);
}

void Ripper::UpdateProgress() {
  int progress = (finished_success_ + finished_failed_) * 100;
  QMap<QUrl, float> current_jobs = transcoder_->GetProgress();
  for (float value : current_jobs.values()) {
    progress += qBound(0, static_cast<int>(value * 100), 99);
  }
  emit Progress(progress);
  qLog(Debug) << "Progress:" << progress;
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
    emit Finished();
  }

  reply->deleteLater();
}
