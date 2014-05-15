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

#include "core/application.h"
#include "iconloader.h"
#include "streamingsettingspage.h"
#include "ui_streamingsettingspage.h"
#include "streaming/streamserver.h"
#include "settingsdialog.h"

#include <QSettings>
#include <QHostInfo>

StreamingSettingsPage::StreamingSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_StreamingSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("ipodtouchicon"));
}

StreamingSettingsPage::~StreamingSettingsPage() { delete ui_; }

void StreamingSettingsPage::Load() {
  QSettings s;

  s.beginGroup(StreamServer::kSettingsGroup);

  ui_->use_streaming->setChecked(s.value("use_streaming").toBool());
  ui_->stream_port->setValue(
      s.value("port", StreamServer::kDefaultServerPort).toInt());

  ui_->bitrate_box->setValue(s.value("bitrate", 128).toInt());

  s.endGroup();
}

void StreamingSettingsPage::Save() {
  QSettings s;

  s.beginGroup(StreamServer::kSettingsGroup);
  bool wasStreaming = s.value("use_streaming", false).toBool();

  s.setValue("use_streaming", ui_->use_streaming->isChecked());
  s.setValue("port", ui_->stream_port->value());
  s.setValue("bitrate", ui_->bitrate_box->value());

  s.endGroup();

  if (wasStreaming != ui_->use_streaming->isChecked()) {
    if (ui_->use_streaming->isChecked())
      dialog()->app()->stream_server()->Listen();
    else
      dialog()->app()->stream_server()->StopListening();
  }

}
