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

#include "backgroundstreamssettingspage.h"

#include <QCheckBox>
#include <QGroupBox>

#include "core/backgroundstreams.h"
#include "iconloader.h"
#include "settingsdialog.h"
#include "ui_backgroundstreamssettingspage.h"

BackgroundStreamsSettingsPage::BackgroundStreamsSettingsPage(
    SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_BackgroundStreamsSettingsPage),
      loaded_(false) {
  ui_->setupUi(this);
  setWindowIcon(
      IconLoader::Load("weather-showers-scattered", IconLoader::Base));
}

BackgroundStreamsSettingsPage::~BackgroundStreamsSettingsPage() { delete ui_; }

void BackgroundStreamsSettingsPage::Load() {
  if (!loaded_ and dialog()->background_streams()) {
    for (const QString& name : dialog()->background_streams()->streams()) {
      AddStream(name);
    }
    loaded_ = true;
  }
}

void BackgroundStreamsSettingsPage::Save() {
  dialog()->background_streams()->SaveStreams();
}

void BackgroundStreamsSettingsPage::AddStream(const QString& name) {
  BackgroundStreams* streams = dialog()->background_streams();

  QGroupBox* box = new QGroupBox(tr(name.toUtf8()));
  QLabel* label = new QLabel(box);
  QSlider* slider = new QSlider(Qt::Horizontal, box);
  QCheckBox* check = new QCheckBox(box);
  QHBoxLayout* hbox_layout = new QHBoxLayout(box);
  hbox_layout->addWidget(label);
  hbox_layout->addWidget(slider);
  hbox_layout->addWidget(check);

  QVBoxLayout* streams_layout = qobject_cast<QVBoxLayout*>(layout());
  streams_layout->insertWidget(streams_layout->count() - 1, box);

  label->setPixmap(QPixmap(
      IconLoader::Load("audio-volume-medium", IconLoader::Base).pixmap(32)));
  slider->setProperty("stream_name", name);
  check->setProperty("stream_name", name);

  connect(slider, SIGNAL(valueChanged(int)), SLOT(StreamVolumeChanged(int)));
  connect(check, SIGNAL(toggled(bool)), SLOT(EnableStream(bool)));

  slider->setValue(streams->GetStreamVolume(name));
  check->setCheckState(streams->IsPlaying(name) ? Qt::Checked : Qt::Unchecked);
}

void BackgroundStreamsSettingsPage::EnableStream(bool enabled) {
  const QString name = sender()->property("stream_name").toString();
  dialog()->background_streams()->EnableStream(name, enabled);
}

void BackgroundStreamsSettingsPage::StreamVolumeChanged(int value) {
  const QString name = sender()->property("stream_name").toString();
  dialog()->background_streams()->SetStreamVolume(name, value);
}
