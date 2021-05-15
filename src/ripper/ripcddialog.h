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

#ifndef SRC_RIPPER_RIPCDDIALOG_H_
#define SRC_RIPPER_RIPCDDIALOG_H_

#include <QDialog>
#include <QFile>
#include <QThread>
#include <memory>

#include "core/song.h"
#include "core/tagreaderclient.h"

class QCheckBox;
class QCloseEvent;
class QLineEdit;
class QShowEvent;

class CddaSongLoader;
class Ripper;
class Ui_RipCDDialog;

class RipCDDialog : public QDialog {
  Q_OBJECT

 public:
  explicit RipCDDialog(QWidget* parent = nullptr);
  ~RipCDDialog();
  bool CheckCDIOIsValid();

 protected:
  void closeEvent(QCloseEvent* event);
  void showEvent(QShowEvent* event);

 private slots:
  void ClickedRipButton();
  void Options();
  void AddDestination();
  void SelectAll();
  void SelectNone();
  void InvertSelection();
  void Finished();
  void Cancelled();
  void SetupProgressBarLimits(int min, int max);
  void UpdateProgressBar(int progress);
  void BuildTrackListTable(const SongList& songs);
  void AddAlbumMetadataFromMusicBrainz(const SongList& songs);

 private:
  static const char* kSettingsGroup;
  static const int kMaxDestinationItems;

  // Constructs a filename from the given base name with a path taken
  // from the ui dialog and an extension that corresponds to the audio
  // format chosen in the ui.
  void AddDestinationDirectory(QString dir);
  QString GetOutputFileName(const QString& basename) const;
  QString ParseFileFormatString(const QString& file_format, int track_no) const;
  void SetWorking(bool working);
  void ResetDialog();

  void MediaChangedPoller();

  class MediaChangedPollingThread : public QThread {

  public:
    MediaChangedPollingThread(RipCDDialog& dialog);
    ~MediaChangedPollingThread() override;

  protected:
    void run() override;

  private:
    RipCDDialog& dialog_;
  };

  QList<QCheckBox*> checkboxes_;
  QList<QLineEdit*> track_names_;
  QString last_add_dir_;
  QPushButton* cancel_button_;
  QPushButton* close_button_;
  QPushButton* rip_button_;
  std::unique_ptr<Ui_RipCDDialog> ui_;
  Ripper* ripper_;
  bool working_;
  CddaSongLoader* loader_;
  MediaChangedPollingThread mediaChangedWatchingThread_;
  QMutex mutex_;  // mutex to control access to track list/metadata updates
};
#endif  // SRC_RIPPER_RIPCDDIALOG_H_
