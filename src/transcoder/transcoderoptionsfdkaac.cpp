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
 Settings for the fdkaacenc gstreamer element.
 https://gstreamer.freedesktop.org/documentation/fdkaac/fdkaacenc.html
*/

#include "transcoderoptionsfdkaac.h"

#include <QSettings>

#include "ui_transcoderoptionsfdkaac.h"

const char* TranscoderOptionsFDKAAC::kSettingsGroup = "Transcoder/fdkaacenc";

TranscoderOptionsFDKAAC::TranscoderOptionsFDKAAC(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsFDKAAC) {
  ui_->setupUi(this);
  connect(ui_->target_radio, SIGNAL(toggled(bool)),
          SLOT(TargetBitrateToggle(bool)));
}

TranscoderOptionsFDKAAC::~TranscoderOptionsFDKAAC() { delete ui_; }

void TranscoderOptionsFDKAAC::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  // If bitrate is set to 0 (default), then bitrate is detemined by sample rate
  // and channel count.
  int bitrate = s.value("bitrate", 0).toInt();
  if (bitrate == 0) {
    ui_->auto_radio->setChecked(true);
    ui_->bitrate_group->setEnabled(false);
  } else {
    ui_->target_radio->setChecked(true);
    ui_->bitrate_slider->setValue(bitrate / 1000);
  }
}

void TranscoderOptionsFDKAAC::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  if (ui_->target_radio->isChecked()) {
    s.setValue("bitrate", ui_->bitrate_slider->value() * 1000);
  } else {
    s.setValue("bitrate", 0);
  }
}

void TranscoderOptionsFDKAAC::TargetBitrateToggle(bool checked) {
  ui_->bitrate_group->setEnabled(checked);
}
