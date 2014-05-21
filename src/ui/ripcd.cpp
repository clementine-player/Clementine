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

#include "ripcd.h"
#include "config.h"
#include "ui_ripcd.h"
#include "transcoder/transcoder.h"
#include "transcoder/transcoderoptionsdialog.h"
#include "ui/iconloader.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"

#include <QSettings>
#include <QCheckBox>
#include <QDataStream>
#include <QFileDialog>
#include <QFrame>
#include <QLineEdit>
#include <QMessageBox>
#include <QMutexLocker>
#include <QtDebug>
#include <QtConcurrentRun>
#include <cdio/cdio.h>

// winspool.h defines this :(
#ifdef AddJob
#undef AddJob
#endif

namespace {
bool ComparePresetsByName(const TranscoderPreset& left,
                          const TranscoderPreset& right) {
  return left.name_ < right.name_;
}

const char kWavHeaderRiffMarker[] = "RIFF";
const char kWavFileTypeFormatChunk[] = "WAVEfmt ";
const char kWavDataString[] = "data";

const int kCheckboxColumn = 0;
const int kTrackNumberColumn = 1;
const int kTrackTitleColumn = 2;
}
const char* RipCD::kSettingsGroup = "Transcoder";
const int RipCD::kProgressInterval = 500;
const int RipCD::kMaxDestinationItems = 10;

