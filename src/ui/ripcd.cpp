#include "ripcd.h"
#include "config.h"
#include "ui_ripcd.h"
#include <QCheckBox>
#include <QFrame>
#include <QtDebug>
#include <cdio/cdda.h>
#include <cdio/cdio.h>
#include <cdio/cdtext.h>
#include <sys/types.h>

RipCD::RipCD(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
  setWindowTitle(tr("Rip CD"));
  //QFrame *_frame = new QFrame();
  //QCheckBox *_t = new QCheckBox(tr(""), ui_.tableWidget);

  //track_t i_first_track;
  	track_t i_tracks;
  	CdIo_t *p_cdio;
  	p_cdio = cdio_open(NULL, DRIVER_UNKNOWN);
  	//i_first_track = cdio_get_first_track_num(p_cdio);
  	i_tracks = cdio_get_num_tracks(p_cdio);
  	ui_.tableWidget->setRowCount(i_tracks);
  for(int i=0; i<i_tracks; i++) {
  	qDebug() << i;
  	QCheckBox *_t = new QCheckBox(tr(""), ui_.tableWidget);
  	ui_.tableWidget->setCellWidget(i,0,_t);
  	ui_.tableWidget->setCellWidget(i,1,new QLabel(QString::number(i)));
  }
  cdio_destroy(p_cdio);
}

/**
 * Down goes a model for cd ripping using cdda and paranoia to wav files
 * it needs to be incorporated to the main code, which is to follow
 */
