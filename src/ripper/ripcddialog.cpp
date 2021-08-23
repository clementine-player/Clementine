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
#include "core/organiseformat.h"
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
const int kTrackFilenamePreviewColumn = 4;
}  // namespace

const char* RipCDDialog::kSettingsGroup = "Transcoder";
const int RipCDDialog::kMaxDestinationItems = 10;

RipCDDialog::RipCDDialog(DeviceManager* device_manager, QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_RipCDDialog),
      device_manager_(device_manager),
      cdda_devices_(
          device_manager->FindDevicesByUrlSchemes(CddaDevice::url_schemes())),
      working_(false),
      cdda_device_() {
  Q_ASSERT(device_manager);
  // Init
  ui_->setupUi(this);

  // Set column widths in the QTableWidget.
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kCheckboxColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kTrackNumberColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kTrackDurationColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kTrackTitleColumn, QHeaderView::ResizeToContents);
  ui_->tableWidget->horizontalHeader()->setSectionResizeMode(
      kTrackFilenamePreviewColumn, QHeaderView::Stretch);

  // Add a rip button
  rip_button_ = ui_->button_box->addButton(tr("Start ripping"),
                                           QDialogButtonBox::ActionRole);
  cancel_button_ = ui_->button_box->button(QDialogButtonBox::Cancel);
  close_button_ = ui_->button_box->button(QDialogButtonBox::Close);

  // Hide elements
  cancel_button_->hide();
  ui_->progress_group->hide();

  rip_button_->setEnabled(
      false);  // will be enabled by signal handlers if a valid device is
               // selected by user and a list of tracks is loaded

  connect(ui_->select_all_button, SIGNAL(clicked()), SLOT(SelectAll()));
  connect(ui_->select_none_button, SIGNAL(clicked()), SLOT(SelectNone()));
  connect(ui_->invert_selection_button, SIGNAL(clicked()),
          SLOT(InvertSelection()));
  connect(rip_button_, SIGNAL(clicked()), SLOT(ClickedRipButton()));
  connect(close_button_, SIGNAL(clicked()), SLOT(hide()));

  connect(ui_->options, SIGNAL(clicked()), SLOT(Options()));
  connect(ui_->select, SIGNAL(clicked()), SLOT(AddDestination()));

  connect(ui_->naming_group, SIGNAL(FormatStringChanged()),
          SLOT(FormatStringUpdated()));
  connect(ui_->naming_group, SIGNAL(OptionChanged()),
          SLOT(FormatStringUpdated()));

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

  QString last_output_format =
      s.value("last_output_format", "audio/x-vorbis").toString();
  qLog(Debug) << "last_output_format loaded: " << last_output_format;
  for (int i = 0; i < ui_->format->count(); ++i) {
    if (last_output_format ==
        ui_->format->itemData(i).value<TranscoderPreset>().codec_mimetype_) {
      ui_->format->setCurrentIndex(i);
      break;
    }
  }

  connect(ui_->format, SIGNAL(currentIndexChanged(int)),
          SLOT(UpdateFileNamePreviews()));

  connect(ui_->artistLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(UpdateMetadataFromGUI()));
  connect(ui_->albumLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(UpdateMetadataFromGUI()));
  connect(ui_->genreLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(UpdateMetadataFromGUI()));
  connect(ui_->yearLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(YearEditChanged(const QString&)));
  connect(ui_->discLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(DiscEditChanged(const QString&)));

  InitializeDevices();
}

RipCDDialog::~RipCDDialog() {}

void RipCDDialog::closeEvent(QCloseEvent* event) {
  if (working_) {
    event->ignore();
  }
}

