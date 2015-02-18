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

#ifndef SRC_RIPPER_RIPPER_H_
#define SRC_RIPPER_RIPPER_H_

#include <cdio/cdio.h>
#include <QMutex>
#include <QObject>

#include "core/song.h"
#include "core/tagreaderclient.h"
#include "transcoder/transcoder.h"

class QFile;

class Ripper : public QObject {
  Q_OBJECT

 public:
  explicit Ripper(QObject* parent = nullptr);
  ~Ripper();
  void AddTrack(int track_number, const QString& title,
                const QString& transcoded_filename,
                const TranscoderPreset& preset);
  void SetAlbumInformation(const QString& album, const QString& artist,
                           const QString& genre, int year, int disc,
                           Song::FileType type);
  int TracksOnDisc() const;
  int AddedTracks() const;
  void ClearTracks();
  bool CheckCDIOIsValid();
  bool MediaChanged() const;

signals:
  void Finished();
  void Cancelled();
  void ProgressInterval(int min, int max);
  void Progress(int progress);
  void RippingComplete();

 public slots:
  void Start();
  void Cancel();

 private slots:
  void TranscodingJobComplete(const QString& input, const QString& output,
                              bool success);
  void AllTranscodingJobsComplete();
  void LogLine(const QString& message);
  void FileTagged(TagReaderReply* reply);

 private:
  struct TrackInformation {
    TrackInformation(int track_number, const QString& title,
                     const QString& transcoded_filename,
                     const TranscoderPreset& preset)
        : track_number(track_number),
          title(title),
          transcoded_filename(transcoded_filename),
          preset(preset) {}

    int track_number;
    QString title;
    QString transcoded_filename;
    TranscoderPreset preset;
  };

  struct AlbumInformation {
    AlbumInformation(const QString& album = QString(),
                     const QString& artist = QString(),
                     const QString& genre = QString(), int year = 0,
                     int disc = 0, Song::FileType type = Song::Type_Unknown)
        : album(album),
          artist(artist),
          genre(genre),
          year(year),
          disc(disc),
          type(type) {}

    QString album;
    QString artist;
    QString genre;
    int year;
    int disc;
    Song::FileType type;
  };

  void WriteWAVHeader(QFile* stream, int32_t i_bytecount);
  void Rip();
  void SetupProgressInterval();
  void UpdateProgress();
  void RemoveTemporaryDirectory();
  void TagFiles();

  CdIo_t* cdio_;
  Transcoder* transcoder_;
  QString temporary_directory_;
  bool cancel_requested_;
  QMutex mutex_;
  int finished_success_;
  int finished_failed_;
  int files_tagged_;
  QList<TrackInformation> tracks_;
  AlbumInformation album_;
};

#endif  // SRC_RIPPER_RIPPER_H_
