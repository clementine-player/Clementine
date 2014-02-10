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

#include "iconloader.h"
#include "playbacksettingspage.h"
#include "settingsdialog.h"
#include "ui_playbacksettingspage.h"
#include "engines/gstengine.h"
#include "playlist/playlist.h"

PlaybackSettingsPage::PlaybackSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_PlaybackSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("media-playback-start"));

  connect(ui_->fading_cross, SIGNAL(toggled(bool)),
          SLOT(FadingOptionsChanged()));
  connect(ui_->fading_out, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
  connect(ui_->fading_auto, SIGNAL(toggled(bool)),
          SLOT(FadingOptionsChanged()));
  connect(ui_->gst_plugin, SIGNAL(currentIndexChanged(int)),
          SLOT(GstPluginChanged(int)));

  connect(ui_->replaygain_preamp, SIGNAL(valueChanged(int)),
          SLOT(RgPreampChanged(int)));
  ui_->replaygain_preamp_label->setMinimumWidth(
      QFontMetrics(ui_->replaygain_preamp_label->font()).width("-WW.W dB"));
  RgPreampChanged(ui_->replaygain_preamp->value());
}

PlaybackSettingsPage::~PlaybackSettingsPage() { delete ui_; }

void PlaybackSettingsPage::Load() {
  const GstEngine* engine = dialog()->gst_engine();

  if (ui_->gst_plugin->count() <= 1 && engine) {
    GstEngine::PluginDetailsList list = engine->GetOutputsList();

    ui_->gst_plugin->setItemData(0, GstEngine::kAutoSink);
    for (const GstEngine::PluginDetails& details : list) {
      if (details.name == "autoaudiosink") continue;

      ui_->gst_plugin->addItem(details.long_name, details.name);
    }
    ui_->gst_group->setEnabled(true);
    ui_->replaygain_group->setEnabled(true);
  }

  QSettings s;

  s.beginGroup(Playlist::kSettingsGroup);
  ui_->current_glow->setChecked(s.value("glow_effect", true).toBool());
  s.endGroup();

  s.beginGroup(Engine::Base::kSettingsGroup);
  ui_->fading_out->setChecked(s.value("FadeoutEnabled", true).toBool());
  ui_->fading_cross->setChecked(s.value("CrossfadeEnabled", true).toBool());
  ui_->fading_auto->setChecked(s.value("AutoCrossfadeEnabled", false).toBool());
  ui_->fading_duration->setValue(s.value("FadeoutDuration", 2000).toInt());
  ui_->fading_samealbum->setChecked(
      s.value("NoCrossfadeSameAlbum", true).toBool());
  ui_->fadeout_pause->setChecked(
      s.value("FadeoutPauseEnabled", false).toBool());
  ui_->fading_pause_duration->setValue(
      s.value("FadeoutPauseDuration", 250).toInt());
  s.endGroup();

  s.beginGroup(GstEngine::kSettingsGroup);
  QString sink = s.value("sink", GstEngine::kAutoSink).toString();
  ui_->gst_plugin->setCurrentIndex(0);
  for (int i = 0; i < ui_->gst_plugin->count(); ++i) {
    if (ui_->gst_plugin->itemData(i).toString() == sink) {
      ui_->gst_plugin->setCurrentIndex(i);
      break;
    }
  }
  ui_->gst_device->setText(s.value("device").toString());
  ui_->replaygain->setChecked(s.value("rgenabled", false).toBool());
  ui_->replaygain_mode->setCurrentIndex(s.value("rgmode", 0).toInt());
  ui_->replaygain_preamp->setValue(s.value("rgpreamp", 0.0).toDouble() * 10 +
                                   150);
  ui_->replaygain_compression->setChecked(
      s.value("rgcompression", true).toBool());
  ui_->buffer_duration->setValue(s.value("bufferduration", 4000).toInt());
  ui_->mono_playback->setChecked(s.value("monoplayback", false).toBool());
  s.endGroup();
}

void PlaybackSettingsPage::Save() {
  QSettings s;

  s.beginGroup(Playlist::kSettingsGroup);
  s.setValue("glow_effect", ui_->current_glow->isChecked());
  s.endGroup();

  s.beginGroup(Engine::Base::kSettingsGroup);
  s.setValue("FadeoutEnabled", ui_->fading_out->isChecked());
  s.setValue("FadeoutDuration", ui_->fading_duration->value());
  s.setValue("CrossfadeEnabled", ui_->fading_cross->isChecked());
  s.setValue("AutoCrossfadeEnabled", ui_->fading_auto->isChecked());
  s.setValue("NoCrossfadeSameAlbum", ui_->fading_samealbum->isChecked());
  s.setValue("FadeoutPauseEnabled", ui_->fadeout_pause->isChecked());
  s.setValue("FadeoutPauseDuration", ui_->fading_pause_duration->value());
  s.endGroup();

  s.beginGroup(GstEngine::kSettingsGroup);
  s.setValue("sink", ui_->gst_plugin->itemData(ui_->gst_plugin->currentIndex())
                         .toString());
  s.setValue("device", ui_->gst_device->text());
  s.setValue("rgenabled", ui_->replaygain->isChecked());
  s.setValue("rgmode", ui_->replaygain_mode->currentIndex());
  s.setValue("rgpreamp", float(ui_->replaygain_preamp->value()) / 10 - 15);
  s.setValue("rgcompression", ui_->replaygain_compression->isChecked());
  s.setValue("bufferduration", ui_->buffer_duration->value());
  s.setValue("monoplayback", ui_->mono_playback->isChecked());
  s.endGroup();
}

void PlaybackSettingsPage::GstPluginChanged(int index) {
  QString name = ui_->gst_plugin->itemData(index).toString();

  bool enabled = GstEngine::
      DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(name);

  ui_->gst_device->setEnabled(enabled);
  ui_->gst_device_label->setEnabled(enabled);
}

void PlaybackSettingsPage::RgPreampChanged(int value) {
  float db = float(value) / 10 - 15;
  QString db_str;
  db_str.sprintf("%+.1f dB", db);
  ui_->replaygain_preamp_label->setText(db_str);
}

void PlaybackSettingsPage::FadingOptionsChanged() {
  ui_->fading_options->setEnabled(ui_->fading_out->isChecked() ||
                                  ui_->fading_cross->isChecked() ||
                                  ui_->fading_auto->isChecked());
}
