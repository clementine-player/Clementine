#include "streamdetailsdialog.h"
#include "ui_streamdetailsdialog.h"

#include <QDialogButtonBox>

StreamDetailsDialog::StreamDetailsDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::StreamDetailsDialog) {
  ui->setupUi(this);
}

StreamDetailsDialog::~StreamDetailsDialog() { delete ui; }

void StreamDetailsDialog::setUrl(const QString& url) {
  ui->url->setText(url);
  ui->url->setCursorPosition(0);
}
void StreamDetailsDialog::setFormat(const QString& format) {
  ui->format->setText(format);
}
void StreamDetailsDialog::setBitrate(unsigned int bitrate) {
  ui->bitrate->setText(QString("%1 kbps").arg(bitrate / 1000));

  // Some bitrates aren't properly reported by GStreamer.
  // In that case do not display bitrate information.
  ui->bitrate->setVisible(bitrate != 0);
  ui->bitrate_label->setVisible(bitrate != 0);
}
void StreamDetailsDialog::setChannels(unsigned int channels) {
  ui->channels->setText(QString::number(channels));
}
void StreamDetailsDialog::setDepth(unsigned int depth) {
  // Right now GStreamer seems to be reporting incorrect numbers for MP3 and AAC
  // streams, so we leave that value hidden in the UI.
  // ui->depth->setText(QString("%1 bits").arg(depth));
  ui->depth->setVisible(false);
  ui->depth_label->setVisible(false);
}
void StreamDetailsDialog::setSampleRate(unsigned int sample_rate) {
  ui->sample_rate->setText(QString("%1 Hz").arg(sample_rate));
}

void StreamDetailsDialog::Close() { this->close(); }
