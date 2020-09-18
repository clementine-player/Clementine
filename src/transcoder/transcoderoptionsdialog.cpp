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

#include "transcoderoptionsaac.h"
#include "transcoderoptionsflac.h"
#include "transcoderoptionsmp3.h"
#include "transcoderoptionsopus.h"
#include "transcoderoptionsspeex.h"
#include "transcoderoptionsvorbis.h"
#include "transcoderoptionswma.h"
#include "ui_transcoderoptionsdialog.h"

TranscoderOptionsDialog::TranscoderOptionsDialog(Song::FileType type,
                                                 QWidget* parent)
    : QDialog(parent), ui_(new Ui_TranscoderOptionsDialog), options_(nullptr) {
  ui_->setupUi(this);

  switch (type) {
    case Song::Type_Flac:
    case Song::Type_OggFlac:
      options_ = new TranscoderOptionsFlac(this);
      break;
    case Song::Type_Mp4:
      options_ = new TranscoderOptionsAAC(this);
      break;
    case Song::Type_Mpeg:
      options_ = new TranscoderOptionsMP3(this);
      break;
    case Song::Type_OggVorbis:
      options_ = new TranscoderOptionsVorbis(this);
      break;
    case Song::Type_OggOpus:
      options_ = new TranscoderOptionsOpus(this);
      break;
    case Song::Type_OggSpeex:
      options_ = new TranscoderOptionsSpeex(this);
      break;
    case Song::Type_Asf:
      options_ = new TranscoderOptionsWma(this);
      break;
    default:
      break;
  }

  if (options_) {
    setWindowTitle(windowTitle() + " - " + Song::TextForFiletype(type));
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
