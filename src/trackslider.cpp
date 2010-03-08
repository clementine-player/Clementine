#include "trackslider.h"

TrackSlider::TrackSlider(QWidget* parent)
  : QWidget(parent),
    setting_value_(false)
{
  ui_.setupUi(this);

  QFont font("Courier");
  ui_.elapsed->setFont(font);
  ui_.remaining->setFont(font);

  UpdateLabelWidth();

  connect(ui_.slider, SIGNAL(sliderMoved(int)), SIGNAL(ValueChanged(int)));
  connect(ui_.slider, SIGNAL(valueChanged(int)), SLOT(ValueMaybeChanged(int)));
}

void TrackSlider::UpdateLabelWidth() {
  // We set the label's minimum size so it won't resize itself when the user
  // is dragging the slider.
  UpdateLabelWidth(ui_.elapsed, "0:00:00");
  UpdateLabelWidth(ui_.remaining, "-0:00:00");
}

void TrackSlider::UpdateLabelWidth(QLabel* label, const QString& text) {
  QString old_text = label->text();
  label->setText(text);
  label->setMinimumWidth(0);
  int width = label->sizeHint().width();
  label->setText(old_text);

  label->setMinimumWidth(width);
}

QSize TrackSlider::sizeHint() const {
  int width = 500;
  width += ui_.elapsed->sizeHint().width();
  width += ui_.remaining->sizeHint().width();

  int height = qMax(ui_.slider->sizeHint().height(), ui_.elapsed->sizeHint().height());

  return QSize(width, height);
}

void TrackSlider::SetValue(int elapsed, int total) {
  setting_value_ = true; // This is so we don't emit from QAbstractSlider::valueChanged
  ui_.slider->setMaximum(total);
  ui_.slider->setValue(elapsed);
  setting_value_ = false;

  UpdateTimes(elapsed);
}

void TrackSlider::UpdateTimes(int elapsed) {
  ui_.elapsed->setText(PrettyTime(elapsed));
  ui_.remaining->setText("-" + PrettyTime(ui_.slider->maximum() - elapsed));

  setEnabled(true);
}

void TrackSlider::SetStopped() {
  setEnabled(false);
  ui_.elapsed->setText("0:00:00");
  ui_.remaining->setText("0:00:00");

  setting_value_ = true;
  ui_.slider->setValue(0);
  setting_value_ = false;
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

void TrackSlider::ValueMaybeChanged(int value) {
  if (setting_value_)
    return;

  UpdateTimes(value);
  emit ValueChanged(value);
}

bool TrackSlider::event(QEvent* e) {
  switch (e->type()) {
    case QEvent::ApplicationFontChange:
    case QEvent::StyleChange:
      UpdateLabelWidth();
      break;
  }
  return false;
}
