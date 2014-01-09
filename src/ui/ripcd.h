/********************************************************************************
 ** Form generated from reading UI file 'ripcd.ui'
 **
 ** Created: Fri Dec 27 02:11:12 2013
 **      by: Qt User Interface Compiler version 4.8.1
 **
 ** WARNING! All changes made in this file will be lost when recompiling UI file!
 ********************************************************************************/

#ifndef RIPCD_H
#define RIPCD_H

#include <QDialog>
#include <QThread>
#include <cdio/cdio.h>
#include "ui_ripcd.h"
#include "transcoder/transcoder.h"

class RipCD : public QDialog
{
	Q_OBJECT
//	QThread thread;

public:
	RipCD(QWidget* parent = 0);

private:
	Transcoder* transcoder_;
	  int queued_;
	  int finished_success_;
	  int finished_failed_;
	Ui::RipCD ui_;
	CdIo_t *p_cdio;
	void write_WAV_header(FILE *stream,int32_t i_bytecount);
	void put_num(long int num, FILE *stream, int bytes);
private slots:
	void clickedRipButton();
	void toThreadClickedRipButton();
	void AllJobsComplete();
};

#endif // RIPCD_H
