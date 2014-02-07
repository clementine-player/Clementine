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
#include <cdio/cdio.h>
#include "ui_ripcd.h"

class Ui_RipCD;
class Transcoder;

struct TranscoderPreset;

class RipCD : public QDialog {
  Q_OBJECT

 public:
  explicit RipCD(QWidget* parent = 0);
  ~RipCD();
  bool CDIOIsValid() const;

 private:
  static const char* kSettingsGroup;
  static const int kProgressInterval;
  static const int kMaxDestinationItems;
  Transcoder* transcoder_;
  int queued_;
  int finished_success_;
  int finished_failed_;
  track_t i_tracks_;
  Ui_RipCD* ui_;
  CdIo_t* cdio_;
  QList<QCheckBox*> checkboxes_;
  QList<QString> generated_files_;
  QList<int> tracks_to_rip_;
  QList<QLineEdit*> track_names_;
  QString last_add_dir_;
  QPushButton* cancel_button_;
  QPushButton* close_button_;
  QPushButton* rip_button_;
  QString temporary_directory_;

  void WriteWAVHeader(QFile* stream, int32_t i_bytecount);
  int NumTracksToRip();
  void ThreadClickedRipButton();
  QString TrimPath(const QString& path) const;
  QString GetOutputFileName(const QString& input,
                            const TranscoderPreset& preset) const;
  QString ParseFileFormatString(const QString& file_format, int track_no) const;
  void SetWorking(bool working);
  void AddDestinationDirectory(QString dir);
  void RemoveTemporaryDirectory();

signals:
  void RippingComplete();
  void SignalUpdateProgress();
 private slots:
  void UpdateProgress();
  void ThreadedTranscoding();
  void ClickedRipButton();
  void JobComplete(const QString& filename, bool success);
  void AllJobsComplete();
  void AppendOutput(const QString& filename);
  void Options();
  void AddDestination();
  void Cancel();
  void SelectAll();
  void SelectNone();
  void InvertSelection();
};

#endif  // SRC_UI_RIPCD_H_
