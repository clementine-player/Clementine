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
 Settings for the voaacenc gstreamer element.
 https://gstreamer.freedesktop.org/documentation/voaacenc/index.html
*/

#include "transcoderoptionsvoaac.h"

#include <QSettings>

#include "ui_transcoderoptionsvoaac.h"

const char* TranscoderOptionsVOAAC::kSettingsGroup = "Transcoder/voaacenc";

TranscoderOptionsVOAAC::TranscoderOptionsVOAAC(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsVOAAC) {
  ui_->setupUi(this);
}

TranscoderOptionsVOAAC::~TranscoderOptionsVOAAC() { delete ui_; }

void TranscoderOptionsVOAAC::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  ui_->bitrate_slider->setValue(s.value("bitrate", 128000).toInt() / 1000);
}

void TranscoderOptionsVOAAC::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  s.setValue("bitrate", ui_->bitrate_slider->value() * 1000);
}
