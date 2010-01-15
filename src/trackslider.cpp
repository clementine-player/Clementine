#include "trackslider.h"

TrackSlider::TrackSlider(QWidget* parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  QFont font("Courier");
  ui_.elapsed->setFont(font);
  ui_.remaining->setFont(font);

  connect(ui_.slider, SIGNAL(sliderMoved(int)), SIGNAL(ValueChanged(int)));
}

QSize TrackSlider::sizeHint() const {
  int width = 500;
  width += ui_.elapsed->sizeHint().width();
  width += ui_.remaining->sizeHint().width();

  int height = qMax(ui_.slider->sizeHint().height(), ui_.elapsed->sizeHint().height());

  return QSize(width, height);
}

void TrackSlider::SetValue(int elapsed, int total) {
  ui_.slider->setMaximum(total);
  ui_.slider->setValue(elapsed);

  ui_.elapsed->setText(PrettyTime(elapsed));
  ui_.remaining->setText(PrettyTime(total - elapsed));

  setEnabled(true);
}

void TrackSlider::SetStopped() {
  setEnabled(false);
  ui_.elapsed->setText("0:00:00");
  ui_.remaining->setText("0:00:00");
  ui_.slider->setValue(0);
}

QString TrackSlider::PrettyTime(int seconds) {
  int hours = seconds / (60*60);
  int minutes = (seconds / 60) % 60;
  seconds %= 60;

  QString ret;
  if (hours)
    ret.sprintf("%d:%02d:%02d", hours, minutes, seconds);
  else
    ret.sprintf("%d:%02d", minutes, seconds);

  return ret;
}

void TrackSlider::SetCanSeek(bool can_seek) {
  ui_.slider->setEnabled(can_seek);
}
