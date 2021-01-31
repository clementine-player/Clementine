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

  QString element =
      Transcoder::GetEncoderFactoryForMimeType(preset.codec_mimetype_);

  qLog(Debug) << "Options for element" << element;
  if (element == "flacenc") {
    options_ = new TranscoderOptionsFlac(this);
  } else if (element == "faac") {
    options_ = new TranscoderOptionsAAC(this);
  } else if (element == "lamemp3enc") {
    options_ = new TranscoderOptionsMP3(this);
  } else if (element == "vorbisenc") {
    options_ = new TranscoderOptionsVorbis(this);
  } else if (element == "opusenc") {
    options_ = new TranscoderOptionsOpus(this);
  } else if (element == "speexenc") {
    options_ = new TranscoderOptionsSpeex(this);
  } else if (element == "ffenc_wmav2") {
    options_ = new TranscoderOptionsWma(this);
  } else if (element.isEmpty()) {
    ui_->errorMessage->setText(tr("Could not find a suitable encoder element "
                                  "for <b>%1</b>.")
                                   .arg(preset.name_));
  } else {
    QString url = Utilities::MakeBugReportUrl(
        QString("transcoder settings: Unknown encoder element: ") + element);
    ui_->errorMessage->setText(tr("No settings page available for encoder "
                                  "element <b>%1</b>. "
                                  "Please report this issue:<br>"
                                  "<a href=\"%2\">%2</a>")
                                   .arg(element)
                                   .arg(url));
  }

  setWindowTitle(tr("Transcoding options - %1").arg(preset.name_));

  // Show options widget if available.
  if (options_) {
    ui_->errorMessage->setVisible(false);
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
