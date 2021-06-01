/* This file is part of Clementine.
 Copyright 2014, Andre Siviero <altsiviero@gmail.com>
 Copyright 2021, Lukas Prediger <lumip@lumip.de>

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
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <algorithm>

#include "config.h"
#include "core/logging.h"
#include "core/tagreaderclient.h"
#include "devices/cddadevice.h"
#include "devices/cddasongloader.h"
#include "devices/devicemanager.h"
#include "ripper/ripper.h"
#include "transcoder/transcoder.h"
#include "transcoder/transcoderoptionsdialog.h"
#include "ui/iconloader.h"
#include "ui_ripcddialog.h"
namespace {
bool ComparePresetsByName(const TranscoderPreset& left,
                          const TranscoderPreset& right) {
  return left.name_ < right.name_;
}

const int kCheckboxColumn = 0;
const int kTrackNumberColumn = 1;
const int kTrackTitleColumn = 2;
const int kTrackDurationColumn = 3;
}  // namespace

const char* RipCDDialog::kSettingsGroup = "Transcoder";
const int RipCDDialog::kMaxDestinationItems = 10;

RipCDDialog::RipCDDialog(DeviceManager& device_manager, QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_RipCDDialog),
      device_manager_(device_manager),
      cdda_devices_(
          device_manager.FindDeviceByUrlSchemes(CddaDevice::url_schemes())),
      working_(false),
      cdda_device_(),
      loader_(nullptr) {
  // Init
  ui_->setupUi(this);

  // Set column widths in the QTableWidget.
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kCheckboxColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kTrackNumberColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kTrackTitleColumn, QHeaderView::Stretch);

  // Add a rip button
  rip_button_ = ui_->button_box->addButton(tr("Start ripping"),
                                           QDialogButtonBox::ActionRole);
  cancel_button_ = ui_->button_box->button(QDialogButtonBox::Cancel);
  close_button_ = ui_->button_box->button(QDialogButtonBox::Close);

  // Hide elements
  cancel_button_->hide();
  ui_->progress_group->hide();

  rip_button_->setEnabled(false);  // will be enabled by DeviceSelected if a
                                   // valid device is selected

  InitializeDevices();

  connect(ui_->select_all_button, SIGNAL(clicked()), SLOT(SelectAll()));
  connect(ui_->select_none_button, SIGNAL(clicked()), SLOT(SelectNone()));
  connect(ui_->invert_selection_button, SIGNAL(clicked()),
          SLOT(InvertSelection()));
  connect(rip_button_, SIGNAL(clicked()), SLOT(ClickedRipButton()));
  connect(close_button_, SIGNAL(clicked()), SLOT(hide()));

  connect(ui_->options, SIGNAL(clicked()), SLOT(Options()));
  connect(ui_->select, SIGNAL(clicked()), SLOT(AddDestination()));

  setWindowTitle(tr("Rip CD"));
  AddDestinationDirectory(QDir::homePath());

  // Get presets
  QList<TranscoderPreset> presets = Transcoder::GetAllPresets();
  std::sort(presets.begin(), presets.end(), ComparePresetsByName);
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

void RipCDDialog::closeEvent(QCloseEvent* event) {
  if (working_) {
    event->ignore();
  }
}

void RipCDDialog::showEvent(QShowEvent* event) {
  ResetDialog();
  if (loader_) loader_->LoadSongs();
  if (!working_) {
    ui_->progress_group->hide();
  }
}

void RipCDDialog::InitializeDevices() {
  Q_ASSERT(!cdda_device_);

  // add all devices to drop down selection
  for (const DeviceInfo* device_info : cdda_devices_) {
    ui_->cd_drive_selection->addItem(device_info->friendly_name_);
  }
  // ensure that selecting the first device below will emit a
  // currentIndexChanged signal
  ui_->cd_drive_selection->setCurrentIndex(-1);

  connect(ui_->cd_drive_selection, SIGNAL(currentIndexChanged(int)),
          SLOT(DeviceSelected(int)));

  // look for any already connected device, guess that might be the one the user
  // is interested in and make it the active selection
  for (int i = 0; i < cdda_devices_.size(); ++i) {
    DeviceInfo* device_info = cdda_devices_[i];
    if (device_info->device_) {
      // found one!
      ui_->cd_drive_selection->setCurrentIndex(i);
      return;
    }
  }

  // there is no device that is already connected; just select the first one
  if (!cdda_devices_.isEmpty()) ui_->cd_drive_selection->setCurrentIndex(0);
}

void RipCDDialog::ClickedRipButton() {
  Q_ASSERT(cdda_device_);

  // create and connect Ripper instance for this task
  Ripper* ripper = new Ripper(cdda_device_->raw_cdio(), this);
  connect(cancel_button_, SIGNAL(clicked()), ripper, SLOT(Cancel()));

  connect(ripper, SIGNAL(Finished()), SLOT(Finished()));
  connect(ripper, SIGNAL(Cancelled()), SLOT(Cancelled()));
  connect(ripper, SIGNAL(ProgressInterval(int, int)),
          SLOT(SetupProgressBarLimits(int, int)));
  connect(ripper, SIGNAL(Progress(int)), SLOT(UpdateProgressBar(int)));

  // Add tracks and album information to the ripper.
  ripper->ClearTracks();
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();
  for (int i = 1; i <= ui_->tableWidget->rowCount(); ++i) {
    if (!checkboxes_.value(i - 1)->isChecked()) {
      continue;
    }
    QString transcoded_filename = GetOutputFileName(
        ParseFileFormatString(ui_->format_filename->text(), i));
    QString title = track_names_.value(i - 1)->text();
    ripper->AddTrack(i, title, transcoded_filename, preset);
  }
  ripper->SetAlbumInformation(
      ui_->albumLineEdit->text(), ui_->artistLineEdit->text(),
      ui_->genreLineEdit->text(), ui_->yearLineEdit->text().toInt(),
      ui_->discLineEdit->text().toInt(), preset.type_);

  SetWorking(true);
  ripper->Start();
}

void RipCDDialog::Options() {
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();

  TranscoderOptionsDialog dialog(preset, this);
  dialog.exec();
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

void RipCDDialog::DeviceSelected(int device_index) {
  // disconnecting from previous loader and device, if any
  if (loader_) disconnect(loader_, nullptr, this, nullptr);
  if (cdda_device_) disconnect(cdda_device_.get(), nullptr, this, nullptr);

  ResetDialog();
  if (device_index < 0)
    return;  // Invalid selection, probably no devices around

  Q_ASSERT(device_index < cdda_devices_.size());

  DeviceInfo* device_info = cdda_devices_[device_index];
  std::shared_ptr<ConnectedDevice> device =
      device_manager_.Connect(device_info);
  cdda_device_ = std::dynamic_pointer_cast<CddaDevice>(device);
  if (!cdda_device_) {
    rip_button_->setEnabled(false);
    QMessageBox cdio_fail(QMessageBox::Critical, tr("Error"),
                          tr("Failed reading CD drive"));
    cdio_fail.exec();
    return;
  }

  connect(cdda_device_.get(), SIGNAL(DiscChanged()), SLOT(DiscChanged()));

  // get SongLoader from device and connect signals
  loader_ = cdda_device_->loader();
  Q_ASSERT(loader_);

  connect(loader_, SIGNAL(SongsDurationLoaded(SongList)),
          SLOT(BuildTrackListTable(SongList)));
  connect(loader_, SIGNAL(SongsMetadataLoaded(SongList)),
          SLOT(UpdateTrackListTable(SongList)));
  connect(loader_, SIGNAL(SongsMetadataLoaded(SongList)),
          SLOT(AddAlbumMetadataFromMusicBrainz(SongList)));

  // load songs from new SongLoader
  loader_->LoadSongs();
  rip_button_->setEnabled(true);
}

void RipCDDialog::Finished() {
  SetWorking(false);
  Ripper* ripper = dynamic_cast<Ripper*>(sender());
  Q_ASSERT(ripper);
  ripper->deleteLater();
}

void RipCDDialog::Cancelled() {
  ui_->progress_bar->setValue(0);
  Finished();
}

void RipCDDialog::SetupProgressBarLimits(int min, int max) {
  ui_->progress_bar->setRange(min, max);
}

void RipCDDialog::UpdateProgressBar(int progress) {
  ui_->progress_bar->setValue(progress);
}

void RipCDDialog::BuildTrackListTable(const SongList& songs) {
  checkboxes_.clear();
  track_names_.clear();

  ui_->tableWidget->setRowCount(songs.length());
  int current_row = 0;
  for (const Song& song : songs) {
    QCheckBox* checkbox = new QCheckBox(ui_->tableWidget);
    checkbox->setCheckState(Qt::Checked);
    checkboxes_.append(checkbox);
    ui_->tableWidget->setCellWidget(current_row, kCheckboxColumn, checkbox);
    ui_->tableWidget->setCellWidget(current_row, kTrackNumberColumn,
                                    new QLabel(QString::number(song.track())));
    QLineEdit* line_edit_track_title =
        new QLineEdit(song.title(), ui_->tableWidget);
    track_names_.append(line_edit_track_title);
    ui_->tableWidget->setCellWidget(current_row, kTrackTitleColumn,
                                    line_edit_track_title);
    ui_->tableWidget->setCellWidget(current_row, kTrackDurationColumn,
                                    new QLabel(song.PrettyLength()));
    current_row++;
  }
}

void RipCDDialog::UpdateTrackListTable(const SongList& songs) {
  if (track_names_.length() == songs.length()) {
    BuildTrackListTable(songs);
  } else {
    qLog(Error) << "Number of tracks in metadata does not match number of "
                   "songs on disc!";
  }
}

void RipCDDialog::AddAlbumMetadataFromMusicBrainz(const SongList& songs) {
  Q_ASSERT(songs.length() > 0);

  const Song& song = songs.first();
  ui_->albumLineEdit->setText(song.album());
  ui_->artistLineEdit->setText(song.artist());
  ui_->yearLineEdit->setText(QString::number(song.year()));
}

void RipCDDialog::DiscChanged() { ResetDialog(); }

void RipCDDialog::SetWorking(bool working) {
  working_ = working;
  rip_button_->setVisible(!working);
  cancel_button_->setVisible(working);
  close_button_->setVisible(!working);
  ui_->input_group->setEnabled(!working);
  ui_->output_group->setEnabled(!working);
  ui_->progress_group->setVisible(true);
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

void RipCDDialog::ResetDialog() {
  ui_->tableWidget->setRowCount(0);
  ui_->albumLineEdit->clear();
  ui_->artistLineEdit->clear();
  ui_->genreLineEdit->clear();
  ui_->yearLineEdit->clear();
  ui_->discLineEdit->clear();
}
