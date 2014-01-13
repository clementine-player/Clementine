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

#include <QSettings>
#include <QCheckBox>
#include <QFileDialog>
#include <QFrame>
#include <QLineEdit>
#include <QtDebug>
#include <QtConcurrentRun>
#include <cdio/cdda.h>
#include <cdio/cdio.h>
//#include <cdio/paranoia.h>
#include <cdio/cdtext.h>
#include <sys/types.h>
#include <tag.h>
#include <taglib.h>
#include <tfile.h>
#include <fileref.h>
#include <wavfile.h>
#include <tpropertymap.h>
#include <tstring.h>
#include <tstringlist.h>

// winspool.h defines this :(
#ifdef AddJob
#  undef AddJob
#endif

static bool ComparePresetsByName(const TranscoderPreset& left,
		const TranscoderPreset& right) {
	return left.name_ < right.name_;
}

const char* RipCD::kSettingsGroup = "Transcoder";
const int RipCD::kProgressInterval = 500;
const int RipCD::kMaxDestinationItems = 10;

RipCD::RipCD(QWidget* parent) :
		QDialog(parent), transcoder_(new Transcoder(this)), queued_(0), finished_success_(
				0), finished_failed_(0), ui_(new Ui_RipCD()), checkboxes_(
				QList<QCheckBox*>()), generated_files_(QList<QString>()), tracks_to_rip_(
				QList<int>()), track_names_(QList<QLineEdit*>()) {
	// Init
	ui_->setupUi(this);
	cancel_button_ = ui_->button_box->button(QDialogButtonBox::Cancel);

	connect(ui_->ripButton, SIGNAL(clicked()), this, SLOT(clickedRipButton()));
	connect(cancel_button_, SIGNAL(clicked()), SLOT(Cancel()));

	connect(transcoder_, SIGNAL(JobComplete(QString,bool)), SLOT(JobComplete(QString,bool)));
	connect(transcoder_, SIGNAL(AllJobsComplete()), SLOT(AllJobsComplete()));
	connect(transcoder_, SIGNAL(JobOutputName(QString)),
			SLOT(AppendOutput(QString)));
	connect(this, SIGNAL(RippingComplete()), SLOT(threadedTranscoding()));
	connect(this, SIGNAL(signalUpdateProgress()), SLOT(UpdateProgress()));

	connect(ui_->options, SIGNAL(clicked()), SLOT(Options()));
	connect(ui_->select, SIGNAL(clicked()), SLOT(AddDestination()));

	setWindowTitle(tr("Rip CD"));

	//track_t i_first_track;
	p_cdio = cdio_open(NULL, DRIVER_UNKNOWN);
	//i_first_track = cdio_get_first_track_num(p_cdio);
	i_tracks = cdio_get_num_tracks(p_cdio);
	ui_->tableWidget->setRowCount(i_tracks);
	for (int i = 1; i <= i_tracks; i++) {
		QCheckBox *_t = new QCheckBox(tr(""), ui_->tableWidget);
		_t->click();
		checkboxes_.append(_t);
		ui_->tableWidget->setCellWidget(i - 1, 0, _t);
		ui_->tableWidget->setCellWidget(i - 1, 1, new QLabel(QString::number(i)));
		QString _str_track;
		_str_track = "Track %1";
		QLineEdit *_ql = new QLineEdit(_str_track.arg(QString::number(i)),
				ui_->tableWidget);
		track_names_.append(_ql);
		ui_->tableWidget->setCellWidget(i - 1, 2, _ql);
	}
	// Get presets
	QList < TranscoderPreset > presets = Transcoder::GetAllPresets();
	qSort(presets.begin(), presets.end(), ComparePresetsByName);
	foreach (const TranscoderPreset& preset, presets) {
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
		if (last_output_format
				== ui_->format->itemData(i).value<TranscoderPreset>().extension_) {
			ui_->format->setCurrentIndex(i);
			break;
		}
	}

	ui_->progress_bar->setValue(0);
	ui_->progress_bar->setMaximum(100);
}