// This must come after Qt includes
//#include <cdio/cdda.h>
//#include <cdio/cdio.h>
//#include <cdio/paranoia.h>
//#include <stdio.h>
//#include <cdio/cdtext.h>
//#include <sys/types.h>
//#include <cdio/cdtext.h>
//#include <string.h>
//
//static void put_num(long int num, FILE *stream, int bytes) {
//	unsigned int i;
//		unsigned char c;
//
//		for (i=0; bytes--; i++) {
//			c = (num >> (i<<3)) & 0xff;
//			if (fwrite(&c, sizeof(char), 1, stream)==-1) {
//				perror("Could not write to output.");
//				exit(1);
//			}
//		}
//
//}
//
//static void write_WAV_header(FILE *stream,int32_t i_bytecount) {
//	fwrite("RIFF",sizeof(char),4,stream);
//	put_num(i_bytecount+44-8, stream, 4);     /*  4-7 */
//	fwrite("WAVEfmt ",sizeof(char),8,stream); /*  8-15 */
//	put_num(16, stream, 4);                   /* 16-19 */
//		put_num(1, stream, 2);                    /* 20-21 */
//		put_num(2, stream, 2);                    /* 22-23 */
//		put_num(44100, stream, 4);                /* 24-27 */
//		put_num(44100*2*2, stream, 4);            /* 28-31 */
//		put_num(4, stream, 2);                    /* 32-33 */
//		put_num(16, stream, 2);                   /* 34-35 */
//		fwrite("data",sizeof(char),4,stream);     /* 36-39 */
//		put_num(i_bytecount, stream, 4);          /* 40-43 */
//
//}
//
//int main(int argc, const char *argv[]) {
//
//	cdrom_drive_t *d = NULL; /* Place to store handle given by cd-paranoia. */
//	char **ppsz_cd_drives;   /* List of all drives with a loaded CDDA in it. */
//
//	/* See if we can find a device with a loaded CD-DA in it. */
//	ppsz_cd_drives = cdio_get_devices_with_cap(NULL, 1, false);
//
//	if (ppsz_cd_drives) {
//		/* Found such a CD-ROM with a CD-DA loaded. Use the first drive in
//       the list. */
//		d=cdio_cddap_identify(*ppsz_cd_drives, 1, NULL);
//	} else {
//		printf("Unable find or access a CD-ROM drive with an audio CD in it.\n");
//		//exit(1);
//	}
//
//	/* Don't need a list of CD's with CD-DA's any more. */
//	cdio_free_device_list(ppsz_cd_drives);
//
//	/* We'll set for verbose paranoia messages. */
//	cdio_cddap_verbose_set(d, CDDA_MESSAGE_PRINTIT, CDDA_MESSAGE_PRINTIT);
//
//	if ( 0 != cdio_cddap_open(d) ) {
//		printf("Unable to open disc.\n");
//		//    exit(1);
//	}
//	/* Okay now set up to read up to the first 300 frames of the first
//       audio track of the Audio CD. */
//	{
//		cdrom_paranoia_t *p = cdio_paranoia_init(d);
//		lsn_t i_first_lsn = cdio_cddap_disc_firstsector(d);
//
//		if ( -1 == i_first_lsn ) {
//			printf("Trouble getting starting LSN\n");
//		} else {
//			lsn_t   i_cursor;
//			ssize_t bytes_ret;
//			track_t i_track    = cdda_sector_gettrack(d, i_first_lsn);
//			printf("Going for track %d\n",i_track);
//			lsn_t   i_last_lsn = cdda_track_lastsector(d, 1);
//
//			printf("CD has %d tracks\n",cdda_tracks(d));
//			//lsn_t   i_last_lsn = cdda_track_lastsector(d, i_track);
//			/*int fd = creat("track1s.wav", 0644);
//			if (-1 == fd) {
//				printf("Unable to create track1s.wav\n");
//				exit(1);
//			}*/
//
//
//			/* For demo purposes we'll read only 300 frames (about 4
//  	 seconds).  We don't want this to take too long. On the other
//  	 hand, I suppose it should be something close to a real test.
//			 */
//			//if ( i_last_lsn - i_first_lsn > 300) i_last_lsn = i_first_lsn + 299;
//			for(track_t j = 1; j <= cdda_tracks(d); j++) {
//				printf("Going for track %d\n",j);
//				i_first_lsn = cdda_track_firstsector(d,j);
//				i_last_lsn = cdda_track_lastsector(d,j);
//				printf("Reading track %d from LSN %ld to LSN %ld\n", j,
//						(long int) i_first_lsn, (long int) i_last_lsn);
//				char joined[20];
//				sprintf(joined,"track%d.wav",j);
//				FILE *fp = fopen(joined,"w");
//				/*int fd = creat(joined, 0644);
//				if (-1 == fd) {
//					printf("Unable to create track1s.wav\n");
//					exit(1);
//				}*/
//
//				/* Set reading mode for full paranoia, but allow skipping sectors. */
//				paranoia_modeset(p, PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP);
//
//				paranoia_seek(p, i_first_lsn, SEEK_SET);
//				//write_WAV_header(fd, (i_last_lsn-i_first_lsn+1) * CDIO_CD_FRAMESIZE_RAW);
//				write_WAV_header(fp,(i_last_lsn-i_first_lsn+1) * CDIO_CD_FRAMESIZE_RAW);
//
//				//FILE *test = fopen("test.wav","w");
//				for ( i_cursor = i_first_lsn; i_cursor <= i_last_lsn; i_cursor ++) {
//					/* read a sector */
//					int16_t *p_readbuf=cdio_paranoia_read(p, NULL);
//					char *psz_err=cdio_cddap_errors(d);
//					char *psz_mes=cdio_cddap_messages(d);
//
//					if (psz_mes || psz_err)
//						printf("%s%s\n", psz_mes ? psz_mes: "", psz_err ? psz_err: "");
//
//					if (psz_err) free(psz_err);
//					if (psz_mes) free(psz_mes);
//					if( !p_readbuf ) {
//						printf("paranoia read error. Stopping.\n");
//						break;
//					} else {
//						fwrite(p_readbuf, 1, CDIO_CD_FRAMESIZE_RAW, fp);
//					}
//				}
//			  fclose(fp);
//			}
//		}
//		cdio_paranoia_free(p);
//	}
//
//
//	cdio_cddap_close(d);
//
//
//
//
//	return 0;
//}
