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

#include "transcoderoptionsvorbis.h"
#include "ui_transcoderoptionsvorbis.h"

#include <QSettings>

const char* TranscoderOptionsVorbis::kSettingsGroup = "Transcoder/vorbisenc";

TranscoderOptionsVorbis::TranscoderOptionsVorbis(QWidget* parent)
    : TranscoderOptionsInterface(parent), ui_(new Ui_TranscoderOptionsVorbis) {
  ui_->setupUi(this);
}

TranscoderOptionsVorbis::~TranscoderOptionsVorbis() { delete ui_; }

void TranscoderOptionsVorbis::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

#define GET_BITRATE(variable, property)         \
  int variable = s.value(property, -1).toInt(); \
  variable = variable == -1 ? 0 : variable / 1000

  GET_BITRATE(bitrate, "bitrate");
  GET_BITRATE(min_bitrate, "min-bitrate");
  GET_BITRATE(max_bitrate, "max-bitrate");
#undef GET_BITRATE

  ui_->quality_slider->setValue(s.value("quality", 0.3).toDouble() * 10);
  ui_->managed->setChecked(s.value("managed", false).toBool());
  ui_->max_bitrate_slider->setValue(max_bitrate);
  ui_->min_bitrate_slider->setValue(min_bitrate);
  ui_->bitrate_slider->setValue(bitrate);
}

void TranscoderOptionsVorbis::Save() {
  QSettings s;
  s.beginGroup(kSettingsGroup + settings_postfix_);

#define GET_BITRATE(variable, ui_slider) \
  int variable = ui_slider->value();     \
  variable = variable == 0 ? -1 : variable * 1000

  GET_BITRATE(bitrate, ui_->bitrate_slider);
  GET_BITRATE(min_bitrate, ui_->min_bitrate_slider);
  GET_BITRATE(max_bitrate, ui_->max_bitrate_slider);
#undef GET_BITRATE

  s.setValue("quality", double(ui_->quality_slider->value()) / 10);
  s.setValue("managed", ui_->managed->isChecked());
  s.setValue("bitrate", bitrate);
  s.setValue("min-bitrate", min_bitrate);
  s.setValue("max-bitrate", max_bitrate);
}
