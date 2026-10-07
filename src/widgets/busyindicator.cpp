/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "busyindicator.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>

// Draws the spinner instead of playing a GIF so it stays sharp on high-DPI
// screens and follows the palette's text colour.
class BusyIndicatorSpinner : public QWidget {
 public:
  explicit BusyIndicatorSpinner(QWidget* parent = nullptr)
      : QWidget(parent), timer_(new QTimer(this)), step_(0) {
    setFixedSize(16, 16);
    timer_->setInterval(100);
    connect(timer_, &QTimer::timeout, this, [this] {
      step_ = (step_ + 1) % kDots;
      update();
    });
  }

  void start() { timer_->start(); }
  void stop() { timer_->stop(); }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.translate(QRectF(rect()).center());

    const qreal dot_radius = width() * 0.1;
    const qreal ring_radius = width() / 2.0 - dot_radius - 0.5;
    QColor color = palette().color(QPalette::WindowText);

    for (int i = 0; i < kDots; ++i) {
      // The dot at step_ is the head; the ones behind it fade out.
      const int age = (step_ - i + kDots) % kDots;
      color.setAlphaF(1.0 - age * 0.85 / (kDots - 1));
      p.setBrush(color);

      p.save();
      p.rotate(i * 360.0 / kDots);
      p.drawEllipse(QPointF(0, -ring_radius), dot_radius, dot_radius);
      p.restore();
    }
  }

 private:
  static const int kDots = 8;

  QTimer* timer_;
  int step_;
};

BusyIndicator::BusyIndicator(const QString& text, QWidget* parent)
    : QWidget(parent) {
  Init(text);
}

BusyIndicator::BusyIndicator(QWidget* parent) : QWidget(parent) {
  Init(QString());
}

void BusyIndicator::Init(const QString& text) {
  spinner_ = new BusyIndicatorSpinner;
  label_ = new QLabel;

  label_->setWordWrap(true);
  label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(spinner_);
  layout->addSpacing(6);
  layout->addWidget(label_);

  set_text(text);
}

BusyIndicator::~BusyIndicator() {}

void BusyIndicator::showEvent(QShowEvent*) { spinner_->start(); }

void BusyIndicator::hideEvent(QHideEvent*) { spinner_->stop(); }

void BusyIndicator::set_text(const QString& text) {
  label_->setText(text);
  label_->setVisible(!text.isEmpty());
}

QString BusyIndicator::text() const { return label_->text(); }
