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

#include "ripper/ripcddialog.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>

#include "config.h"
#include "core/logging.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"
#include "ripper/ripper.h"
#include "ui_ripcddialog.h"
#include "transcoder/transcoder.h"
#include "transcoder/transcoderoptionsdialog.h"
#include "ui/iconloader.h"

namespace {
bool ComparePresetsByName(const TranscoderPreset& left,
                          const TranscoderPreset& right) {
  return left.name_ < right.name_;
}

const int kCheckboxColumn = 0;
const int kTrackNumberColumn = 1;
const int kTrackTitleColumn = 2;
const int kTrackDurationColumn = 3;
}

const char* RipCDDialog::kSettingsGroup = "Transcoder";
const int RipCDDialog::kMaxDestinationItems = 10;

RipCDDialog::RipCDDialog(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_RipCDDialog),
      ripper_(new Ripper(this)),
      working_(false) {
  // Init
  ui_->setupUi(this);

  // Set column widths in the QTableWidget.
  ui_->tableWidget->horizontalHeader()->setResizeMode(
      kCheckboxColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setResizeMode(
      kTrackNumberColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setResizeMode(kTrackTitleColumn,
                                                      QHeaderView::Stretch);

  // Add a rip button
  rip_button_ = ui_->button_box->addButton(tr("Start ripping"),
                                           QDialogButtonBox::ActionRole);
  cancel_button_ = ui_->button_box->button(QDialogButtonBox::Cancel);
  close_button_ = ui_->button_box->button(QDialogButtonBox::Close);

  // Hide elements
  cancel_button_->hide();
  ui_->progress_group->hide();

  connect(ui_->select_all_button, SIGNAL(clicked()), SLOT(SelectAll()));
  connect(ui_->select_none_button, SIGNAL(clicked()), SLOT(SelectNone()));
  connect(ui_->invert_selection_button, SIGNAL(clicked()),
          SLOT(InvertSelection()));
  connect(rip_button_, SIGNAL(clicked()), SLOT(ClickedRipButton()));
  connect(cancel_button_, SIGNAL(clicked()), ripper_, SLOT(Cancel()));
  connect(close_button_, SIGNAL(clicked()), SLOT(hide()));

  connect(ui_->options, SIGNAL(clicked()), SLOT(Options()));
  connect(ui_->select, SIGNAL(clicked()), SLOT(AddDestination()));

  connect(ripper_, SIGNAL(Finished()), SLOT(Finished()));
  connect(ripper_, SIGNAL(Cancelled()), SLOT(Cancelled()));
  connect(ripper_, SIGNAL(ProgressInterval(int, int)),
          SLOT(SetupProgressBarLimits(int, int)));
  connect(ripper_, SIGNAL(Progress(int)), SLOT(UpdateProgressBar(int)));

  setWindowTitle(tr("Rip CD"));
  AddDestinationDirectory(QDir::homePath());

  // Get presets
  QList<TranscoderPreset> presets = Transcoder::GetAllPresets();
  qSort(presets.begin(), presets.end(), ComparePresetsByName);
  for (const TranscoderPreset& preset : presets) {
    ui_->format->addItem(
        QString("%1 (.%2)").arg(preset.name_).arg(preset.extension_),
        QVariant::fromValue(preset));
  }

  // Load settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  last_add_dir_ = s.value("last_add_dir", QDir::homePath()).toString();

  QString last_output_format = s.value("last_output_format", "ogg").toString();
  for (int i = 0; i < ui_->format->count(); ++i) {
    if (last_output_format ==
        ui_->format->itemData(i).value<TranscoderPreset>().extension_) {
      ui_->format->setCurrentIndex(i);
      break;
    }
  }
}

RipCDDialog::~RipCDDialog() {}

bool RipCDDialog::CheckCDIOIsValid() { return ripper_->CheckCDIOIsValid(); }

void RipCDDialog::showEvent(QShowEvent* event) {
  BuildTrackListTable();
  if (!working_) {
    ui_->progress_group->hide();
  }
}

void RipCDDialog::ClickedRipButton() {
  if (ripper_->MediaChanged()) {
    QMessageBox cdio_fail(QMessageBox::Critical, tr("Error Ripping CD"),
                          tr("Media has changed. Reloading"));
    cdio_fail.exec();
    if (CheckCDIOIsValid()) {
      BuildTrackListTable();
    } else {
      ui_->tableWidget->clearContents();
    }
    return;
  }

  // Add tracks and album information to the ripper.
  ripper_->ClearTracks();
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();
  for (int i = 1; i <= ui_->tableWidget->rowCount(); ++i) {
    if (!checkboxes_.value(i - 1)->isChecked()) {
      continue;
    }
    QString transcoded_filename = GetOutputFileName(
        ParseFileFormatString(ui_->format_filename->text(), i));
    QString title = track_names_.value(i - 1)->text();
    ripper_->AddTrack(i, title, transcoded_filename, preset);
  }
  ripper_->SetAlbumInformation(
      ui_->albumLineEdit->text(), ui_->artistLineEdit->text(),
      ui_->genreLineEdit->text(), ui_->yearLineEdit->text().toInt(),
      ui_->discLineEdit->text().toInt(), preset.type_);

  SetWorking(true);
  ripper_->Start();
}

void RipCDDialog::Options() {
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();

  TranscoderOptionsDialog dialog(preset.type_, this);
  if (dialog.is_valid()) {
    dialog.exec();
  }
}

// Adds a folder to the destination box.
void RipCDDialog::AddDestination() {
  int index = ui_->destination->currentIndex();
  QString initial_dir = (!ui_->destination->itemData(index).isNull()
                             ? ui_->destination->itemData(index).toString()
                             : QDir::homePath());
  QString dir =
      QFileDialog::getExistingDirectory(this, tr("Add folder"), initial_dir);

  if (!dir.isEmpty()) {
    // Keep only a finite number of items in the box.
    while (ui_->destination->count() >= kMaxDestinationItems) {
      ui_->destination->removeItem(0);  // The oldest item.
    }
    AddDestinationDirectory(dir);
  }
}

// Adds a directory to the 'destination' combo box.
void RipCDDialog::AddDestinationDirectory(QString dir) {
  QIcon icon = IconLoader::Load("folder", IconLoader::Base);
  QVariant data = QVariant::fromValue(dir);
  // Do not insert duplicates.
  int duplicate_index = ui_->destination->findData(data);
  if (duplicate_index == -1) {
    ui_->destination->addItem(icon, dir, data);
    ui_->destination->setCurrentIndex(ui_->destination->count() - 1);
  } else {
    ui_->destination->setCurrentIndex(duplicate_index);
  }
}

void RipCDDialog::SelectAll() {
  for (QCheckBox* checkbox : checkboxes_) {
    checkbox->setCheckState(Qt::Checked);
  }
}

void RipCDDialog::SelectNone() {
  for (QCheckBox* checkbox : checkboxes_) {
    checkbox->setCheckState(Qt::Unchecked);
  }
}

void RipCDDialog::InvertSelection() {
  for (QCheckBox* checkbox : checkboxes_) {
    checkbox->setCheckState(checkbox->isChecked() ? Qt::Unchecked
                                                  : Qt::Checked);
  }
}

void RipCDDialog::Finished() { SetWorking(false); }

void RipCDDialog::Cancelled() {
  ui_->progress_bar->setValue(0);
  SetWorking(false);
}

void RipCDDialog::SetupProgressBarLimits(int min, int max) {
  ui_->progress_bar->setRange(min, max);
}

void RipCDDialog::UpdateProgressBar(int progress) {
  ui_->progress_bar->setValue(progress);
}

void RipCDDialog::SetWorking(bool working) {
  working_ = working;
  rip_button_->setVisible(!working);
  cancel_button_->setVisible(working);
  close_button_->setVisible(!working);
  ui_->input_group->setEnabled(!working);
  ui_->output_group->setEnabled(!working);
  ui_->progress_group->setVisible(true);
}

void RipCDDialog::BuildTrackListTable() {
  checkboxes_.clear();
  track_names_.clear();

  int tracks = ripper_->TracksOnDisc();

  ui_->tableWidget->setRowCount(tracks);
  for (int i = 1; i <= tracks; i++) {
    QCheckBox* checkbox_i = new QCheckBox(ui_->tableWidget);
    checkbox_i->setCheckState(Qt::Checked);
    checkboxes_.append(checkbox_i);
    ui_->tableWidget->setCellWidget(i - 1, kCheckboxColumn, checkbox_i);
    ui_->tableWidget->setCellWidget(i - 1, kTrackNumberColumn,
                                    new QLabel(QString::number(i)));
    QString track_title = QString("Track %1").arg(i);
    QLineEdit* line_edit_track_title_i =
        new QLineEdit(track_title, ui_->tableWidget);
    track_names_.append(line_edit_track_title_i);
    ui_->tableWidget->setCellWidget(i - 1, kTrackTitleColumn,
                                    line_edit_track_title_i);
    QString track_duration =
        Utilities::PrettyTime(ripper_->TrackDurationSecs(i));
    ui_->tableWidget->setCellWidget(i - 1, kTrackDurationColumn,
                                    new QLabel(track_duration));
  }
}

QString RipCDDialog::GetOutputFileName(const QString& basename) const {
  QFileInfo path(
      ui_->destination->itemData(ui_->destination->currentIndex()).toString());
  QString extension = ui_->format->itemData(ui_->format->currentIndex())
                          .value<TranscoderPreset>()
                          .extension_;
  return path.filePath() + '/' + basename + '.' + extension;
}

QString RipCDDialog::ParseFileFormatString(const QString& file_format,
                                           int track_no) const {
  QString to_return = file_format;
  to_return.replace(QString("%artist"), ui_->artistLineEdit->text());
  to_return.replace(QString("%album"), ui_->albumLineEdit->text());
  to_return.replace(QString("%disc"), ui_->discLineEdit->text());
  to_return.replace(QString("%genre"), ui_->genreLineEdit->text());
  to_return.replace(QString("%year"), ui_->yearLineEdit->text());
  to_return.replace(QString("%title"),
                    track_names_.value(track_no - 1)->text());
  to_return.replace(QString("%track"), QString::number(track_no));

  return to_return;
}
