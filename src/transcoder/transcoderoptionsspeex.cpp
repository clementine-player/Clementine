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

#include "transcoderoptionsspeex.h"
#include "ui_transcoderoptionsspeex.h"

#include <QSettings>

const char* TranscoderOptionsSpeex::kSettingsGroup = "Transcoder/speexenc";

TranscoderOptionsSpeex::TranscoderOptionsSpeex(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsSpeex) {
  ui_->setupUi(this);
}

TranscoderOptionsSpeex::~TranscoderOptionsSpeex() { delete ui_; }

void TranscoderOptionsSpeex::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  ui_->quality_slider->setValue(s.value("quality", 8).toInt());
  ui_->bitrate_slider->setValue(s.value("bitrate", 0).toInt() / 1000);
  ui_->mode->setCurrentIndex(s.value("mode", 0).toInt());
  ui_->vbr->setChecked(s.value("vbr", false).toBool());
  ui_->abr_slider->setValue(s.value("abr", 0).toInt() / 1000);
  ui_->vad->setChecked(s.value("vad", false).toBool());
  ui_->dtx->setChecked(s.value("dtx", false).toBool());
  ui_->complexity->setValue(s.value("complexity", 3).toInt());
  ui_->nframes->setValue(s.value("nframes", 1).toInt());
}

void TranscoderOptionsSpeex::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("quality", ui_->quality_slider->value());
  s.setValue("bitrate", ui_->bitrate_slider->value() * 1000);
  s.setValue("mode", ui_->mode->currentIndex());
  s.setValue("vbr", ui_->vbr->isChecked());
  s.setValue("abr", ui_->abr_slider->value() * 1000);
  s.setValue("vad", ui_->vad->isChecked());
  s.setValue("dtx", ui_->dtx->isChecked());
  s.setValue("complexity", ui_->complexity->value());
  s.setValue("nframes", ui_->nframes->value());
}
