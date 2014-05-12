/* This file is part of Clementine.
   Copyright 2013, Martin Brodbeck <martin@brodbeck-online.de>

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

#include "transcoderoptionsopus.h"
#include "ui_transcoderoptionsopus.h"

#include <QSettings>

// TODO: Add more options than only bitrate as soon as gst doesn't crash
// anymore while using the cbr parmameter (like cbr=false)

const char* TranscoderOptionsOpus::kSettingsGroup = "Transcoder/opusenc";

TranscoderOptionsOpus::TranscoderOptionsOpus(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsOpus) {
  ui_->setupUi(this);
}

TranscoderOptionsOpus::~TranscoderOptionsOpus() { delete ui_; }

void TranscoderOptionsOpus::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  ui_->bitrate_slider->setValue(s.value("bitrate", 128000).toInt() / 1000);
}

void TranscoderOptionsOpus::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("bitrate", ui_->bitrate_slider->value() * 1000);
}
