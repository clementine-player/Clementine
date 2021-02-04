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

#include "transcoderoptionsdialog.h"

#include "core/logging.h"
#include "core/utilities.h"
#include "transcoder.h"
#include "transcoderoptionsaac.h"
#include "transcoderoptionserror.h"
#include "transcoderoptionsflac.h"
#include "transcoderoptionsmp3.h"
#include "transcoderoptionsopus.h"
#include "transcoderoptionsspeex.h"
#include "transcoderoptionsvorbis.h"
#include "transcoderoptionswma.h"
#include "ui_transcoderoptionsdialog.h"

TranscoderOptionsDialog::TranscoderOptionsDialog(const TranscoderPreset& preset,
                                                 QWidget* parent)
    : QDialog(parent), ui_(new Ui_TranscoderOptionsDialog), options_(nullptr) {
  ui_->setupUi(this);

  options_ = MakeOptionsPage(preset.codec_mimetype_, this);

  setWindowTitle(tr("Transcoding options - %1").arg(preset.name_));

  // Show options widget
  if (options_) {
    options_->layout()->setContentsMargins(0, 0, 0, 0);
    ui_->verticalLayout->insertWidget(0, options_);
    resize(width(), minimumHeight());
  }
}

TranscoderOptionsDialog::~TranscoderOptionsDialog() { delete ui_; }

void TranscoderOptionsDialog::showEvent(QShowEvent* e) {
  if (options_) {
    options_->Load();
  }
}

void TranscoderOptionsDialog::accept() {
  if (options_) {
    options_->Save();
  }
  QDialog::accept();
}

void TranscoderOptionsDialog::set_settings_postfix(
    const QString& settings_postfix) {
  if (options_) {
    options_->settings_postfix_ = settings_postfix;
  }
}

TranscoderOptionsInterface* TranscoderOptionsDialog::MakeOptionsPage(
    const QString& mime_type, QWidget* parent) {
  QString element = Transcoder::GetEncoderFactoryForMimeType(mime_type);

  qLog(Debug) << "Options for element" << element;
  if (element == "flacenc") {
    return new TranscoderOptionsFlac(parent);
  } else if (element == "faac") {
    return new TranscoderOptionsAAC(parent);
  } else if (element == "lamemp3enc") {
    return new TranscoderOptionsMP3(parent);
  } else if (element == "vorbisenc") {
    return new TranscoderOptionsVorbis(parent);
  } else if (element == "opusenc") {
    return new TranscoderOptionsOpus(parent);
  } else if (element == "speexenc") {
    return new TranscoderOptionsSpeex(parent);
  } else if (element == "ffenc_wmav2") {
    return new TranscoderOptionsWma(parent);
  } else {
    return new TranscoderOptionsError(mime_type, element, parent);
  }
}
