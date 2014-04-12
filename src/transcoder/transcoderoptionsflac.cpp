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

#include "transcoderoptionsflac.h"
#include "ui_transcoderoptionsflac.h"

#include <QSettings>

const char* TranscoderOptionsFlac::kSettingsGroup = "Transcoder/flacenc";

TranscoderOptionsFlac::TranscoderOptionsFlac(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsFlac) {
  ui_->setupUi(this);
}

TranscoderOptionsFlac::~TranscoderOptionsFlac() { delete ui_; }

void TranscoderOptionsFlac::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  ui_->quality->setValue(s.value("quality", 5).toInt());
}

void TranscoderOptionsFlac::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("quality", ui_->quality->value());
}
