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

#include "transcodersettingspage.h"

#include "transcoder.h"
#include "transcoderoptionsdialog.h"
#include "ui/iconloader.h"
#include "ui_transcodersettingspage.h"

TranscoderSettingsPage::TranscoderSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_TranscoderSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("tools-wizard", IconLoader::Base));

  AddTab(tr("FLAC"), Transcoder::Codec_Flac);
  AddTab(tr("AAC"), Transcoder::Codec_Mp4);
  AddTab(tr("MP3"), Transcoder::Codec_Mp3);
  AddTab(tr("Vorbis"), Transcoder::Codec_Vorbis);
  AddTab(tr("Speex"), Transcoder::Codec_Speex);
  AddTab(tr("Opus"), Transcoder::Codec_Opus);
  AddTab(tr("WMA"), Transcoder::Codec_Wma);
}

TranscoderSettingsPage::~TranscoderSettingsPage() { delete ui_; }

void TranscoderSettingsPage::AddTab(const QString& label,
                                    Transcoder::CodecType codec) {
  const QString mime_type = Transcoder::MimeType(codec);

  TranscoderOptionsInterface* tab =
      TranscoderOptionsDialog::MakeOptionsPage(mime_type);
  options_pages_ << tab;

  // Insert in localized alphabetical order
  for (int i = 0; i < ui_->optionTabs->count(); i++) {
    if (label.localeAwareCompare(ui_->optionTabs->tabText(i)) < 0) {
      ui_->optionTabs->insertTab(i, tab, label);
      return;
    }
  }
  ui_->optionTabs->addTab(tab, label);
}

void TranscoderSettingsPage::Load() {
  for (TranscoderOptionsInterface* options : options_pages_) {
    options->Load();
  }
}

void TranscoderSettingsPage::Save() {
  for (TranscoderOptionsInterface* options : options_pages_) {
    options->Save();
  }
}