RipCD::RipCD(QWidget* parent)
    : QDialog(parent),
      transcoder_(new Transcoder(this)),
      queued_(0),
      finished_success_(0),
      finished_failed_(0),
      ui_(new Ui_RipCD),
      cancel_requested_(false),
      files_tagged_(0) {
  cdio_ = cdio_open(NULL, DRIVER_UNKNOWN);
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
  connect(cancel_button_, SIGNAL(clicked()), SLOT(Cancel()));
  connect(close_button_, SIGNAL(clicked()), SLOT(hide()));

  connect(transcoder_, SIGNAL(JobComplete(QString, bool)),
          SLOT(TranscodingJobComplete(QString, bool)));
  connect(transcoder_, SIGNAL(AllJobsComplete()),
          SLOT(AllTranscodingJobsComplete()));
  connect(transcoder_, SIGNAL(LogLine(QString)), SLOT(LogLine(QString)));
  connect(this, SIGNAL(RippingComplete()), SLOT(ThreadedTranscoding()));
  connect(this, SIGNAL(SignalUpdateProgress()), SLOT(UpdateProgress()));

  connect(ui_->options, SIGNAL(clicked()), SLOT(Options()));
  connect(ui_->select, SIGNAL(clicked()), SLOT(AddDestination()));

  setWindowTitle(tr("Rip CD"));
  AddDestinationDirectory(QDir::homePath());

  // Get presets
  QList<TranscoderPreset> presets = Transcoder::GetAllPresets();
  qSort(presets.begin(), presets.end(), ComparePresetsByName);
  for (const TranscoderPreset& preset : presets) {
    ui_->format->addItem(
        QString("%1 (.%2)").arg(preset.name_, preset.extension_),
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

  ui_->progress_bar->setValue(0);
  ui_->progress_bar->setMaximum(100);
}

RipCD::~RipCD() { cdio_destroy(cdio_); }

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
void RipCD::WriteWAVHeader(QFile* stream, int32_t i_bytecount) {
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

int RipCD::NumTracksToRip() {
  int k = 0;
  for (int i = 0; i < checkboxes_.length(); i++) {
    if (checkboxes_.value(i)->isChecked()) {
      k++;
    }
  }
  return k;
}

void RipCD::ThreadClickedRipButton() {
  temporary_directory_ = Utilities::MakeTempDir() + "/";
  finished_success_ = 0;
  finished_failed_ = 0;
  ui_->progress_bar->setMaximum(NumTracksToRip() * 2 * 100);

  // Set up progress bar
  emit(SignalUpdateProgress());

  for (const TrackInformation& track : tracks_) {
    QString filename =
        QString("%1%2.wav").arg(temporary_directory_).arg(track.track_number);
    QFile* destination_file = new QFile(filename);
    destination_file->open(QIODevice::WriteOnly);

    lsn_t i_first_lsn = cdio_get_track_lsn(cdio_, track.track_number);
    lsn_t i_last_lsn = cdio_get_track_last_lsn(cdio_, track.track_number);
    WriteWAVHeader(destination_file,
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
        destination_file->write(buffered_input_bytes.data(),
                                buffered_input_bytes.size());
      } else {
        qLog(Error) << "CD read error";
        break;
      }
    }
    finished_success_++;
    emit(SignalUpdateProgress());
    TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                  .value<TranscoderPreset>();

    transcoder_->AddJob(filename, preset, track.transcoded_filename);
  }
  emit(RippingComplete());
}

QString RipCD::GetOutputFileName(const QString& basename) const {
  QString path =
      ui_->destination->itemData(ui_->destination->currentIndex()).toString();
  QString extension = ui_->format->itemData(ui_->format->currentIndex())
                          .value<TranscoderPreset>()
                          .extension_;
  return path + '/' + basename + '.' + extension;
}

QString RipCD::ParseFileFormatString(const QString& file_format,
                                     int track_no) const {
  QString to_return = file_format;
  to_return.replace(QString("%artist%"), ui_->artistLineEdit->text());
  to_return.replace(QString("%album%"), ui_->albumLineEdit->text());
  to_return.replace(QString("%genre%"), ui_->genreLineEdit->text());
  to_return.replace(QString("%year%"), ui_->yearLineEdit->text());
  to_return.replace(QString("%tracknum%"), QString::number(track_no));
  to_return.replace(QString("%track%"),
                    track_names_.value(track_no - 1)->text());
  return to_return;
}

void RipCD::UpdateProgress() {
  int progress = (finished_success_ + finished_failed_) * 100;
  QMap<QString, float> current_jobs = transcoder_->GetProgress();
  for (float value : current_jobs.values()) {
    progress += qBound(0, static_cast<int>(value * 100), 99);
  }

  ui_->progress_bar->setValue(progress);
}

void RipCD::ThreadedTranscoding() {
  transcoder_->Start();
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();
  // Save the last output format
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("last_output_format", preset.extension_);
}

void RipCD::ClickedRipButton() {
  if (cdio_ && cdio_get_media_changed(cdio_)) {
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

  // Add tracks to the rip list.
  tracks_.clear();
  for (int i = 1; i <= i_tracks_; ++i) {
    if (!checkboxes_.value(i - 1)->isChecked()) {
      continue;
    }
    QString transcoded_filename = GetOutputFileName(
        ParseFileFormatString(ui_->format_filename->text(), i));
    QString title = track_names_.value(i - 1)->text();
    AddTrack(i, title, transcoded_filename);
  }

  // Do nothing if no tracks are selected.
  if (tracks_.isEmpty())
    return;

  // Start ripping.
  SetWorking(true);
  {
    QMutexLocker l(&mutex_);
    cancel_requested_ = false;
  }
  QtConcurrent::run(this, &RipCD::ThreadClickedRipButton);
}

void RipCD::AddTrack(int track_number, const QString& title,
                     const QString& transcoded_filename) {
  TrackInformation track(track_number, title, transcoded_filename);
  tracks_.append(track);
}

void RipCD::TranscodingJobComplete(const QString& filename, bool success) {
  (*(success ? &finished_success_ : &finished_failed_))++;
  emit(SignalUpdateProgress());
}

void RipCD::AllTranscodingJobsComplete() {
  RemoveTemporaryDirectory();

  // Save tags.
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();
  AlbumInformation album(
      ui_->albumLineEdit->text(), ui_->artistLineEdit->text(),
      ui_->genreLineEdit->text(), ui_->yearLineEdit->text().toInt(),
      ui_->discLineEdit->text().toInt(), preset.type_);
  TagFiles(album, tracks_);
}

void RipCD::TagFiles(const AlbumInformation& album,
                     const QList<TrackInformation>& tracks) {
  files_tagged_ = 0;
  for (const TrackInformation& track : tracks_) {
    Song song;
    song.InitFromFilePartial(track.transcoded_filename);
    song.set_track(track.track_number);
    song.set_title(track.title);
    song.set_album(album.album);
    song.set_artist(album.artist);
    song.set_genre(album.genre);
    song.set_year(album.year);
    song.set_disc(album.disc);
    song.set_filetype(album.type);

    TagReaderReply* reply =
        TagReaderClient::Instance()->SaveFile(song.url().toLocalFile(), song);
    NewClosure(reply, SIGNAL(Finished(bool)), this,
               SLOT(FileTagged(TagReaderReply*)), reply);
  }
}

void RipCD::FileTagged(TagReaderReply* reply) {
  files_tagged_++;
  qLog(Debug) << "Tagged" << files_tagged_ << "of" << tracks_.length()
              << "files";

  // Stop working if all files are tagged.
  if (files_tagged_ == tracks_.length()) {
    qLog(Debug) << "CD ripper finished.";
    SetWorking(false);
  }

  reply->deleteLater();
}

void RipCD::Options() {
  TranscoderPreset preset = ui_->format->itemData(ui_->format->currentIndex())
                                .value<TranscoderPreset>();

  TranscoderOptionsDialog dialog(preset.type_, this);
  if (dialog.is_valid()) {
    dialog.exec();
  }
}

// Adds a folder to the destination box.
void RipCD::AddDestination() {
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
void RipCD::AddDestinationDirectory(QString dir) {
  QIcon icon = IconLoader::Load("folder");
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

void RipCD::Cancel() {
  {
    QMutexLocker l(&mutex_);
    cancel_requested_ = true;
  }
  ui_->progress_bar->setValue(0);
  transcoder_->Cancel();
  RemoveTemporaryDirectory();
  SetWorking(false);
}

bool RipCD::CheckCDIOIsValid() {
  if (cdio_) {
    cdio_destroy(cdio_);
  }
  cdio_ = cdio_open(NULL, DRIVER_UNKNOWN);
  // Refresh the status of the cd media. This will prevent unnecessary
  // rebuilds of the track list table.
  cdio_get_media_changed(cdio_);
  return cdio_;
}

void RipCD::SetWorking(bool working) {
  rip_button_->setVisible(!working);
  cancel_button_->setVisible(working);
  close_button_->setVisible(!working);
  ui_->input_group->setEnabled(!working);
  ui_->output_group->setEnabled(!working);
  ui_->progress_group->setVisible(true);
}

void RipCD::SelectAll() {
  for (QCheckBox* checkbox : checkboxes_) {
    checkbox->setCheckState(Qt::Checked);
  }
}

void RipCD::SelectNone() {
  for (QCheckBox* checkbox : checkboxes_) {
    checkbox->setCheckState(Qt::Unchecked);
  }
}

void RipCD::InvertSelection() {
  for (QCheckBox* checkbox : checkboxes_) {
    if (checkbox->isChecked()) {
      checkbox->setCheckState(Qt::Unchecked);
    } else {
      checkbox->setCheckState(Qt::Checked);
    }
  }
}

void RipCD::RemoveTemporaryDirectory() {
  if (!temporary_directory_.isEmpty())
    Utilities::RemoveRecursive(temporary_directory_);
  temporary_directory_.clear();
}

void RipCD::BuildTrackListTable() {
  checkboxes_.clear();
  track_names_.clear();
  i_tracks_ = cdio_get_num_tracks(cdio_);
  ui_->tableWidget->setRowCount(i_tracks_);
  for (int i = 1; i <= i_tracks_; i++) {
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
  }
}

void RipCD::LogLine(const QString& message) { qLog(Debug) << message; }

void RipCD::showEvent(QShowEvent* event) { BuildTrackListTable(); }
