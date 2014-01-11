#include "ripcd.h"
#include "config.h"
#include "ui_ripcd.h"
#include "transcoder/transcoder.h"
#include <QSettings>
#include <QCheckBox>
#include <QFrame>
#include <QLineEdit>
#include <QtDebug>
#include <QtConcurrentRun>
#include <cdio/cdda.h>
#include <cdio/cdio.h>
#include <cdio/paranoia.h>
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

RipCD::RipCD(QWidget* parent)
: QDialog(parent),
  transcoder_(new Transcoder(this)),
  queued_(0),
	finished_success_(0),
	finished_failed_(0)
{
	// Init
	ui_.setupUi(this);
	connect(ui_.ripButton,SIGNAL(clicked()),this,SLOT(clickedRipButton()));

	connect(transcoder_, SIGNAL(JobComplete(QString,bool)), SLOT(JobComplete(QString,bool)));
	connect(transcoder_, SIGNAL(AllJobsComplete()), SLOT(AllJobsComplete()));
	connect(this, SIGNAL(RippingComplete()), SLOT(threadedTranscoding()));
	setWindowTitle(tr("Rip CD"));




	//track_t i_first_track;
	track_t i_tracks;
	p_cdio = cdio_open(NULL, DRIVER_UNKNOWN);
	//i_first_track = cdio_get_first_track_num(p_cdio);
	i_tracks = cdio_get_num_tracks(p_cdio);
	ui_.tableWidget->setRowCount(i_tracks);
	for(int i=1; i<=i_tracks; i++) {
		qDebug() << i;
		QCheckBox *_t = new QCheckBox(tr(""), ui_.tableWidget);
		ui_.tableWidget->setCellWidget(i-1,0,_t);
		ui_.tableWidget->setCellWidget(i-1,1,new QLabel(QString::number(i)));
		QString _str_track;
		_str_track = "Track %1";
		ui_.tableWidget->setCellWidget(i-1,2,new QLineEdit(_str_track.arg(QString::number(i)),ui_.tableWidget));
	}
}

void RipCD::clickedRipButton() {
	QtConcurrent::run(this,&RipCD::toThreadClickedRipButton);

}

void RipCD::toThreadClickedRipButton() {
	QString source_directory = "/tmp/";
	track_t i_tracks = cdio_get_num_tracks(p_cdio);
	for(int i=1; i <= i_tracks; i++) {
		qDebug() << "Going for track " << i;
		lsn_t i_first_lsn = cdio_get_track_lsn(p_cdio,i);
		lsn_t i_last_lsn = cdio_get_track_last_lsn(p_cdio,i);
//		lsn_t i_last_lsn = i_first_lsn+300; // debug

		lsn_t i_cursor;
		int16_t *p_readbuf = (int16_t *)calloc(CDIO_CD_FRAMESIZE_RAW,1);
		FILE *fp = fopen(QString(source_directory + "track" + QString::number(i) + ".wav").toUtf8().constData(),"w");
		write_WAV_header(fp,(i_last_lsn-i_first_lsn+1) * CDIO_CD_FRAMESIZE_RAW);
		for ( i_cursor = i_first_lsn; i_cursor <= i_last_lsn; i_cursor ++) {
			cdio_read_audio_sector(p_cdio,p_readbuf,i_cursor);
			if( !p_readbuf ) {
				qDebug() << "Read error. Stopping.";
				break;
			} else {
				fwrite(p_readbuf, 1, CDIO_CD_FRAMESIZE_RAW, fp);
			}

		}
		fclose(fp);
		free(p_readbuf);
		p_readbuf = NULL;
	}
	emit(RippingComplete());
}

void RipCD::threadedTranscoding() {
	QString source_directory = "/tmp/";
	track_t i_tracks = cdio_get_num_tracks(p_cdio);

		TranscoderPreset preset(Transcoder::PresetForFileType(Song::Type_OggVorbis));
	for(int i=1; i <= i_tracks; i++) {
			transcoder_->AddJob(QString(source_directory + "track" + QString::number(i) + ".wav").toUtf8().constData(), preset);
		}
	qDebug() << "Total jobs: " << transcoder_->QueuedJobsCount();
	transcoder_->Start();

}

void RipCD::put_num(long int num, FILE *stream, int bytes) {
	unsigned int i;
	unsigned char c;

	for (i=0; bytes--; i++) {
		c = (num >> (i<<3)) & 0xff;
		if (fwrite(&c, sizeof(char), 1, stream)==-1) {
			perror("Could not write to output.");
			exit(1);
		}
	}

}

void RipCD::write_WAV_header(FILE *stream,int32_t i_bytecount) {
	fwrite("RIFF",sizeof(char),4,stream);
	put_num(i_bytecount+44-8, stream, 4);     /*  4-7 */
	fwrite("WAVEfmt ",sizeof(char),8,stream); /*  8-15 */
	put_num(16, stream, 4);                   /* 16-19 */
	put_num(1, stream, 2);                    /* 20-21 */
	put_num(2, stream, 2);                    /* 22-23 */
	put_num(44100, stream, 4);                /* 24-27 */
	put_num(44100*2*2, stream, 4);            /* 28-31 */
	put_num(4, stream, 2);                    /* 32-33 */
	put_num(16, stream, 2);                   /* 34-35 */
	fwrite("data",sizeof(char),4,stream);     /* 36-39 */
	put_num(i_bytecount, stream, 4);          /* 40-43 */

}

void RipCD::AllJobsComplete() {
	qDebug() << "All Jobs Complete emmited";
	// TODO Handle this properly
	// having a little trouble on wav files, works fine on mp3
	TagLib::FileRef f(QString("/tmp/track" + QString::number(1) + ".ogg").toUtf8().constData());
	f.tag()->setArtist("Queen");
	f.save();
}

void RipCD::JobComplete(const QString& filename, bool success) {
  qDebug() << "Completed";
}