void RipCD::write_WAV_header(FILE *stream, int32_t i_bytecount) {
	fwrite("RIFF", sizeof(char), 4, stream);
	put_num(i_bytecount + 44 - 8, stream, 4); /*  4-7 */
	fwrite("WAVEfmt ", sizeof(char), 8, stream); /*  8-15 */
	put_num(16, stream, 4); /* 16-19 */
	put_num(1, stream, 2); /* 20-21 */
	put_num(2, stream, 2); /* 22-23 */
	put_num(44100, stream, 4); /* 24-27 */
	put_num(44100 * 2 * 2, stream, 4); /* 28-31 */
	put_num(4, stream, 2); /* 32-33 */
	put_num(16, stream, 2); /* 34-35 */
	fwrite("data", sizeof(char), 4, stream); /* 36-39 */
	put_num(i_bytecount, stream, 4); /* 40-43 */
}

void RipCD::put_num(long int num, FILE *stream, int bytes) {
	unsigned int i;
	unsigned char c;

	for (i = 0; bytes--; i++) {
		c = (num >> (i << 3)) & 0xff;
		if (fwrite(&c, sizeof(char), 1, stream) == -1) {
			perror("Could not write to output.");
			exit(1);
		}
	}
}

int RipCD::nTracksToRip() {
	int k = 0;
	for (int i = 0; i < checkboxes_.length(); i++) {
		if (checkboxes_.value(i)->isChecked() == true) {
			k++;
		}
	}
	return k;
}

void RipCD::toThreadClickedRipButton() {
	QString source_directory = "/tmp/";

	finished_success_ = 0;
	finished_failed_ = 0;
	ui_->progress_bar->setMaximum(nTracksToRip() * 2 * 100);

	emit(signalUpdateProgress());

	// Set up the progressbar

	for (int i = 1; i <= i_tracks; i++) {
		if (checkboxes_.value(i - 1)->isChecked() == false) {
			continue;
		}
		tracks_to_rip_.append(i);
		lsn_t i_first_lsn = cdio_get_track_lsn(p_cdio, i);
		lsn_t i_last_lsn = cdio_get_track_last_lsn(p_cdio, i);
//		lsn_t i_last_lsn = i_first_lsn+300; // debug

		lsn_t i_cursor;
		int16_t *p_readbuf = (int16_t *) calloc(CDIO_CD_FRAMESIZE_RAW, 1);

		QString filename = source_directory + ParseFileFormatString(ui_->format_filename->text(),i);
		FILE *fp = fopen(filename.toUtf8().constData(), "w");
		write_WAV_header(fp,
				(i_last_lsn - i_first_lsn + 1) * CDIO_CD_FRAMESIZE_RAW);
		for (i_cursor = i_first_lsn; i_cursor <= i_last_lsn; i_cursor++) {
			cdio_read_audio_sector(p_cdio, p_readbuf, i_cursor);
			if (!p_readbuf) {
				qDebug() << "Read error. Stopping.";
				break;
			} else {
				fwrite(p_readbuf, 1, CDIO_CD_FRAMESIZE_RAW, fp);
			}

		}
		finished_success_++;
		emit(signalUpdateProgress());
		fclose(fp);
		free(p_readbuf);
		p_readbuf = NULL;

		//TranscoderPreset preset(Transcoder::PresetForFileType(Song::Type_OggVorbis));
		TranscoderPreset preset =
				ui_->format->itemData(ui_->format->currentIndex()).value<
						TranscoderPreset>();

		QString outfilename = GetOutputFileName(filename, preset);
		transcoder_->AddJob(filename.toUtf8().constData(), preset, outfilename);
	}
	emit (RippingComplete());}

// Returns the rightmost non-empty part of 'path'.
QString RipCD::TrimPath(const QString& path) const {
	return path.section('/', -1, -1, QString::SectionSkipEmpty);
}

