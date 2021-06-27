/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

/*
 Settings for the avenc_aac gstreamer element.
 https://gstreamer.freedesktop.org/documentation/libav/avenc_aac.html
*/

#include "transcoderoptionsavaac.h"

#include <QSettings>

#include "core/logging.h"
#include "ui_transcoderoptionsavaac.h"

const char* TranscoderOptionsAvAAC::kSettingsGroup = "Transcoder/avenc_aac";

TranscoderOptionsAvAAC::TranscoderOptionsAvAAC(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsAvAAC) {
  ui_->setupUi(this);
  connect(ui_->bitrate_target, SIGNAL(toggled(bool)),
          SLOT(TargetBitrateToggle(bool)));
}

TranscoderOptionsAvAAC::~TranscoderOptionsAvAAC() { delete ui_; }

void TranscoderOptionsAvAAC::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  // The gst-libav defaults to "fast". ffmpeg's default is twoloop.
  int encoder = s.value("aac-coder", 2).toInt();
  switch (encoder) {
    case 0:
      ui_->encoder_anmr->setChecked(true);
      break;
    case 1:
      ui_->encoder_twoloop->setChecked(true);
      break;
    default:
      qLog(Warning) << "Unknown encoder setting" << encoder;
      // Fall through.
    case 2:
      ui_->encoder_fast->setChecked(true);
      break;
  }

  // If bitrate is set to 0 (default), then the codec default.
  int bitrate = s.value("bitrate", 0).toInt();
  if (bitrate == 0) {
    ui_->bitrate_default->setChecked(true);
    ui_->bitrate_group->setEnabled(false);
  } else {
    ui_->bitrate_target->setChecked(true);
    ui_->bitrate_slider->setValue(bitrate / 1000);
  }
}

void TranscoderOptionsAvAAC::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  int encoder;
  int strict = 0;
  if (ui_->encoder_anmr->isChecked()) {
    encoder = 0;
    // Allow experimental
    strict = -2;
  } else if (ui_->encoder_twoloop->isChecked()) {
    encoder = 1;
  } else {
    encoder = 2;
  }
  s.setValue("aac-coder", encoder);
  s.setValue("strict", strict);

  if (ui_->bitrate_target->isChecked()) {
    s.setValue("bitrate", ui_->bitrate_slider->value() * 1000);
  } else {
    s.setValue("bitrate", 0);
  }
}

void TranscoderOptionsAvAAC::TargetBitrateToggle(bool checked) {
  ui_->bitrate_group->setEnabled(checked);
}
