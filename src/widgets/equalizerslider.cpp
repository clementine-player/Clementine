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

#include "equalizerslider.h"
#include "ui_equalizerslider.h"

#include <QFontMetrics>

EqualizerSlider::EqualizerSlider(const QString& label, QWidget* parent)
    : QWidget(parent), ui_(new Ui_EqualizerSlider) {
  ui_->setupUi(this);
  ui_->band->setText(label);  // Band [Hz]

  QFontMetrics fm = ui_->gain->fontMetrics();
  int longestLabelWidth = fm.width(tr("%1 dB").arg(-99.99));
  ui_->gain->setMinimumWidth(longestLabelWidth);
  ui_->gain->setText(tr("%1 dB").arg(0));  // Gain [dB]

  ui_->slider->setValue(0);

  connect(ui_->slider, SIGNAL(valueChanged(int)), this,
          SLOT(onValueChanged(int)));
}

void EqualizerSlider::onValueChanged(int value) {
  // Converting % to dB as per GstEnginePipeline::UpdateEqualizer():
  float gain = (value < 0) ? value * 0.24 : value * 0.12;

  ui_->gain->setText(tr("%1 dB").arg(gain));  // Gain [dB]
  emit ValueChanged(value);
}

EqualizerSlider::~EqualizerSlider() { delete ui_; }

int EqualizerSlider::value() const { return ui_->slider->value(); }

void EqualizerSlider::set_value(int value) { ui_->slider->setValue(value); }
