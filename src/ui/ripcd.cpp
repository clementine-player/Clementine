#include "ripcd.h"
#include "config.h"
#include "ui_ripcd.h"
#include <QCheckBox>
#include <QFrame>
#include <QtDebug>
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