QString RipCD::GetOutputFileName(const QString& input,
		const TranscoderPreset &preset) const {
	QString path =
			ui_->destination->itemData(ui_->destination->currentIndex()).toString();
	if (path.isEmpty()) {
		// Keep the original path.
		return input.section('.', 0, -2) + '.' + preset.extension_;
	} else {
		QString file_name = TrimPath(input);
		file_name = file_name.section('.', 0, -2);
		return path + '/' + file_name + '.' + preset.extension_;
	}
}

QString RipCD::ParseFileFormatString(const QString& file_format, int trackNo) const {
	QString to_return = file_format;
	to_return.replace(QString("%artist%"),ui_->artistLineEdit->text());
	to_return.replace(QString("%album%"),ui_->albumLineEdit->text());
	to_return.replace(QString("%genre%"),ui_->genreLineEdit->text());
	to_return.replace(QString("%year%"),ui_->yearLineEdit->text());
	to_return.replace(QString("%tracknum%"),QString::number(trackNo));
	to_return.replace(QString("%track%"),track_names_.value(trackNo - 1)->text());
	return to_return;
}

void RipCD::UpdateProgress() {
	int progress = (finished_success_ + finished_failed_) * 100;
	QMap<QString, float> current_jobs = transcoder_->GetProgress();
	foreach (float value, current_jobs.values()) {
		progress += qBound(0, int(value * 100), 99);
	}

	ui_->progress_bar->setValue(progress);
}

void RipCD::threadedTranscoding() {
	transcoder_->Start();
	TranscoderPreset preset =
			ui_->format->itemData(ui_->format->currentIndex()).value<TranscoderPreset>();
	// Save the last output format
	QSettings s;
	s.beginGroup(kSettingsGroup);
	s.setValue("last_output_format", preset.extension_);

}

void RipCD::clickedRipButton() {
	QtConcurrent::run(this, &RipCD::toThreadClickedRipButton);

}

void RipCD::JobComplete(const QString& filename, bool success) {
	(*(success ? &finished_success_ : &finished_failed_))++;
	emit(signalUpdateProgress());
}

void RipCD::AllJobsComplete() {

	// having a little trouble on wav files, works fine on ogg-vorbis
	qSort(generated_files_);

	int i;
	for (i = 0; i < generated_files_.length(); i++) {
		TagLib::FileRef f(generated_files_.value(i).toUtf8().constData());

		f.tag()->setTitle(
				track_names_.value(tracks_to_rip_.value(i) - 1)->text().toUtf8().constData());
		f.tag()->setAlbum(ui_->albumLineEdit->text().toUtf8().constData());
		f.tag()->setArtist(ui_->artistLineEdit->text().toUtf8().constData());
		f.tag()->setGenre(ui_->genreLineEdit->text().toUtf8().constData());
		f.tag()->setYear(ui_->yearLineEdit->text().toInt());
		f.tag()->setTrack(tracks_to_rip_.value(i) - 1);
		// Need to check this
		//f.tag()->setDisc(ui_->discLineEdit->text().toInt());
		f.save();
	}
	// Resets lists
	generated_files_.clear();
	tracks_to_rip_.clear();
}

void RipCD::AppendOutput(const QString &filename) {
	generated_files_.append(filename);
}

void RipCD::Options() {
	TranscoderPreset preset =
			ui_->format->itemData(ui_->format->currentIndex()).value<TranscoderPreset>();

	TranscoderOptionsDialog dialog(preset.type_, this);
	if (dialog.is_valid()) {
		dialog.exec();
	}
}

// Adds a folder to the destination box.
void RipCD::AddDestination() {
	int index = ui_->destination->currentIndex();
	QString initial_dir = (
			!ui_->destination->itemData(index).isNull() ?
					ui_->destination->itemData(index).toString() : QDir::homePath());
	QString dir = QFileDialog::getExistingDirectory(this, tr("Add folder"),
			initial_dir);

	if (!dir.isEmpty()) {
		// Keep only a finite number of items in the box.
		while (ui_->destination->count() >= kMaxDestinationItems) {
			ui_->destination->removeItem(1); // The oldest folder item.
		}

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
}

void RipCD::Cancel() {
	transcoder_->Cancel();
}
