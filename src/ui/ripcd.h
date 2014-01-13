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

#ifndef RIPCD_H
#define RIPCD_H

#include <QDialog>
#include <QCheckBox>
#include <QThread>
#include <cdio/cdio.h>
#include "ui_ripcd.h"

class Ui_RipCD;
class Transcoder;

struct TranscoderPreset;

class RipCD: public QDialog {
	Q_OBJECT
//	QThread thread;

 public:
	RipCD(QWidget* parent = 0);
	static const char* kSettingsGroup;
	static const int kProgressInterval;
	static const int kMaxDestinationItems;

 private:
	Transcoder* transcoder_;
	int queued_;
	int finished_success_;
	int finished_failed_;
	track_t i_tracks;
	Ui_RipCD* ui_;
	CdIo_t *p_cdio;
	QList<QCheckBox*> checkboxes_;
	QList<QString> generated_files_;
	QList<int> tracks_to_rip_;
	QList<QLineEdit*> track_names_;
	QString last_add_dir_;
	QPushButton* cancel_button_;

	void write_WAV_header(FILE *stream, int32_t i_bytecount);
	void put_num(long int num, FILE *stream, int bytes);
	int nTracksToRip();
	void toThreadClickedRipButton();
	QString TrimPath(const QString& path) const;
	QString GetOutputFileName(const QString& input,
														const TranscoderPreset& preset) const;
	QString ParseFileFormatString(const QString& file_format, int trackNo) const;



 signals:
	void RippingComplete();
	void signalUpdateProgress();

 private slots:
	void UpdateProgress();
	void threadedTranscoding();
	void clickedRipButton();
	void JobComplete(const QString& filename, bool success);
	void AllJobsComplete();
	void AppendOutput(const QString &filename);
	void Options();
	void AddDestination();
	void Cancel();
};

#endif // RIPCD_H
