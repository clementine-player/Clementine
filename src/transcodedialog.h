/* This file is part of Clementine.

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

#ifndef TRANSCODEDIALOG_H
#define TRANSCODEDIALOG_H

#include <QDialog>

#include "ui_transcodedialog.h"
#include "ui_transcodelogdialog.h"

class Transcoder;

class TranscodeDialog : public QDialog {
  Q_OBJECT

 public:
  TranscodeDialog(QWidget* parent = 0);

  static const char* kSettingsGroup;

  void SetFilenames(const QStringList& filenames);

 private slots:
  void Add();
  void Remove();
  void Start();
  void Cancel();
  void JobComplete(const QString& filename, bool success);
  void LogLine(const QString& message);
  void AllJobsComplete();

 private:
  void SetWorking(bool working);
  void UpdateStatusText();

 private:
  Ui::TranscodeDialog ui_;
  Ui::TranscodeLogDialog log_ui_;
  QDialog* log_dialog_;

  QPushButton* start_button_;
  QPushButton* cancel_button_;
  QPushButton* close_button_;

  QString last_add_dir_;

  Transcoder* transcoder_;
  int queued_;
  int finished_success_;
  int finished_failed_;
};

#endif // TRANSCODEDIALOG_H
