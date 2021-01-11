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

#include <QSettings>

#include "ui_transcoderoptionsflac.h"

const char* TranscoderOptionsFlac::kSettingsGroup = "Transcoder/flacenc";

TranscoderOptionsFlac::TranscoderOptionsFlac(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsFlac) {
  ui_->setupUi(this);
  connect(ui_->quality, SIGNAL(valueChanged(int)), this,
          SLOT(ValueChanged(int)));
}

TranscoderOptionsFlac::~TranscoderOptionsFlac() { delete ui_; }

void TranscoderOptionsFlac::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  ui_->quality->setValue(s.value("quality", 5).toInt());
}

void TranscoderOptionsFlac::ValueChanged(int value) {
  if (!IsInStreamingSubset(value)) {
    ui_->info->setStyleSheet("QLabel { color : red }");
    ui_->info->setText(
        tr("Warning: This compression level is outside of the streamable "
           "subset. This means that a decoder may not be able to start "
           "playing it mid-stream. It may also affect the performance of "
           "hardware decoders."));
  } else {
    ui_->info->setStyleSheet("");
    ui_->info->setText("");
  }
}

void TranscoderOptionsFlac::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

  int quality = ui_->quality->value();
  s.setValue("quality", quality);

  s.setValue("streamable-subset", IsInStreamingSubset(quality));
}

bool TranscoderOptionsFlac::IsInStreamingSubset(int level) {
  // The gstreamer flacenc element defines its own quality settings. The
  // highest level, 9 (aka "insane"), uses settings that are outside of the
  // streamable subset.
  // https://xiph.org/flac/format.html#subset
  return (level < 9);
}