void RipCDDialog::showEvent(QShowEvent* event) {
  // No need to refresh track list here anymore. On first open this is initiated
  // by InitializeDevices(). Afterwards track list is always updates when a disc
  // change is detected due to working through CddaDevice
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

  OrganiseFormat format = ui_->naming_group->format();
  Q_ASSERT(format.IsValid());

  QFileInfo path(
      ui_->destination->itemData(ui_->destination->currentIndex()).toString());

  // create and connect Ripper instance for this task
  Ripper* ripper = new Ripper(cdda_device_->raw_cdio(), this);
  connect(cancel_button_, SIGNAL(clicked()), ripper, SLOT(Cancel()));

  connect(ripper, &Ripper::Finished, this,
          [this, ripper]() { this->Finished(ripper); });
  connect(ripper, &Ripper::Cancelled, this,
          [this, ripper]() { this->Cancelled(ripper); });
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
    Song& song = songs_[i - 1];
    QString transcoded_filename = format.GetFilenameForSong(
        song, preset, /*prefix_path=*/path.filePath());
    ripper->AddTrack(i, song.title(), transcoded_filename, preset,
                     ui_->naming_group->overwrite_existing());
  }

  ripper->SetAlbumInformation(
      ui_->albumLineEdit->text(), ui_->artistLineEdit->text(),
      ui_->genreLineEdit->text(), ui_->yearLineEdit->text().toInt(),
      ui_->discLineEdit->text().toInt(), preset.type_);

  SetWorking(true);
  ripper->Start();

  // store settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("last_output_format", preset.codec_mimetype_);
  qLog(Debug) << "last_output_format stored: " << preset.codec_mimetype_;

  ui_->naming_group->StoreSettings();
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
  // disconnecting from previous device, if any
  if (cdda_device_) disconnect(cdda_device_.get(), nullptr, this, nullptr);

  ResetDialog();
  EnableIfPossible();
  if (device_index < 0)
    return;  // Invalid selection, probably no devices around

  Q_ASSERT(device_index < cdda_devices_.size());

  DeviceInfo* device_info = cdda_devices_[device_index];
  std::shared_ptr<ConnectedDevice> device =
      device_manager_->Connect(device_info);
  cdda_device_ = std::dynamic_pointer_cast<CddaDevice>(device);
  if (!cdda_device_) {
    rip_button_->setEnabled(false);
    QMessageBox cdio_fail(QMessageBox::Critical, tr("Error"),
                          tr("Failed reading CD drive"));
    cdio_fail.exec();
    return;
  }

  SongList songs = cdda_device_->songs();
  SongsLoaded(songs);

  connect(cdda_device_.get(), SIGNAL(DiscChanged()), SLOT(DiscChanged()));
  connect(cdda_device_.get(), SIGNAL(SongsDiscovered(SongList)),
          SLOT(SongsLoaded(SongList)));
}

void RipCDDialog::Finished(Ripper* ripper) {
  SetWorking(false);
  ripper->deleteLater();
}

void RipCDDialog::Cancelled(Ripper* ripper) {
  ui_->progress_bar->setValue(0);
  Finished(ripper);
}

void RipCDDialog::SetupProgressBarLimits(int min, int max) {
  ui_->progress_bar->setRange(min, max);
}

void RipCDDialog::UpdateProgressBar(int progress) {
  ui_->progress_bar->setValue(progress);
}

void RipCDDialog::SongsLoaded(const SongList& songs) {
  if (songs_.isEmpty() || songs_.length() == songs.length()) {
    songs_ = songs;
    UpdateTrackListTable();
    UpdateMetadataEdits();
  } else {
    qLog(Error) << "Number of tracks in metadata does not match number of "
                   "songs on disc!";
  }
  EnableIfPossible();
}

void RipCDDialog::UpdateTrackListTable() {
  checkboxes_.clear();

  ui_->tableWidget->clear();
  ui_->tableWidget->setRowCount(songs_.length());
  int current_row = 0;
  for (const Song& song : songs_) {
    QCheckBox* checkbox = new QCheckBox(ui_->tableWidget);
    checkbox->setCheckState(Qt::Checked);
    checkboxes_.append(checkbox);
    ui_->tableWidget->setCellWidget(current_row, kCheckboxColumn, checkbox);
    ui_->tableWidget->setCellWidget(current_row, kTrackNumberColumn,
                                    new QLabel(QString::number(song.track())));
    QLineEdit* line_edit_track_title =
        new QLineEdit(song.title(), ui_->tableWidget);
    connect(line_edit_track_title, &QLineEdit::textChanged,
            [this, current_row](const QString& text) {
              songs_[current_row].set_title(text);
              UpdateFileNamePreviews();
            });
    ui_->tableWidget->setCellWidget(current_row, kTrackTitleColumn,
                                    line_edit_track_title);
    ui_->tableWidget->setCellWidget(current_row, kTrackDurationColumn,
                                    new QLabel(song.PrettyLength()));
    current_row++;
  }
  UpdateFileNamePreviews();
}

