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

#include "transcoderoptionsaac.h"

#include <QSettings>

#include "ui_transcoderoptionsaac.h"

const char* TranscoderOptionsAAC::kSettingsGroup = "Transcoder/faac";

TranscoderOptionsAAC::TranscoderOptionsAAC(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsAAC) {
  ui_->setupUi(this);
}

TranscoderOptionsAAC::~TranscoderOptionsAAC() { delete ui_; }

void TranscoderOptionsAAC::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  ui_->bitrate_slider->setValue(s.value("bitrate", 128000).toInt() / 1000);
  ui_->profile->setCurrentIndex(s.value("profile", 2).toInt() - 1);
  ui_->tns->setChecked(s.value("tns", false).toBool());
  ui_->midside->setChecked(s.value("midside", true).toBool());
  ui_->shortctl->setCurrentIndex(s.value("shortctl", 0).toInt());
}

void TranscoderOptionsAAC::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  s.setValue("bitrate", ui_->bitrate_slider->value() * 1000);
  s.setValue("profile", ui_->profile->currentIndex() + 1);
  s.setValue("tns", ui_->tns->isChecked());
  s.setValue("midside", ui_->midside->isChecked());
  s.setValue("shortctl", ui_->shortctl->currentIndex());
}
