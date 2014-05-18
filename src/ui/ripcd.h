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

#ifndef SRC_UI_RIPCD_H_
#define SRC_UI_RIPCD_H_

#include <QDialog>
#include <QCheckBox>
#include <QThread>
#include <QFile>
#include <QMutex>
#include <cdio/cdio.h>
#include "core/song.h"
#include "ui_ripcd.h"
#include <memory>
class Ui_RipCD;
class Transcoder;

struct TranscoderPreset;

class RipCD : public QDialog {
  Q_OBJECT

 public:
  explicit RipCD(QWidget* parent = nullptr);
  ~RipCD();
  bool CheckCDIOIsValid();
  void BuildTrackListTable();

 protected:
  void showEvent(QShowEvent* event);

 private:
  struct TrackInformation {
    TrackInformation(int track_number, const QString& title,
                     const QString& transcoded_filename)
        : track_number(track_number),
          title(title),
          transcoded_filename(transcoded_filename) {}

    int track_number;
    QString title;
    QString transcoded_filename;
  };

  struct AlbumInformation {
    AlbumInformation(const QString& album, const QString& artist,
                     const QString& genre, int year, int disc,
                     Song::FileType type)
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

  static const char* kSettingsGroup;
  static const int kProgressInterval;
  static const int kMaxDestinationItems;
  Transcoder* transcoder_;
  int queued_;
  int finished_success_;
  int finished_failed_;
  track_t i_tracks_;
  std::unique_ptr<Ui_RipCD> ui_;
  CdIo_t* cdio_;
  QList<QCheckBox*> checkboxes_;
  QList<QLineEdit*> track_names_;
  QList<TrackInformation> tracks_;
  QString last_add_dir_;
  QPushButton* cancel_button_;
  QPushButton* close_button_;
  QPushButton* rip_button_;
  QString temporary_directory_;
  bool cancel_requested_;
  QMutex mutex_;

  void WriteWAVHeader(QFile* stream, int32_t i_bytecount);
  int NumTracksToRip();
  void AddTrack(int track_number, const QString& title,
                const QString& transcoded_filename);
  void ThreadClickedRipButton();
  // Constructs a filename from the given base name with a path taken
  // from the ui dialog and an extension that corresponds to the audio
  // format chosen in the ui.
  QString GetOutputFileName(const QString& basename) const;
  QString ParseFileFormatString(const QString& file_format, int track_no) const;
  void SetWorking(bool working);
  void AddDestinationDirectory(QString dir);
  void RemoveTemporaryDirectory();
  // Tags the final files. This function should not be run in the UI thread.
  void TagFiles(const AlbumInformation& album,
                const QList<TrackInformation>& tracks);

signals:
  void RippingComplete();
  void SignalUpdateProgress();

 private slots:
  void UpdateProgress();
  void ThreadedTranscoding();
  void ClickedRipButton();
  void JobComplete(const QString& filename, bool success);
  void AllJobsComplete();
  void TaggingComplete();
  void Options();
  void AddDestination();
  void Cancel();
  void SelectAll();
  void SelectNone();
  void InvertSelection();
  void LogLine(const QString& message);
};

#endif  // SRC_UI_RIPCD_H_
