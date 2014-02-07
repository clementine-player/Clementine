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

#include "transcoderoptionsmp3.h"
#include "ui_transcoderoptionsmp3.h"

#include <QSettings>

const char* TranscoderOptionsMP3::kSettingsGroup = "Transcoder/lamemp3enc";

TranscoderOptionsMP3::TranscoderOptionsMP3(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsMP3) {
  ui_->setupUi(this);

  connect(ui_->quality_slider, SIGNAL(valueChanged(int)),
          SLOT(QualitySliderChanged(int)));
  connect(ui_->quality_spinbox, SIGNAL(valueChanged(double)),
          SLOT(QualitySpinboxChanged(double)));
}

TranscoderOptionsMP3::~TranscoderOptionsMP3() { delete ui_; }

void TranscoderOptionsMP3::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  ;

  if (s.value("target", 1).toInt() == 0) {
    ui_->target_quality->setChecked(true);
  } else {
    ui_->target_bitrate->setChecked(true);
  }

  ui_->quality_spinbox->setValue(s.value("quality", 4.0).toFloat());
  ui_->bitrate_slider->setValue(s.value("bitrate", 128).toInt());
  ui_->cbr->setChecked(s.value("cbr", true).toBool());
  ui_->encoding_engine_quality->setCurrentIndex(
      s.value("encoding-engine-quality", 1).toInt());
  ui_->mono->setChecked(s.value("mono", false).toBool());
}

void TranscoderOptionsMP3::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("target", ui_->target_quality->isChecked() ? 0 : 1);
  s.setValue("quality", ui_->quality_spinbox->value());
  s.setValue("bitrate", ui_->bitrate_slider->value());
  s.setValue("cbr", ui_->cbr->isChecked());
  s.setValue("encoding-engine-quality",
             ui_->encoding_engine_quality->currentIndex());
  s.setValue("mono", ui_->mono->isChecked());
}

void TranscoderOptionsMP3::QualitySliderChanged(int value) {
  ui_->quality_spinbox->setValue(float(value) / 100);
}

void TranscoderOptionsMP3::QualitySpinboxChanged(double value) {
  ui_->quality_slider->setValue(value * 100);
}