void RipCDDialog::UpdateFileNamePreviews() {
  OrganiseFormat format = ui_->naming_group->format();
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();

  int current_row = 0;
  for (const Song& song : songs_) {
    if (format.IsValid())
      ui_->tableWidget->setCellWidget(
          current_row, kTrackFilenamePreviewColumn,
          new QLabel(format.GetFilenameForSong(song, preset)));
    else
      ui_->tableWidget->setCellWidget(current_row, kTrackFilenamePreviewColumn,
                                      new QLabel(tr("Invalid format")));
    current_row++;
  }
}

void RipCDDialog::UpdateMetadataEdits() {
  if (songs_.length() <= 0) return;

  const Song& song = songs_.first();
  ui_->albumLineEdit->setText(song.album());
  if (!song.artist().isEmpty())
    ui_->artistLineEdit->setText(song.artist());
  else
    ui_->artistLineEdit->setText(song.albumartist());
  ui_->yearLineEdit->setText(song.PrettyYear());
  ui_->genreLineEdit->setText(song.genre());
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

void RipCDDialog::ResetDialog() {
  songs_.clear();
  ui_->tableWidget->setRowCount(0);
  ui_->albumLineEdit->clear();
  ui_->artistLineEdit->clear();
  ui_->genreLineEdit->clear();
  ui_->yearLineEdit->clear();
  ui_->discLineEdit->clear();
}

void RipCDDialog::FormatStringUpdated() {
  UpdateFileNamePreviews();
  EnableIfPossible();
}

void RipCDDialog::EnableIfPossible() {
  bool disc_ok;
  ui_->discLineEdit->text().toInt(&disc_ok);
  disc_ok |= ui_->discLineEdit->text().isEmpty();

  bool year_ok;
  ui_->yearLineEdit->text().toInt(&year_ok);
  year_ok |= ui_->yearLineEdit->text().isEmpty();

  rip_button_->setEnabled(!songs_.isEmpty() &&
                          ui_->naming_group->format().IsValid() && disc_ok &&
                          year_ok);
}

void RipCDDialog::DiscEditChanged(const QString& disc_string) {
  bool disc_ok = false;
  disc_string.toInt(&disc_ok);

  bool is_valid = disc_string.isEmpty() || disc_ok;

  QString style;
  if (!is_valid) {
    style = "color: red;";
  } else {
    UpdateMetadataFromGUI();
  }
  ui_->discLineEdit->setStyleSheet(style);
  EnableIfPossible();
}

void RipCDDialog::YearEditChanged(const QString& year_string) {
  bool year_ok = false;
  year_string.toInt(&year_ok);

  bool is_valid = year_string.isEmpty() || year_ok;

  QString style;
  if (!is_valid) {
    style = "color: red;";
  } else {
    UpdateMetadataFromGUI();
  }
  ui_->yearLineEdit->setStyleSheet(style);
  EnableIfPossible();
}

void RipCDDialog::UpdateMetadataFromGUI() {
  QString artist = ui_->artistLineEdit->text();
  QString album = ui_->albumLineEdit->text();
  QString genre = ui_->genreLineEdit->text();
  bool disc_ok = false;
  int disc = ui_->discLineEdit->text().toInt(&disc_ok);
  bool year_ok = false;
  int year = ui_->yearLineEdit->text().toInt(&year_ok);

  for (Song& song : songs_) {
    song.set_artist(artist);
    song.set_album(album);
    song.set_genre(genre);
    if (disc_ok)
      song.set_disc(disc);
    else
      song.set_disc(-1);
    if (year_ok)
      song.set_year(year);
    else
      song.set_year(-1);
  }
  UpdateFileNamePreviews();
}
