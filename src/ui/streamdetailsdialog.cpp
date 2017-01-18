#include "streamdetailsdialog.h"
#include "ui_streamdetailsdialog.h"

#include <QDialogButtonBox>

StreamDetailsDialog::StreamDetailsDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui::StreamDetailsDialog) {
  ui_->setupUi(this);
}

StreamDetailsDialog::~StreamDetailsDialog() {}

void StreamDetailsDialog::setUrl(const QString& url) {
  ui_->url->setText(url);
  ui_->url->setCursorPosition(0);
}
void StreamDetailsDialog::setFormat(const QString& format) {
  ui_->format->setText(format);
}
void StreamDetailsDialog::setBitrate(int bitrate) {
  ui_->bitrate->setText(QString("%1 kbps").arg(bitrate / 1000));

  // Some bitrates aren't properly reported by GStreamer.
  // In that case do not display bitrate information.
  ui_->bitrate->setVisible(bitrate != 0);
  ui_->bitrate_label->setVisible(bitrate != 0);
}
void StreamDetailsDialog::setChannels(int channels) {
  ui_->channels->setText(QString::number(channels));
}
void StreamDetailsDialog::setDepth(int depth) {
  // Right now GStreamer seems to be reporting incorrect numbers for MP3 and AAC
  // streams, so we leave that value hidden in the UI.
  // ui_->depth->setText(QString("%1 bits").arg(depth));
  ui_->depth->setVisible(false);
  ui_->depth_label->setVisible(false);
}
void StreamDetailsDialog::setSampleRate(int sample_rate) {
  ui_->sample_rate->setText(QString("%1 Hz").arg(sample_rate));
}

void StreamDetailsDialog::Close() { this->close(); }
