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
#include "ui_transcodersettingspage.h"
#include "ui/iconloader.h"

TranscoderSettingsPage::TranscoderSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_TranscoderSettingsPage)
{
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("tools-wizard"));
}

TranscoderSettingsPage::~TranscoderSettingsPage() {
  delete ui_;
}

void TranscoderSettingsPage::Load() {
  ui_->transcoding_aac->Load();
  ui_->transcoding_flac->Load();
  ui_->transcoding_mp3->Load();
  ui_->transcoding_speex->Load();
  ui_->transcoding_vorbis->Load();
  ui_->transcoding_wma->Load();
  ui_->transcoding_opus->Load();
}

void TranscoderSettingsPage::Save() {
  ui_->transcoding_aac->Save();
  ui_->transcoding_flac->Save();
  ui_->transcoding_mp3->Save();
  ui_->transcoding_speex->Save();
  ui_->transcoding_vorbis->Save();
  ui_->transcoding_wma->Save();
  ui_->transcoding_opus->Save();
}
