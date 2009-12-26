#include "busyindicator.h"

#include <QMovie>

BusyIndicator::BusyIndicator(QWidget* parent)
    : QLabel(parent),
      movie_(new QMovie(":spinner.gif"))
{
  setMovie(movie_);
}

BusyIndicator::~BusyIndicator() {
  delete movie_;
}

void BusyIndicator::showEvent(QShowEvent*) {
  movie_->start();
}

void BusyIndicator::hideEvent(QHideEvent*) {
  movie_->stop();
}
